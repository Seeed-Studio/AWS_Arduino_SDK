#!/usr/bin/expect -f
# connect via scp

set timeout -1
# modify the following line to contain your board's IP address
spawn scp -r ./certs aws_iot_mqtt_client.py root@[your_boards_IP]:/root/
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
# modify the following to your own board's password (default: "seeeduino")
exp_send "[your_password]\r"
}
}
interact