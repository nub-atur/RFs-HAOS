#include <main.h>

int interval = 2000; 

Preferences prf;
String readings;
Scheduler userScheduler;
painlessMesh mesh;
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//---------------------------------------prototypes---------------------------------------------------------------
void meshSendMessage() ;
void meshReceivedCallback( uint32_t from, String &msg );
void loraOnReceive(int packetSize);
void vLoRa_go(void *pvParameters);
void vScreen_go(void *pvParameters);

//------------------------------------Tasks and Queue-------------------------------------------------------------
Task mesh_go(TASK_SECOND, TASK_FOREVER, &meshSendMessage);   
TaskHandle_t lora_go;
TaskHandle_t screen_go;
QueueHandle_t rssi_flag;
QueueHandle_t screen_flag;

void vScreen_go(void * pvParameters){
  (void)pvParameters;

  String en;
  int rssi;

  for(;;){
    if (uxQueueMessagesWaiting(rssi_flag)){
      if (xQueueReceive(rssi_flag, &rssi, portMAX_DELAY) == pdTRUE){
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("RSSI = ");
        display.print(rssi); 
        display.display();
      }
    }
    vTaskDelay(1000);    
  }
}

void vLoRa_go(void * pvParameters){
  (void)pvParameters;

  uint8_t countData = 0;
  long lastSendTime = 0;                // last send time

  LoRa.setPins(ss, rst, dio0);
  LoRa.setSyncWord(0x23);
  if (!LoRa.begin(433E6)) {             // initialize ratio at 433 MHz
    #ifdef DEBUG_ka
      Serial.println("LoRa init failed. Check your connections.");
    #endif
    while (true);                       // if failed, do nothing
  }
  
  LoRa.onReceive(loraOnReceive);
  LoRa.receive(); 

  for (;;){
    if (xTaskGetTickCount() - lastSendTime > interval) 
    {
      data dataReceive;
      JsonDocument doc;
      doc["name"]    = "brigdeNode";
      doc["root"][0] = (int)(lastSendTime/1000);
      doc["root"][1] = ( (int)analogRead(BATTERY_PIN) / (2.1 * 4096 /3.3) * 100 );

      doc["node1"][0] = prf.getDouble("hum");
      doc["node1"][1] = prf.getDouble("tem");
      doc["node1"][2] = prf.getInt("t1");
      doc["node1"][3] = prf.getInt("ack1");
      
      doc["node2"][0] = prf.getInt("motor");
      doc["node2"][1] = prf.getInt("obs");
      doc["node2"][2] = prf.getInt("t2");
      doc["node2"][3] = prf.getInt("ack2");
      
      LoRa.beginPacket();                     // start packet
      countData = serializeJson(doc, LoRa);
      LoRa.endPacket();

      lastSendTime = xTaskGetTickCount();     // timestamp the message

      interval = random(3000);                // 2-3 seconds

      #ifdef DEBUG_ka
        serializeJson(doc, Serial);
        Serial.printf("\nDEBUG: LoRa send: %d bytes\r\n", countData);
      #endif
    }

    LoRa.receive();                           // go back into receive mode                         
    vTaskDelay(200);
  } 
}


//-------------------------------------FUNCTIONS------------------------------------------------------------------------------------------------------------------------
void meshSendMessage()
{
  //------main fucntion of task mesh_go-----------------------------------------------------------------------//                                                                                                                                                                    
      String rxmes1;                                                                                          //
      uint8_t flag = 0;                                                                                       //
      if (uxQueueMessagesWaiting(screen_flag)){                                                                     //
        if (xQueueReceive(screen_flag, &flag, portMAX_DELAY) == pdTRUE){                                            //
          if (flag > 0){                                                                                      //
            rxmes1 = prf.getString("text");                                                                   //
          } else {                                                                                            //
            rxmes1 = "null";                                                                                  //
          }                                                                                                   //
        }                                                                                                     //
      }                                                                                                       //

      JSONVar data;                                                                                           //
      data["node"] = 3;                                                                                       //
      data["cmd"] = rxmes1;                                                                                   //
      String msg = JSON.stringify(data);                                                                      //
        #ifdef DEBUG_ka                                                                                       //
          Serial.print("DEBUG : meshFrame = ");                                                               //
          Serial.println(msg);                                                                                //
        #endif                                                                                                //
      if( mesh.sendBroadcast(msg) ) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));                     //
