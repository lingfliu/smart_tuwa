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
for i in range(0,1): 
    sce_type_val = int(math.ceil(random.random()*3))
    sce_type = '%c'%sce_type_val

    sce_id_major_val = round(random.random()*1000)
    sce_id_major = '%08d'%sce_id_major_val

    sce_id_minor_val = round(random.random()*1000)
    sce_id_minor = '%08d'%sce_id_minor_val

    sce_mac_val= round(random.random()*1000)
    sce_mac = '%08d'%sce_mac_val

    sce_name_val = round(random.random()*100)
    sce_name = 'scene'+'%04d'%sce_name_val + '\x00'*51

    sce_type_val = int(math.ceil(random.random()*4))
    sce_type = '%c'%sce_type_val
    sce_type +='\x00'*3

    sce_trigger_num = 100#int(random.random()*100)
    sce_trigger = ''
    for m in range(0, sce_trigger_num):
        sce_trigger_val = round(random.random()*100)
        sce_trigger += ('%08d'%sce_trigger_val)*6

    sce_item_num = 100 #int(random.random()*100)
    sce_item = ''
    for m in range(0, sce_item_num):
        sce_item_val = round(random.random()*100) 
        sce_item += ('%08d'%sce_item_val)*6 
        body_len_val = 48*sce_item_num + 48*sce_trigger_num + 96 
        body_len = ''
        body_len +='%c'%(int(body_len_val/256))
        body_len +='%c'%(body_len_val%256)

    msg_set_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0f\x00'+ body_len + sce_id_major +sce_id_minor+sce_mac+sce_type+sce_name+'%c'%sce_trigger_num + '\x00'*3+'%c'%sce_item_num+'\x00'*3+sce_trigger+sce_item

            #print('message length=' + str(len(msg_set_scene)))
            #print('body length=' + str(body_len_val))
    print (sce_id_major + ' ' + sce_id_minor + ' ' + sce_mac + ' ' + sce_name + ' ' + str(sce_trigger_num) + ' ' + str(sce_item_num) )
            #print(str('%c'%sce_trigger_num))
            #print(body_len)

            #print('msg = ' + msg_set_scene)
    m = 0
    while(True):
        if m+256 < len(msg_set_scene):
            pkt = msg_set_scene[m:m+256]
            length = skt.send(pkt)
            print length
            m += 256
            time.sleep(0.01)
            continue
        else:
            pkt = msg_set_scene[m:]
            length = skt.send(pkt)
            time.sleep(0.01)
            print length
            break
    #length = skt.send(msg_set_scene())

    msg_bak = skt.recv(1024)
    print msg_bak
    #time.sleep(5)

msg_finish_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x11\x00'+'\x00\x01' + '\x00'
            
print('msg finish = ' + msg_finish_scene)
length = skt.send(msg_finish_scene)
print length
msg_bak = skt.recv(1024)
print msg_bak

#while(True):
    #msg_bak = skt.recv(1024)
    #print msg_bak
    #pass

