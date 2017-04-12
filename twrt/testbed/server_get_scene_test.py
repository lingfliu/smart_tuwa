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
    msg_id_gw = '2016A011' 
    msg_id_dev = '00000000' 
    msg_devtype = '\x00\x00' 
    msg_auth_ack_datatype = '\x3e\x00'

    msg_auth_ack = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_ack_datatype+'\x00\x08'+msg_id_gw


    dev_mac_base = 1 
    sce_id_major_base = 1
    sce_id_minor_base = 1

    #Authorization
    length = skt.send(msg_auth_ack)
    print length
    msg_bak = skt.recv(1024)
    print msg_bak


while(True):
	conn, addr = skt.accept()
	print 'connection established from : ' + str(addr)
        #work_thread = threading.Thread(target = work, args = (conn,))
        #work_thread.start()

        msg_header = 'AADD'
        msg_stamp = '\x00\x00\x00\x00'
        msg_id_gw = '2016A011' 
        msg_id_dev = '00000000' 
        msg_devtype = '\x00\x00' 
        msg_auth_ack_datatype = '\x3e\x00'

        msg_auth_ack = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_ack_datatype+'\x00\x08'+msg_id_gw


        dev_mac_base = 1 
        sce_id_major_base = 1
        sce_id_minor_base = 1

        #Authorization
        length = conn.send(msg_auth_ack)
        print length
        msg_bak = conn.recv(1024)
        print msg_bak

        #scene get all

        sce_id_major = '0'*8

        sce_id_minor = '0'*8
        body_len='\x00\x10'

        msg_get_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0e\x00'+ body_len + sce_id_major +sce_id_minor

        #print('msg = ' + msg_set_scene)
        wlength = conn.send(msg_get_scene)
        print length
        while(True):
            msg_bak = conn.recv(200000)
            print len(msg_bak)
            time.sleep(0.02)

        #conn.close()
        #skt.close()
        #break
        time.sleep(500)
