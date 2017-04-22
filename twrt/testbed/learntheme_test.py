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

#scene set
for i in range(0,20): 
    mac = '' 
    for m in range(0,8):
        mac_val = round(random.random()*255)
        mac +='%c'%(int(mac_val))

    state =''
    for m in range(0,8):
        state_val = round(random.random()*100)
        state +='%c'%(int(mac_val))

    type_str = ''
    type_val = round(random.random()*10)
    type_str +='%c'%(int(type_val))

    msg_znode_stat = msg_header + msg_stamp + msg_id_gw + mac + type_str + '\x00' + '\x01\x00' + '\x00\x08' + state

    print('create virtual znode ' + str(i) + ': mac = ' + str(mac) + ' type = ' + str(type_str) + ' state = ' + str(state) ) 
    m = 0
    while(True):
        if m+256 < len(msg_znode_stat):
            pkt = msg_znode_stat[m:m+256]
            length = skt.send(pkt)
            print length
            m += 256
            time.sleep(0.01)
            continue
        else:
            pkt = msg_znode_stat[m:]
            length = skt.send(pkt)
            time.sleep(0.01)
            print length
            break

    msg_bak = skt.recv(1024)
    print msg_bak
    time.sleep(0.01)


#create learn them ctrl
mac = '' 
for m in range(0,8):
    #mac_val = round(random.random()*255)
    #mac +='%c'%(int(mac_val))
    mac += '\x31'

state =''
for m in range(0,8):
    state_val = round(random.random()*100)
    #state +='%c'%(int(mac_val))
    if m == 1:
        state +='\x32'
    else:
        state += '\x00'

type_str = ''
type_val = round(random.random()*10)
#type_str +='%c'%(int(type_val))
type_str = '\xca'

msg_znode_stat = msg_header + msg_stamp + msg_id_gw + mac + type_str + '\x00' + '\x01\x00' + '\x00\x08' + state

print('create theme learn' + str(i) + ': mac = ' + str(mac) + ' type = ' + str(type_str) + ' state = ' + str(state) ) 
m = 0
while(True):
    if m+256 < len(msg_znode_stat):
        pkt = msg_znode_stat[m:m+256]
        length = skt.send(pkt)
        print length
        m += 256
        time.sleep(0.01)
        continue
    else:
        pkt = msg_znode_stat[m:]
        length = skt.send(pkt)
        time.sleep(0.01)
        print length
        break

msg_bak = skt.recv(1024)
print msg_bak


#while(True):
    #msg_bak = skt.recv(1024)
    #print msg_bak
    #pass
    
