#include "LiquidCrystal_I2C.h"
#include "main.h"
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include "Preferences.h"

#define meshLED        14 
#define loraLED        27
#define motorPIN       12
#define obsPIN         5
#define MESH_PREFIX    "RNTMESH"                           // name for your MESH
#define MESH_PASSWORD  "MESHpassword"                      // password for your MESH
#define MESH_PORT      5555                                // default port

uint8_t lcdColumns = 16;
uint8_t lcdRows    = 2;                                      
uint8_t nodeNumber = 2;                                    // Number for this node
unsigned long invert = 0;
volatile unsigned long lastInterruptTime = 0;
unsigned long interruptInterval = 0;
uint8_t ackFlag;
String readings;                                           // String to send to other nodes with sensor readings

Preferences prf;
LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);
Scheduler userScheduler;                                   // to control your personal task
painlessMesh mesh;

void sendMessage() ;                                      
String getReadings();                                     
void receivedCallback( uint32_t from, String &msg );
// float calcRPM(unsigned long interruptInterval);

QueueHandle_t cmd_queue;
QueueHandle_t mes_queue;
Task taskSendMessage(TASK_SECOND*2, TASK_FOREVER, &sendMessage);
TaskHandle_t handleData;

void vHandleData(void *pvParameters){
  for(;;){
    // digitalWrite(loraLED, HIGH);
    const char* command;
    if (xQueueReceive(cmd_queue, &command, 100) == pdTRUE){

      Serial.println(command);

      if(strcmp(command, "?motorON") == 0){
        digitalWrite(motorPIN, HIGH);
      } else if(strcmp(command, "?motorOFF") == 0){
        digitalWrite(motorPIN, LOW);
      } else if(strcmp(command, "?reset") == 0){
        ESP.restart();
      }
      ackFlag = 1;
      // digitalWrite(loraLED, LOW);
    }
    //   if (flag == 1){
    //     ackFlag = ACK_SUCCESS;

    //     digitalWrite(loraLED, !digitalRead(loraLED));

    //     // cmd = prf.getString("cmd");
    //     uint8_t j = 0;
    //     if (cmd.charAt(0) == 0x02){
    //       switch (cmd.charAt(1))
    //       {
    //       case 0x01:
    //         if (cmd.charAt(2) == 0x00) digitalWrite(motorPIN, LOW);
    //         else if (cmd.charAt(2) == 0x01) digitalWrite(motorPIN, HIGH);
    //         break;
          
    //       case 0x02:
    //         for (uint8_t i = 2; i<cmd.length(); i++){
    //           message[j] = cmd.charAt(i);
    //           j++;
    //         }
    //         message[j] = '\0';
    //         prf.putString("message", message);
    //         xQueueSend(mes_queue, &j, portMAX_DELAY);
    //         break;

    //       default: break;
    //       }
    //     }
    //   } else ackFlag = ACK_FAIL;
    // } else ackFlag = ACK_FULL_QUEUE;
    vTaskDelay(100);
  }
}

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["motor"] = digitalRead(motorPIN);
  jsonReadings["obstacle"] = digitalRead(obsPIN); 
  jsonReadings["tim2"] = (int)(xTaskGetTickCount()/1000);
  jsonReadings["ack2"] = ackFlag;

  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  digitalWrite(meshLED, !digitalRead(meshLED));
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  #ifdef DEBUG_ka
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  #endif

  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];

  switch (node){
    case 1:{ 
      // digitalWrite(meshLED, !digitalRead(meshLED)); 
      break;
    }    
    case 3:{
      uint8_t ack = myObject["ack"];
      if (ack == 1) ackFlag = 0;

      String cmd = myObject["cmd"];
      const char* command = cmd.c_str();
      if (cmd != "") xQueueSend(cmd_queue, &command, portMAX_DELAY);
           
      break;
    }
    default: break;
  }
}

void setup() {
  pinMode(motorPIN, OUTPUT);
  // digitalWrite(motorPIN, HIGH);
  pinMode(meshLED, OUTPUT);
  pinMode(loraLED, OUTPUT);
  pinMode(obsPIN, INPUT);
  // attachInterrupt(obsPIN, count, RISING);

  Serial.begin(115200);

  prf.begin("data");

  // // turn on LCD backlight     
  // lcd.init();                 
  // lcd.backlight();
  // lcd.clear();

  cmd_queue = xQueueCreate(5, sizeof(const char *));

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  
  xTaskCreate(vHandleData, "callback", 1024*3, NULL, 0, &handleData);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}

// void IRAM_ATTR count(){
//   unsigned long currentTime = micros();
//   interruptInterval = currentTime - lastInterruptTime;
//   lastInterruptTime = currentTime;
// }

// float calcRPM(unsigned long interruptInterval){                   
//   float res = (60*1000000/2)/interruptInterval;
//   if ((int)res > 7000 && (int)res < 4000) res = 5000.0 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 1000.0); 
//   return res;
// }
