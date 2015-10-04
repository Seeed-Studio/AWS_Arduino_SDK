#include "aws_iot_mqtt.h" // remove "../ArduinoLib/" when running on board!

#include "Arduino.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#define linux_baud 250000
#define CMD_TIME_OUT 100 // 100 * 0.1 sec = 10 sec
#define RETURN_KEY 13 // ASCII code for '\r'
#define NEXTLINE_KEY 10 // ASCII code for '\n'

bool timeout_flag = false;

int iot_mqtt_client::setup(char* client_id, bool clean_session, MQTTv_t MQTT_version) {
	int rc = 0;
	// Start Serial1
	Serial1.begin(linux_baud);
	while(!Serial1); // blocking until Serial1 is ready

	exec_cmd("cd .\n", false, false); // placeholder: jump over the welcoming prompt for Open WRT
	exec_cmd("cd /root\n", false, false);
    exec_cmd("~\n", true, false); // exit the previous python process
	exec_cmd("python aws_iot_mqtt_client.py\n", false, false);

	// Create obj
	exec_cmd("i\n", false, false);

	sprintf(rw_buf, "%s\n", client_id);
	exec_cmd(rw_buf, false, false);

	int num_temp = clean_session ? 1 : 0;
	sprintf(rw_buf, "%d\n", num_temp);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%u\n", MQTT_version);
	exec_cmd(rw_buf, true, false);

	if(rw_buf[0] == 'I' && rw_buf[2] == 'T') {rc = 0;}
	else {rc = -1;}
	return rc;
}

int iot_mqtt_client::config(char* serverURL, int serverPORT, char* cafile, char* key, char* cert) {
	int rc = 0;
	exec_cmd("g\n",false, false);

	sprintf(rw_buf, "%s\n", serverURL);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%d\n", serverPORT);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%s\n", cafile);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%s\n", key);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%s\n", cert);
	exec_cmd(rw_buf, true, false);

	if(rw_buf[0] == 'G' && rw_buf[2] == 'T') {rc = 0;}
	else {rc = -1;}

	return rc;
}

int iot_mqtt_client::connect() {
	return connect(60); // default keepalive is 60 sec
}

int iot_mqtt_client::connect(int keepalive) {
	int rc = 0;
	exec_cmd("c\n", false, false);

	sprintf(rw_buf, "%d\n", keepalive);
	exec_cmd(rw_buf, true, false);

	if(rw_buf[0] == 'C') {
		if(rw_buf[2] == 'F') {rc = -1;}
		else {
			char* saveptr;
			char* p;
			p = strtok_r(rw_buf, " ", &saveptr); // 'C'
			p = strtok_r(NULL, " ", &saveptr); // return number
			// if the returned feedback does not follow this convention (p == NULL), atoi will result in SEGFAULT!
			if(p == NULL) {rc = -1;}
			else {rc = is_num(p) ? atoi(p) : -1;}
		}
	}
	else {rc = -1;}
	return rc;
}

int iot_mqtt_client::publish(char* topic, char* payload, int qos, bool retain) {
	int rc = 0;
	exec_cmd("p\n", false, false);

	sprintf(rw_buf, "%s\n", topic);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%s\n", payload);
	exec_cmd(rw_buf, false, false);

	sprintf(rw_buf, "%d\n", qos);
	exec_cmd(rw_buf, false, false);

	int num_temp = retain ? 1 : 0;
	sprintf(rw_buf, "%d\n", num_temp);
	exec_cmd(rw_buf, true, false);

	if(rw_buf[0] == 'P') {
		if(rw_buf[2] == 'F') {rc = -1;}
		else {
			char* saveptr;
			char* p;
			p = strtok_r(rw_buf, " ", &saveptr); // 'P'
			p = strtok_r(NULL, " ", &saveptr); // return number
			if(p == NULL) {rc = -1;}
			else {rc = is_num(p) ? atoi(p) : -1;}
		}
	}
	else {rc = -1;}
	return rc;
}

int iot_mqtt_client::subscribe(char* topic, int qos, message_callback cb) {
	int rc = 0;
	// find unused slots for new subscribe
	int i;
	for(i = 0; i < MAX_SUB; i++) {
		if(!sub_group[i].is_used) {break;}
	}
	if(i < MAX_SUB) {
		sub_group[i].is_used = true;
		sub_group[i].callback = cb;

		exec_cmd("s\n", false, false);

		sprintf(rw_buf, "%s\n\0", topic);
		exec_cmd(rw_buf, false, false);

		sprintf(rw_buf, "%d\n\0", qos);
		exec_cmd(rw_buf, false, false);

		sprintf(rw_buf, "%d\n\0", i);
		exec_cmd(rw_buf, true, false);

		if(rw_buf[0] == 'S') {
			if(rw_buf[2] == 'F') {rc = -1;}
			else {
				char* saveptr;
				char* p;
				p = strtok_r(rw_buf, " ", &saveptr); // 'S'
				p = strtok_r(NULL, " ", &saveptr); // return number
				if(p == NULL) {rc = -1;}
				else {rc = is_num(p) ? atoi(p) : -1;}
			}
		}
		else {rc = -1;}
	}
	else {rc = -1;}
	return rc;
}

