import socket
import time
import sys
import random
import math
import threading

HOST = ''
PORT = 8089
skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
	skt.bind( (HOST, PORT) )
except socket.error as msg:
	print 'Bind failed. Error Code: ' + str(msg[0]) + 'Message' + msg[1]
	sys.exit()

skt.listen(10)


def work(skt):

    msg_header = 'AADD'
    msg_stamp = '\x00\x00\x00\x00'
    msg_id_gw = 'carbonx1' 
    msg_id_dev = '00000000' 
    msg_devtype = '\x00\x00' 
    msg_auth_ack_datatype = '\x3e\x00'

    msg_auth_ack = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_ack_datatype+'\x00\x08'+msg_id_gw


    dev_mac_base = 1 
    sce_id_major_base = 1
    sce_id_minor_base = 1

    #Authorization
    len = skt.send(msg_auth_ack)
    print len
    msg_bak = skt.recv(1024)
    print msg_bak

    #control test
    for i in range(1, 500):
        dev_mac_idx = round(random.random()*5000)
        dev_mac = '%08d'%dev_mac_idx
        dev_type_val = round(random.random()*100)
        dev_type = '\x02\x00'
        dev_status_len = '\x00\x04'
        dev_status = '0101'

        msg_dev_ctrl = msg_header + msg_stamp + msg_id_gw+dev_mac+dev_type+'\x02\x00'+dev_status_len+dev_status
        print('msg = ' + msg_dev_ctrl)
        len = skt.send(msg_dev_ctrl)
        print len
        msg_bak = skt.recv(1024)
        print msg_bak

while(True):
	conn, addr = skt.accept()
	print 'connection established from : ' + str(addr)
        #work_thread = threading.Thread(target = work, args = (conn,))
        #work_thread.start()

        msg_header = 'AADD'
        msg_stamp = '\x00\x00\x00\x00'
        msg_id_gw = 'carbonx1' 
        msg_id_dev = '00000000' 
        msg_devtype = '\x00\x00' 
        msg_auth_ack_datatype = '\x3e\x00'

        msg_auth_ack = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_ack_datatype+'\x00\x08'+msg_id_gw


        dev_mac_base = 1 
        sce_id_major_base = 1
        sce_id_minor_base = 1

        #Authorization
        len = conn.send(msg_auth_ack)
        print len
        msg_bak = conn.recv(1024)
        print msg_bak
        '''
        #control test
        for i in range(0, 100):
            dev_mac_idx = round(random.random()*5000)
            dev_mac = '%08d'%dev_mac_idx
            dev_type_val = round(random.random()*100)
            dev_type = '\x02\x00'
            dev_status_len_val = int(math.ceil(random.random()*8))
            dev_status_len = '\x00'+'%c'%dev_status_len_val
            dev_status = ''
            for m in range(0,dev_status_len_val):
                status_val = int(round(random.random()*100))
                dev_status += '%c'%status_val
            msg_dev_ctrl = msg_header + msg_stamp + msg_id_gw+dev_mac+dev_type+'\x02\x00'+dev_status_len+dev_status
            print('msg = ' + msg_dev_ctrl)
            len = conn.send(msg_dev_ctrl)
            print len
            time.sleep(0.1)
            '''
        #scene set
        for i in range(0,100): 
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

            sce_trigger_num = int(math.ceil(random.random()*20))
            sce_trigger = ''
            for m in range(0, sce_trigger_num):
                 sce_trigger_val = round(random.random()*100)
                 sce_trigger += ('%08d'%sce_trigger_val)*6

            sce_item_num = int(round(random.random()*20))
            sce_item = ''
            for m in range(1, sce_item_num):
                sce_item_val = round(random.random()*100) 
                sce_item += ('%08d'%sce_trigger_val)*6 
            body_len_val = 48*sce_item_num + 48*sce_trigger_num + 96 
            body_len = ''
            if (body_len_val > 256): 
                body_len +='%c'%(int(body_len_val/256))
                body_len +='%c'%(body_len_val%256)
            else:
                body_len +='\x00'
                body_len +='%c'%(body_len_val)


            msg_set_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0f\x00'+ body_len + sce_id_major +sce_id_minor+sce_mac+sce_type+sce_name+'%c'%sce_trigger_num + '\x00'*3+'%c'%sce_item_num+'\x00'*3+sce_trigger+sce_item

            print (sce_id_major + ' ' + sce_id_minor + ' ' + sce_mac + ' ' + sce_name + ' ' + str(sce_trigger_num) + ' ' + str(sce_item_num) )
            print(str('%c'%sce_trigger_num))

            #print('msg = ' + msg_set_scene)
            len = conn.send(msg_set_scene)
            print len
            #msg_bak = conn.recv(1024)
            #print msg_bak
            time.sleep(0.2)

        msg_finish_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x11\x00'+'\x00\x01' + '\x00'
            
        print('msg finish = ' + msg_finish_scene)
        len = conn.send(msg_finish_scene)
        print len

        conn.close()
        skt.close()
        break
        #time.sleep(5)
