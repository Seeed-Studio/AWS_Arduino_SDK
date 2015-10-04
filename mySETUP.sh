#!/usr/bin/expect -f
set timeout -1
# modify the following to your board ip
spawn ssh root@[your_boards_IP]
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
# modify the following to your board password
exp_send "[your_password]\r"
}
}
expect "*~#" {send "opkg update\r"}
expect "*~#" {send "opkg install distribute\r"}
expect "*~#" {send "opkg install python-openssl\r"}
expect "*~#" {send "easy_install pip\r"}
expect "*~#" {send "pip install paho-mqtt\r"}
expect "*~#" {send "exit\r"}
interact