int iot_mqtt_client::unsubscribe(char* topic) {
	int rc = 0;
	exec_cmd("u\n", false, false);

	sprintf(rw_buf, "%s\n\0", topic);
	exec_cmd(rw_buf, true, false);

	if(rw_buf[0] == 'U') {
		if(rw_buf[2] == 'F') {rc = -1;}
		else {
			char* saveptr;
			char* p;
			p = strtok_r(rw_buf, " ", &saveptr); // 'U'
			p = strtok_r(NULL, " ", &saveptr); // return number
			if(p == NULL) {rc = -1;}
			else {rc = is_num(p) ? atoi(p) : -1;}
			if(rc == 0) {
				p = strtok_r(NULL, " ", &saveptr); // ino_id
				int ino_id = -1;
				if(p != NULL) ino_id = is_num(p) ? atoi(p) : -1;
				if(ino_id >= 0 && ino_id < MAX_SUB) {
					sub_group[ino_id].is_used = false;
					sub_group[ino_id].callback = NULL;
				}
				else {rc = -1;}
			}
		}
	}
	else {rc = -1;}
	return rc;
}

int iot_mqtt_client::yield() {
	int rc = 0;
	exec_cmd("z\n", true, false); // tell the python runtime to lock the current msg queue size
	if(strcmp(rw_buf, "Z T") != 0) {rc = -1;} // broken protocol
	else { // start the BIG yield loop
		while(true) {
			exec_cmd("y\n", true, false);
			if(strcmp(rw_buf, "Y F") == 0) {break;}
			if(rw_buf[0] != 'Y') { // filter out garbage feedback
				rc = -1;
				break;
			}
			// From here, there is a new message chunk in rw_buf
			char* saveptr;
			char* p;
			p = strtok_r(rw_buf, " ", &saveptr); // 'Y'
			p = strtok_r(NULL, " ", &saveptr); // ino_id
			if(p != NULL) {
			  	int ino_id = is_num(p) ? atoi(p) : -1;
			    int id_len = strlen(p);
			    p = strtok_r(NULL, " ", &saveptr); // more chunk?
			    if(p != NULL) {
			      	int more = is_num(p) ? atoi(p) : -1;
			      	if(more != 1 && more != 0) { // broken protocol
			      		rc = -1;
			      		break;
			      	}
			      	else {
			      		char* payload = rw_buf + id_len + 5; // step over the protocol and get payload
			      		strcat(msg_buf, payload); // is msg_buf is "\0dfwsadwer", this should still work as if msg_buf is new
			      		if(more == 0) { // This is the end of this message, do callback! CLEAN UP!
						    // user callback, watch out for ino_id boundary issue and callback registration
						    if(ino_id >= 0 && ino_id < MAX_SUB && sub_group[ino_id].is_used && sub_group[ino_id].callback != NULL) {
								sub_group[ino_id].callback(msg_buf, strlen(msg_buf));
						    }
						    // clean up
						    msg_buf[0] = '\0'; // mark msg_buf as 'unused', ready for the next flush
			      		}
			      		// more to come? do NOTHING to msg_buf and DO NOT call callback
			      	}
			    }
			    else {
			      	rc = -1;
			      	break;
			    }
			}
			else {
			    rc = -1;
			    break;
			}
		}
	}
	return rc;
}

int iot_mqtt_client::disconnect() {
	int rc = 0;
	exec_cmd("d\n", true, false);

	if(rw_buf[0] == 'D') {
		if(rw_buf[2] == 'F') {rc = -1;}
		else{
			char* saveptr;
			char* p;
			p = strtok_r(rw_buf, " ", &saveptr); // 'D'
			p = strtok_r(NULL, " ", &saveptr); // return number
			if(p == NULL) {rc = -1;}
			else {rc = is_num(p) ? atoi(p) : -1;}
		}
	}
	else {rc = -1;}
	return rc;
}

// Exec command and get feedback into rw_buf
void iot_mqtt_client::exec_cmd(char* cmd, bool wait, bool single_line) {
	// Write cmd
	int cnt = Serial1.write(cmd) + 1;
	timeout_flag = false;
	int timeout_sec = 0;
	// step1: forget the echo
	while(timeout_sec < CMD_TIME_OUT && cnt != 0) {
		if(Serial1.read() != -1) {cnt--;}
		else { // only start counting the timer when the serial1 is keeping us waiting...
			delay(100); // 0.1 sec
			timeout_sec++;
		}
	}
	timeout_flag = timeout_sec == CMD_TIME_OUT; // Update timeout flag
	int ptr = 0;
	if(!timeout_flag) { // step 1 clear
		timeout_flag = false;
		timeout_sec = 0;
		// step2: waiting
		delay(10);
		if(wait) {
			while(timeout_sec < CMD_TIME_OUT && !Serial1.available()) {
				delay(100); // 0.1 sec
				timeout_sec++;
			}
		}
		timeout_flag = timeout_sec == CMD_TIME_OUT; // Update timeout flag
		if(!timeout_flag) { // step 2 clear
			// read feedback
		        // will read all the available data in Serial1 but only store the message with the limit of MAX_BUF_SIZE
			bool stop_sign = false;
			while(Serial1.available()) {
				int cc = Serial1.read();
				if(cc != -1) {
					if(cc == 10 || ptr == MAX_BUF_SIZE - 1) {
						stop_sign = true;
						if(single_line) {break;}
					} // end of feedback
					if(!stop_sign && cc != RETURN_KEY) {
						rw_buf[ptr++] = (char)cc;
					}
					// delayMicroseconds(150);
				}
			}
		}
	}
	timeout_flag = false; // Clear timeout flag
	rw_buf[ptr] = '\0'; // add terminator in case of garbage data in rw_buf
}

bool iot_mqtt_client::is_num(char* src) {
	bool rc = true;
	if(src == NULL) {rc = false;}
	else {
		char* p = src;
		while(*p != '\0') {
			if(*p > '9' || *p < '0') {
				rc = false;
				break;
			}
			p++;
		}
	}
	return rc;
}