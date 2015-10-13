/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef aws_iot_mqtt_h
#define aws_iot_mqtt_h

#include "Arduino.h"
#include "string.h"
#include "aws_iot_config_SDK.h"
#include "aws_iot_error.h"

typedef enum {
	MQTTv31 = 3,
	MQTTv311 = 4
} MQTTv_t;

typedef void(*message_callback)(char*, int);

class aws_iot_mqtt_client {
	public:
		aws_iot_mqtt_client() {
			timeout_flag = false;
			ThingShadow_req_num = 0;
			memset(rw_buf, '\0', MAX_BUF_SIZE);
			memset(msg_buf, '\0', MAX_BUF_SIZE);
			int i;
			for(i = 0; i < MAX_SUB; i++) {
                sub_group[i].ThingShadow = -1;
				sub_group[i].is_used = false;
				sub_group[i].callback = NULL;
			}
		}
		IoT_Error_t setup(char* client_id, bool clean_session=true, MQTTv_t MQTT_version=MQTTv311);
		IoT_Error_t config(char* host, int port, char* cafile_path, char* keyfile_path, char* certfile_path);
		IoT_Error_t connect(int keepalive_interval=60);
		IoT_Error_t publish(char* topic, char* payload, int payload_len, int qos, bool retain);
		IoT_Error_t subscribe(char* topic, int qos, message_callback cb);
		IoT_Error_t unsubscribe(char* topic);
		IoT_Error_t yield();
		IoT_Error_t disconnect();
		// Thing shadow support
		IoT_Error_t shadow_init(char* thingName);
		IoT_Error_t shadow_update(char* thingName, char* payload, int payload_len, message_callback cb, int timeout);
		IoT_Error_t shadow_get(char* thingName, message_callback cb, int timeout);
		IoT_Error_t shadow_delete(char* thingName, message_callback cb, int timeout);
		IoT_Error_t shadow_register_delta_func(char* thingName, message_callback cb);
		IoT_Error_t shadow_unregister_delta_func(char* thingName);
	private:
		typedef struct {
            int ThingShadow;
			bool is_used;
			message_callback callback;
		} mqtt_sub_element;
		const char* my_client_id;
		char rw_buf[MAX_BUF_SIZE];
		char msg_buf[MAX_BUF_SIZE]; // To store message chunks
		mqtt_sub_element sub_group[MAX_SUB];
		bool timeout_flag; // Is there a timeout when executing RPC
		unsigned int ThingShadow_req_num; // sequence number for ThingShadow request
		IoT_Error_t base_subscribe(char* topic, int qos, message_callback cb, int is_delta);
		void exec_cmd(const char* cmd, bool wait, bool single_line);
		bool is_num(char* src);
};

#endif
