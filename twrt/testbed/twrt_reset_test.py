import socket
import time
import sys
import random
import math
import threading


msg_header = 'AADD'
msg_stamp = '\x00\x00\x00\x00'
msg_id_gw = '2016A008'
msg_id_dev = '00000000'
msg_devtype = '\x01\x00'

msg_auth_key = '88888888' 
msg_auth_datatype = '\x1c\x00'

msg_auth = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_datatype+'\x00\x08'+msg_auth_key


#serverAddress = ('192.168.20.104', 9091)
serverAddress = ('localhost', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)

length = skt.send(msg_auth)
msg_bak = skt.recv(1024)
print msg_bak

msg_reset_datatype = '\x65\x00'
msg_reset = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_reset_datatype+'\x00\x01'+'0'

length = skt.send(msg_reset)
skt.close()

#while(True):
    #msg_bak = skt.recv(1024)
    #print msg_bak
    #pass
    
