#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


const char* ssid = "806";
const char* password = "05022001";

const char* mqtt_server = "karolineserver.duckdns.org";
const char* mqtt_topic_image = "camera/image";
const char* mqtt_topic_feedback = "camera/command";
const char* mqtt_topic_rssi = "camera/rssi";
const char* mqtt_topic_ip = "camera/ip";
const char* mqtt_topic_mac = "camera/mac";

#define KA_DEBUG

#ifdef __cplusplus
}
#endif

#endif