#ifndef aws_iot_mqtt_h
#define aws_iot_mqtt_h

#include "Arduino.h"
#include "string.h"

#define MAX_BUF_SIZE 256 // Maximum message can be handled is 256B
#define MAX_SUB 5

typedef enum {
	MQTTv31 = 3,
	MQTTv311 = 4
} MQTTv_t;

typedef void(*message_callback)(char*, int);

void get_error_msg(char* buf, int code); // could be only for debug

class iot_mqtt_client {
	public:
		iot_mqtt_client() {
			memset(rw_buf, '\0', MAX_BUF_SIZE);
			memset(msg_buf, '\0', MAX_BUF_SIZE);
			int i;
			for(i = 0; i < MAX_SUB; i++) {
				sub_group[i].is_used = false;
				sub_group[i].callback = NULL;
			}
		}
		int setup(char* client_id, bool clean_session, MQTTv_t MQTT_version);
		int connect();
		int connect(int keepalive);
		int config(char* serverURL, int serverPORT, char* cafile, char* key, char* cert);
		int publish(char* topic, char* payload, int qos, bool retain);
		int subscribe(char* topic, int qos, message_callback cb);
		int unsubscribe(char* topic);
		int yield();
		int disconnect();
	private:
		typedef struct {
			bool is_used;
			message_callback callback;
		} mqtt_sub_element;
		char rw_buf[MAX_BUF_SIZE];
		char msg_buf[MAX_BUF_SIZE]; // To store message chunks
		mqtt_sub_element sub_group[MAX_SUB];
		void exec_cmd(char* cmd, bool wait, bool single_line);
		bool is_num(char* src);
};

#endif
