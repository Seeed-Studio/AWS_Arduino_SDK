# Python 2.7.3
import time
import signal
import sys
import ssl
import thread
import Queue
import threading
import paho.mqtt.client as mqtt

# conventions
###################################
'''
On python side, for feedback to Arduino:
'I' - constructor
'G' - config
'C' - connect
'P' - publish
'D' - disconnect
'S' - subscribe
'U' - unsubscribe
'Y' - yieldMessage
'Z' - lockQueueSize
*Incoming string starting with the corresponding lower case letter represents the corresponding requests
'''
# marco
###################################
MAX_CONN_TIME = 10
EXIT_TIME_OUT = 25
CHUNK_SIZE = 50 # 50 bytes as a chunk
YIELD_METADATA_SIZE = 5 # 'Y ' + ' <more?> ': 2 + 3

# helper function
###################################
def interrupted():
    raise Exception

# debug func
###################################
def get_input(debug, buf):
    if(debug): # read from the given buffer
        terminator = buf[0].find('\n')
        if(len(buf[0]) != 0 and terminator != -1):
            ret = buf[0][0:terminator]
            buf[0] = buf[0][(terminator+1):]
            return ret
        else:  # simulate no-input blocking
            while 1:
                pass
    else:
        return raw_input() # read from stdin

def send_output(debug, buf, content):
    if(debug): # write to the given buffer
        buf[0] = buf[0][:0] + content[0:]
    else:
        print(content) # write to stdout

# callbacks
###################################
def on_connect(client, userdata, flags, rc):
    userdata.conn_res = rc

def on_disconnect(client, userdata, rc):
    userdata.disconn_res = rc

def on_message(client, userdata, msg):
    userdata.idMap_lock.acquire()
    try:
        for key in userdata.idMap.keys():
            if(mqtt.topic_matches_sub(key, str(msg.topic))): # check for wildcard matching
                ino_id = userdata.idMap[key]
                userdata.msgQ.put(str(ino_id) + " " + str(msg.payload)) # protocol-style convention needed
    except BaseException as e: # ignore clean session = false: msg from pre-subscribed topics
        pass
    userdata.idMap_lock.release()

