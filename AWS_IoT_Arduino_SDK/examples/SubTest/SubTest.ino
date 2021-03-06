/*
 * SubTest.inc
 * A demo for Grove on Seeeduino Cloud with AWS IoT
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


aws_iot_mqtt_client myClient; // init iot_mqtt_client
char msg[32]; // read-write buffer
int rc = -100; // return value placeholder
const int ledPin =  3;      // The SCL pin of I2C port is the D3 pin.

// Basic callback function that prints out the message
void msg_callback(char* src, int len) {
  Serial.print("Subscribe topic/Arduino_LED: ");
  memset(msg, 0, sizeof(msg));
  int i;
  for(i = 0; i < len; i++) {
    msg[i] = src[i];
    Serial.print(src[i]);
  }
  Serial.println("");
}

void setup() {

  pinMode(ledPin, OUTPUT);       // The SCL pin of I2C port is the D3 pin.
  
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
  Serial.println(" ");
  
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

}

void loop() {
  // Receive a new message in each loop and subscribe to "topic/Arduino_LED"

  // Subscribe to "topic/Arduino_LED"
  memset(msg, 0, sizeof(msg));
  if((rc = myClient.subscribe("topic/Arduino_LED", 1, msg_callback)) != 0) {
    Serial.println("Subscribe failed!");
    Serial.println(rc);
  }
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);

  // Get a chance to run a callback
  if((rc = myClient.yield()) != 0) {
    Serial.println("Yield failed!");
    Serial.println(rc);
  }
  
  int i = 0;
  Serial.print("local message: ");
  for(i = 0; i < 32; i++) {
    Serial.print(msg[i]);
  }
  Serial.println("");
  
  if(strcmp(msg, "on") == 0)
  {
    Serial.println("Turn on LED!");
    digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level). The SCL pin of I2C port is the D3 pin.
  }
  else if(strcmp(msg, "off") == 0)
  {
    Serial.println("Turn off LED!");
    digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW. The SCL pin of I2C port is the D3 pin.
  }
  else
  {
    Serial.println("Error Message!");
    digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  }
  
  delay(2000);
}