//------------------------------------------------------------------------------------------------------------// 
}

void meshReceivedCallback(uint32_t from, String &msg)
{
  #ifdef DEBUG_ka
    // Serial.printf("DEBUG: Mesh receive : from %u msg=%s\n", from, msg.c_str());
  #endif
  
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  
  data dataSend;

  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  
  if (node == 1)
  {
    dataSend.temp1 = round(((double)myObject["temp"]) * 100) / 100;
    dataSend.hum1 = round(((double)myObject["hum"]) * 100) / 100;
    dataSend.time1 = (int)myObject["tim1"];
    dataSend.ack_get1 = myObject["ack1"]; 

    prf.putDouble("hum", dataSend.hum1);
    prf.putDouble("tem", dataSend.temp1);
    prf.putInt("t1", dataSend.time1);
    prf.putInt("ack1", dataSend.ack_get1);
  } 
  else if (node == 2)
  {
    dataSend.motor2 = (int)myObject["motor"];
    dataSend.obs2 = (int)myObject["obstacle"];
    dataSend.time2 = (int)myObject["tim2"];
    dataSend.ack_get2 = myObject["ack2"];                                                                                              //( ( (strcmp(ack.c_str(), "KO+F") == 0) || (strcmp(ack.c_str(), "KO+Q") == 0) )? 0x00 : 0x01); 

    prf.putInt("motor", dataSend.motor2);
    prf.putInt("obs", dataSend.obs2);
    prf.putInt("ack2", dataSend.ack_get2);
    prf.putInt("t2", dataSend.time2);
  }
}

// void newConnectionCallback(uint32_t nodeId) {
//   Serial.printf("New Connection, nodeId = %u\n", nodeId);
// }

// void changedConnectionCallback() {
//   Serial.printf("Changed connections\n");
// }

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
// }

void loraOnReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return 

  String res;
  int rssi = LoRa.packetRssi();
  uint8_t i = 0;

  while (LoRa.available()) {
    res += (char)LoRa.read();
    i++;
  }
  res[i] = '\0';

  prf.putString("text", res);

  xQueueSend(rssi_flag, &rssi, portMAX_DELAY); 
  xQueueSend(screen_flag, &i, portMAX_DELAY);
  
  #ifdef DEBUG_ka
    // Serial.print("DEBUG: LoRaRecv = ");
    // while (i>0){
    //   Serial.printf("%d", res[i]);
    //   i--;
    // }
    // Serial.println();
    // Serial.printf("DEBUG: RSSI = %d \r\n", rssi);
  #endif

  digitalWrite(LED, !digitalRead(LED));
}

void setup() {
  #ifdef DEBUG_ka
    Serial.begin(115200);
    Serial.println("DEBUG: ON !!!");

    mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before mesh.init() so that you can see startup messages
  #endif

  display.begin(SCREEN_ADDRESS, true); 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);

  prf.begin("data");
  
  pinMode(LED, OUTPUT); 
  pinMode(LED_BUILTIN, OUTPUT);

  rssi_flag = xQueueCreate(2, sizeof(int));   
  screen_flag = xQueueCreate(2, sizeof(uint8_t));

  xTaskCreatePinnedToCore(vLoRa_go, "LoRa", 1024*4, NULL, 2, &lora_go, 1);
  xTaskCreate(vScreen_go, "screen", 1024*2, NULL, 1, &screen_go);

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&meshReceivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(mesh_go);
  mesh_go.enable();
}

void loop() {
  mesh.update();
}
