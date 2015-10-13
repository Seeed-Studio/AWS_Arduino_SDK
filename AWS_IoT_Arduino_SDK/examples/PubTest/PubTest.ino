/*
 * PubTest.inc
 * A demo for Seeeduino Cloud with AWS IoT
 *
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Wuruibin
 * Modified Time: Oct 2015
 */

#include <aws_iot_mqtt.h>
#include <aws_iot_version.h>
#include <string.h>
#include "aws_iot_config.h"

#define LED_STATE  "on"
//#define LED_STATE  "off"

aws_iot_mqtt_client myClient; // init iot_mqtt_client
char msg[32]; // read-write buffer
int rc = -100; // return value placeholder

// Basic callback function that prints out the message
void msg_callback(char* src, int len) {
  Serial.print("Subscribe local topic/Arduino_LED: ");
  memset(msg, 0, sizeof(msg));
  int i;
  for(i = 0; i < len; i++) {
    msg[i] = src[i];
    Serial.print(src[i]);
  }
  Serial.println("");
}

void setup() {
  // Start Serial for print-out and wait until it's ready
  Serial.begin(115200);
  while(!Serial);
  //
  char curr_version[80];
  sprintf(curr_version, "AWS IoT SDK Version(dev) %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
  Serial.println(curr_version);
  
  // Set up the client
  if((rc = myClient.setup(AWS_IOT_CLIENT_ID)) != 0) {
    Serial.println("Setup failed!");
    Serial.println(rc);
  }
  Serial.print("AWS_IOT_CLIENT_ID: ");
  Serial.println(AWS_IOT_CLIENT_ID);
  
  // Load user configuration
  if((rc = myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) != 0) {
    Serial.println("Config failed!");
    Serial.println(rc);
  }
  
  // Use default connect: 60 sec for keepalive
  if((rc = myClient.connect()) != 0) {
    Serial.println("Connect failed!");
    Serial.println(rc);
  }
  
  memset(msg, 0, sizeof(msg));
  // Subscribe to "topic/Arduino_LED"
  if((rc = myClient.subscribe("topic/Arduino_LED", 1, msg_callback)) != 0) {
    Serial.println("Subscribe failed!");
    Serial.println(rc);
  }
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);
}

void loop() {
  // Generate a new message in each loop and publish to "topic/Arduino_LED"

  if((rc = myClient.publish("topic/Arduino_LED", LED_STATE, strlen(LED_STATE), 1, false)) != 0) {
    Serial.println("Publish failed!");
    Serial.println(rc);
  }
  else
  {
    Serial.print("Publish topic/Arduino_LED: ");
    Serial.println(LED_STATE);
  }
  // Get a chance to run a callback
  if((rc = myClient.yield()) != 0) {
    Serial.println("Yield failed!");
    Serial.println(rc);
  }
/* 
  int i = 0;
  Serial.print("msg:");
  for(i = 0; i < 32; i++) {
    Serial.print(msg[i]);
  }
  Serial.println("");
*/  
  delay(2000);
}
