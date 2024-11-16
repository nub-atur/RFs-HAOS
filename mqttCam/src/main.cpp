#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "Base64.h"
#include "main.h"

//Prototypes
String sendImage();
void callback(char* topic, byte* payload, unsigned int length);
void getCommand(char c);
void executeCommand();

WiFiClient espClient;
PubSubClient client(espClient);

String command="";
byte receiveState=0;

void reconnect() {
  while (!client.connected()) {
    if (client.connect(mqtt_server)) {
      Serial.println("MQTT connected");
      String text = WiFi.localIP().toString();
      client.publish(mqtt_topic_ip, text.c_str());
      text = WiFi.macAddress();
      client.publish(mqtt_topic_mac, text.c_str());
      client.subscribe(mqtt_topic_feedback);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  //
  // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
  //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
  //            Partial images will be transmitted if image exceeds buffer size
  //   
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){ 
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);    

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  String text = WiFi.localIP().toString();
  client.publish(mqtt_topic_ip, text.c_str());
  text = WiFi.macAddress();
  client.publish(mqtt_topic_mac, text.c_str());
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //get RSSI // Keep publishing RSSI to MQTT server 
  int8_t rssi = WiFi.RSSI();
  char rssi_str[4];
  itoa(rssi, rssi_str, 10); 
  const char* rssiValue = rssi_str;
  client.publish(mqtt_topic_rssi, rssiValue);
  delay(10);
}

String sendImage() {
 camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }  

  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
  // String imageFile = "data:image/jpeg;base64,"; //normal way to intergrate with mqtt
  String imageFile = "";                           //Home Assistant handle it already
  for (int i=0;i<fb->len;i++) {
    base64_encode(output, (input++), 3);
    if (i%3==0) imageFile += String(output);
  }
  int fbLen = imageFile.length();
  
  String clientId = "ESP32-";
  clientId += String(random(0xffff), HEX);
  if (client.connect(clientId.c_str())) {
    
    client.beginPublish(mqtt_topic_image, fbLen, true);

    String str = "";
    for (size_t n=0;n<fbLen;n=n+2048) {
      if (n+2048<fbLen) {
        str = imageFile.substring(n, n+2048);
        client.write((uint8_t*)str.c_str(), 2048);
      }
      else if (fbLen%2048>0) {
        size_t remainder = fbLen%2048;
        str = imageFile.substring(n, n+remainder);
        client.write((uint8_t*)str.c_str(), remainder);
      }
    }  
    
    client.endPublish();

    Serial.println("Sent image");
    
    esp_camera_fb_return(fb);
    
    return "";
  }
  esp_camera_fb_return(fb);
  return "failed, rc="+client.state();
}

void callback(char* topic, byte* payload, unsigned int length){
  command="";

  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char c = payload[i];
    getCommand(payload[i]);
    Serial.print(c);
  }
  Serial.println();
    
  if (command!="") 
    executeCommand();
}

void getCommand(char c){
  if (c=='?') receiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) receiveState=0;
  
  if (receiveState==1) {
    command=command+String(c);
  }
}

void executeCommand(){
  if (command=="?restart") {
    ESP.restart();
  } else if (command == "?picture"){
    sendImage();
  }
  command = "";
}