class iot_mqtt_client:
    # client handler
    _iot_mqtt_client_handler = None
    # server information
    _serverURL = "g.us-east-1.pb.iot.amazonaws.com"
    _serverPORT = 8883
    # certs
    _cafile = "./certs/aws-iot-rootCA.crt"
    _key = "./certs/privkey.pem"
    _cert = "./certs/cert.pem"
    # connect result, need to access in callback
    conn_res = -1
    # disconnect result, need to access in callback
    disconn_res = -1
    # message queue
    msgQ = Queue.Queue(0)
    # ino_id mapping
    idMap = {}
    # debug
    wrapper_debug = False
    wrapper_Tx = None
    # lock
    idMap_lock = threading.Lock()
    # internal message buffer and ino_id holder
    _dynamic_str = '' # empty string
    _dynamic_ino_id = -1
    _dynamic_queue_size = 0

    # robust wrapper
    ###################################
    def config(self, src_serverURL, src_serverPORT, src_cafile, src_key, src_cert):
        if(src_serverPORT != None):
            self._serverURL = src_serverURL
        if(src_serverPORT != None):
            self._serverPORT = int(src_serverPORT)
        if(src_cafile != None):
            self._cafile = src_cafile
        if(src_key != None):
            self._key = src_key
        if(src_cert != None):
            self._cert = src_cert
        send_output(self.wrapper_debug, self.wrapper_Tx, "G T")

    def __init__(self, id, clean_session, protocol):
        try:
            self._iot_mqtt_client_handler = mqtt.Client(id, clean_session, self, protocol)
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "I F " + e.message)
            return
        self._iot_mqtt_client_handler.on_connect = on_connect
        self._iot_mqtt_client_handler.on_disconnect = on_disconnect
        self._iot_mqtt_client_handler.on_message = on_message
        send_output(self.wrapper_debug, self.wrapper_Tx, "I T")

    def connect(self, keepalive=60):
        # tls
        try:
            self._iot_mqtt_client_handler.tls_set(self._cafile, self._cert, self._key, ssl.CERT_REQUIRED, ssl.PROTOCOL_SSLv23)
        except ValueError as ve:
            send_output(self.wrapper_debug, self.wrapper_Tx, "C F " + ve.message)
            return
        except:
            send_output(self.wrapper_debug, self.wrapper_Tx, "C F TLS Error")
            return

        # connect
        try:
            self._iot_mqtt_client_handler.connect(self._serverURL, self._serverPORT, keepalive)
            self._iot_mqtt_client_handler.loop_start()
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "C F " + e.message)
            return

        cnt_sec = 0
        while(cnt_sec < MAX_CONN_TIME and self.conn_res == -1): # waiting for connecting to complete (on_connect)
            cnt_sec += 1
            time.sleep(1)

        if(self.conn_res != -1):
            send_output(self.wrapper_debug, self.wrapper_Tx, "C " + str(self.conn_res) + " " + mqtt.connack_string(self.conn_res)) # 0 for connected
        else:
            send_output(self.wrapper_debug, self.wrapper_Tx, "C F Connection time out")
        return self.conn_res

    def publish(self, topic, payload, qos, retain):
        try:
            (rc, mid) = self._iot_mqtt_client_handler.publish(topic, payload, qos, retain)
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "P F " + e.message)
            return
        send_output(self.wrapper_debug, self.wrapper_Tx, "P " + str(rc) + " " + mqtt.error_string(rc))
        return rc

    def disconnect(self):
        try:
            self._iot_mqtt_client_handler.disconnect()
            self._iot_mqtt_client_handler.loop_stop()
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "D F " + e.message)
            return

        cnt_sec = 0
        while(cnt_sec < MAX_CONN_TIME and self.disconn_res == -1): # waiting for on_disconnect
            cnt_sec += 1
            time.sleep(1)

        if(self.disconn_res != -1):
            send_output(self.wrapper_debug, self.wrapper_Tx, "D " + str(self.disconn_res) + " " + mqtt.error_string(self.disconn_res))
        else:
            send_output(self.wrapper_debug, self.wrapper_Tx, "D F Disconnection time out")
        return self.disconn_res

    def subscribe(self, topic, qos, ino_id):
        try:
            (rc, mid) = self._iot_mqtt_client_handler.subscribe(topic, qos)
            if ino_id == None:
                raise ValueError("None ino_id")
            self.idMap_lock.acquire()
            self.idMap.setdefault(topic, ino_id)
            self.idMap_lock.release()
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "S F " + e.message)
            return
        send_output(self.wrapper_debug, self.wrapper_Tx, "S " + str(rc) + " "  + mqtt.error_string(rc)) 
        return rc

    def unsubscribe(self, topic):
        try:
            (rc, mid) = self._iot_mqtt_client_handler.unsubscribe(topic)
            self.idMap_lock.acquire()
            ino_id = self.idMap[topic]
            del self.idMap[topic]
            self.idMap_lock.release()
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "U F " + e.message)
            return
        send_output(self.wrapper_debug, self.wrapper_Tx, "U " + str(rc) + " " + str(ino_id) + " " + mqtt.error_string(rc))
        # send back the return value along with the ino_id for C side reference to free the subgroup slot (important)
        return rc

    def lockQueueSize(self):
        # make sure nothing is hapenning in between
        # this would be the number of messages to be processed in the coming yield
        self._dynamic_queue_size = self.msgQ.qsize()
        send_output(self.wrapper_debug, self.wrapper_Tx, "Z T") # finish with the locking the queue size

    def yieldMessage(self):
        try:
            # No more message to echo/Nothing left from the previous message
            if(self._dynamic_queue_size == 0 and len(self._dynamic_str) == 0):
                # do a clean-up
                self._dynamic_str = ''
                self._dynamic_queue_size = 0
                self._dynamic_ino_id = -1
                # send feedback
                send_output(self.wrapper_debug, self.wrapper_Tx, "Y F")
            # We have something to echo. Do it chunk by chunk
            else:
                # Nothing left from the previous message, start a new one
                if(len(self._dynamic_str) == 0):
                    self._dynamic_str = self.msgQ.get()
                    temp_split = self._dynamic_str.split(' ', 1)
                    self._dynamic_ino_id = int(temp_split[0]) # get ino_id
                    self._dynamic_queue_size -= 1
                    self._dynamic_str = temp_split[1] # pure message body
                # See if we need to split it
                string2send = None
                more = 0
                if(len(self._dynamic_str) + YIELD_METADATA_SIZE + len(str(self._dynamic_ino_id))> CHUNK_SIZE):
                    more = 1 # there is going to be more chunks coming...
                    stoppoint = CHUNK_SIZE - YIELD_METADATA_SIZE - len(str(self._dynamic_ino_id))
                    string2send = self._dynamic_str[0:stoppoint]
                    self._dynamic_str = self._dynamic_str[stoppoint:] # update dynamic string
                else: # last chunk
                    string2send = self._dynamic_str
                    self._dynamic_str = '' # clear it because it has been sent
                # deliver only one chunk for one yield request
                # Y <ino_id> <more?> <message chunk>
                send_output(self.wrapper_debug, self.wrapper_Tx, "Y " + str(self._dynamic_ino_id) + " " + str(more) + " " + string2send)
        except BaseException as e:
            send_output(self.wrapper_debug, self.wrapper_Tx, "Y F " + e.message)


