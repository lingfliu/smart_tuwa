import socket
import time

msg_header = 'AADD'
msg_stamp = '\x00\x00\x00\x00'
msg_id_gw = '00AA00DD'
msg_id_dev = '00000000'
msg_devtype = '\x33\x00'

msg_auth_key = '88888888' 
msg_auth_datatype = '\x1c\x00'

msg_auth = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_datatype+'\x00\x08'+msg_auth_key


msg_ctrl_datatype = '\x02\x00'

data_on = '\x64'
data_off = '\x00'

msg_ctrl_fan_on = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_ctrl_datatype+'\x00\x01'+data_on

msg_ctrl_fan_off = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_ctrl_datatype+'\x00\x01'+data_off

LEN_BUFF_IO = 5000 

serverAddress = ('192.168.20.100', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)


length = skt.send(msg_auth)
msg_bak = skt.recv(1024)
#print msg_bak


length = skt.send(msg_ctrl_fan_on)

time.sleep(10)

length = skt.send(msg_ctrl_fan_off)

#skt.close()
while(True):
    pass
