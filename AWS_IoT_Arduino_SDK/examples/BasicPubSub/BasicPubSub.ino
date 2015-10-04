#include <aws_iot_mqtt.h>

iot_mqtt_client myClient; // init iot_mqtt_client
char msg[32]; // read-write buffer
int cnt = 0; // loop counts
int rc = -100; // return value placeholder

// Basic callback function that prints out the message
void msg_callback(char* src, int len) {
  Serial.println("CALLBACK:");
  int i;
  for(i = 0; i < len; i++) {
    Serial.print(src[i]);
  }
  Serial.println("");
}

void setup() {
  // Start Serial for print-out and wait until it's ready
  Serial.begin(115200);
  while(!Serial);
  // Set up the client
  if((rc = myClient.setup("sample", true, MQTTv311)) != 0) {
    Serial.println("Setup failed!");
    Serial.println(rc);
  }
  // Use default connect: 60 sec for keepalive
  if((rc = myClient.connect()) != 0) {
    Serial.println("Connect failed!");
    Serial.println(rc);
  }
  // Subscribe to "topic1"
  if((rc = myClient.subscribe("topic1", 1, msg_callback)) != 0) {
    Serial.println("Subscribe failed!");
    Serial.println(rc);
  }
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(3000);
}

void loop() {
  // Generate a new message in each loop and publish to "topic1"
  sprintf(msg, "new message %d", cnt);
  if((rc = myClient.publish("topic1", msg, 1, false)) != 0) {
    Serial.println("Publish failed!");
    Serial.println(rc);
  }
  
  // Get a chance to run a callback
  if((rc = myClient.yield()) != 0) {
    Serial.println("Yield failed!");
    Serial.println(rc);
  }
  
  // Done with the current loop
  sprintf(msg, "loop %d done", cnt++);
  Serial.println(msg);
  
  delay(5000);
}