# main func
###################################
signal.signal(signal.SIGALRM, interrupted)
def runtime_func(debug, buf_i, buf_o, mock):
    iot_mqtt_client_obj = None
    cmd_set = set(['i', 'g', 'c', 'p', 'd', 's', 'u', 'y', 'z', '~'])
    try:
        while True:
            # read user input
            signal.alarm(EXIT_TIME_OUT)

            command_type = 'x'
            command_type = get_input(debug, buf_i)

            if(command_type in cmd_set):

                signal.alarm(EXIT_TIME_OUT)

                if(command_type != 'i' and iot_mqtt_client_obj == None):
                    send_output(debug, buf_o, "X no setup")

                elif(command_type == 'i'):
                    src_id = get_input(debug, buf_i)
                    try:
                        src_cleansession = False if(int(get_input(debug, buf_i)) == 0) else True
                    except:
                        src_cleansession = None
                    try:
                        src_protocol = mqtt.MQTTv311 if(int(get_input(debug, buf_i)) == 4) else mqtt.MQTTv31
                    except:
                        src_protocol = None
                    # function call
                    if(not debug):
                        iot_mqtt_client_obj = iot_mqtt_client(src_id, src_cleansession, src_protocol)
                    else:
                        iot_mqtt_client_obj = mock
                elif(command_type == 'g'):
                    src_serverURL = get_input(debug, buf_i)
                    src_serverPORT = get_input(debug, buf_i)
                    src_cafile = get_input(debug, buf_i)
                    src_key = get_input(debug, buf_i)
                    src_cert = get_input(debug, buf_i)
                    # function call
                    iot_mqtt_client_obj.config(src_serverURL, src_serverPORT, src_cafile, src_key, src_cert)
                elif(command_type == 'c'):
                    try:
                        src_keepalive = int(get_input(debug, buf_i))
                    except:
                        src_keepalive = None
                    # function call
                    iot_mqtt_client_obj.connect(src_keepalive)
                elif(command_type == 'p'):
                    src_topic = get_input(debug, buf_i)
                    src_payload = get_input(debug, buf_i)
                    try:
                        src_qos = int(get_input(debug, buf_i))
                    except:
                        src_qos = None
                    try:
                        src_retain = False if(int(get_input(debug, buf_i)) == 0) else True
                    except:
                        src_retain = None
                    # function call
                    iot_mqtt_client_obj.publish(src_topic, src_payload, src_qos, src_retain)
                elif(command_type == 's'):
                    src_topic = get_input(debug, buf_i)
                    try:
                        src_qos = int(get_input(debug, buf_i))
                    except:
                        src_qos = None
                    try:
                        src_ino_id = int(get_input(debug, buf_i))
                    except:
                        src_ino_id = None
                    # function call
                    iot_mqtt_client_obj.subscribe(src_topic, src_qos, src_ino_id)
                elif(command_type == 'u'):
                    src_topic = get_input(debug, buf_i)
                    # function call
                    iot_mqtt_client_obj.unsubscribe(src_topic)
                elif(command_type == 'y'):
                    # function call
                    iot_mqtt_client_obj.yieldMessage()
                elif(command_type == 'd'):
                    # function call
                    iot_mqtt_client_obj.disconnect()
                elif(command_type == 'z'):
                    # function call
                    iot_mqtt_client_obj.lockQueueSize()
                elif(command_type == '~'): # for debug
                    break

            else:
                pass
    except:
        send_output(debug, buf_o, "X cmd timeout")
    pass

# execute
##################################
runtime_func(False, None, None, None)

