import socket
import time
import random

msg_header = 'AADD'
msg_stamp = '\x00\x00\x00\x00'
msg_id_gw = '2016A002'
msg_id_dev = '00000000'
msg_devtype = '\x01\x00'

msg_auth_key = 'TWRT2015' 
msg_auth_datatype = '\x1c\x00'

msg_auth = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_datatype+'\x00\x08'+msg_auth_key


#create trigger data
trigger_mac = []
trigger_state = []
trigger_state_len = 8
trigger_type = '\x6e'

for i in range(0, 8): #8 triggers
    mac = '' 
    for m in range(0,8):
        mac_val = round(random.random()*255)
        mac +='%c'%(int(mac_val))
    trigger_mac.append(mac)

    state =''
    for m in range(0,8):
        state_val = round(random.random()*100)
        state +='%c'%(int(mac_val))
    trigger_state.append(state)

#create scene data
sce_id_major = []
sce_id_minor = []
sce_mac = []

msg_set_scene = []

for i in range(0,10): 
    sce_id_major_val = round(random.random()*100000)
    sce_id_major_str = '%08d'%sce_id_major_val
    sce_id_major.append(sce_id_major_str)

    sce_id_minor_val = round(random.random()*100000)
    sce_id_minor_str = '%08d'%sce_id_minor_val
    sce_id_minor.append(sce_id_minor_str)

    sce_mac_val= round(random.random()*100000)
    sce_mac_str = '%08d'%sce_mac_val
    sce_mac.append(sce_mac_str)

    sce_name_val = round(random.random()*100)
    sce_name = 'scene'+'%04d'%sce_name_val + '\x00'*51

    sce_type = '\x03' + '\x00'*3

    sce_trigger_num = 1
    sce_trigger_idx = int(round(random.random()*7))

    sce_trigger = ''
    sce_trigger += trigger_mac[sce_trigger_idx]
    sce_trigger += trigger_state[sce_trigger_idx]
    sce_trigger += '\x00'*24
    sce_trigger += '\x08'+'\x00'*3
    sce_trigger += trigger_type + '\x00'*3

    sce_item_num = int(random.random()*20)
    sce_item = ''
    for m in range(0, sce_item_num): 
        mac = '' 
        for n in range(0,8):
            mac_val = round(random.random()*255)
            mac +='%c'%(int(mac_val))
        state =''
        for n in range(0,8):
            state_val = round(random.random()*100)
            state +='%c'%(int(state_val))
        sce_item_type_val = round(random.random()*255)
        sce_item_type = '%c'%(int(sce_item_type_val))

        sce_item += mac + state+'\x00'*24 + '\x08'+'\x00'*3 + sce_item_type + '\x00'*3

    body_len_val = 48*sce_item_num + 48*sce_trigger_num + 96 
    body_len = ''
    body_len +='%c'%(int(body_len_val/256))
    body_len +='%c'%(body_len_val%256)

    msg_set_scene.append(msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0f\x00'+ body_len + sce_id_major[i] +sce_id_minor[i] +sce_mac[i] +sce_type+sce_name+'%c'%sce_trigger_num + '\x00'*3+'%c'%sce_item_num+'\x00'*3+sce_trigger+sce_item)


#create trigger stat data 
msg_stat = []
for i in range(0,8):
    msg_stat_str = msg_header + msg_stamp + msg_id_gw+trigger_mac[i]+trigger_type+'\x00'+'\x01\x00'+'\x00\x08'+trigger_state[i]
    #print(str(len(trigger_state[i])))
    msg_stat.append(msg_stat_str)

LEN_BUFF_IO = 5000 

serverAddress = ('localhost', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)


length = skt.send(msg_auth)
msg_bak = skt.recv(1024)
print msg_bak


for i in range(0, 8):
#    print(msg_set_scene[i])
    length = skt.send(msg_set_scene[i]) 
    time.sleep(0.1)
    #pass


#msg_bak = skt.recv(1024)
#print 'received msg' 
#print msg_bak

time.sleep(0.1)
for i in range(0,8):
    length = skt.send(msg_stat[i]) 
    time.sleep(0.1)
    print('sending message: ' + str(length))

#skt.close()
while(True):
    pass
