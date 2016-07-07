import socket
import time

msg_header = 'AADD'
msg_stamp = '\x00\x00\x00\x00'
msg_id_gw = 'carbonx1'
msg_id_dev = '00000000'
msg_devtype = '\x01\x00'

msg_auth_key = 'TWRT2015' 
msg_auth_datatype = '\x1c\x00'

msg_auth = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_datatype+'\x00\x08'+msg_auth_key

dev_1_mac = '00000001'
dev_1_status = '01'
dev_1_type = '\x01\x00'
dev_2_mac = '00000002'
dev_2_status = '02'
dev_2_type = '\x02\x00'
dev_3_mac = '00000003'
dev_3_status = '03'
dev_3_type = '\x03\x00'
dev_status_len = '\x00\x02'

msg_dev_1_stat = msg_header + msg_stamp + msg_id_gw+dev_1_mac+dev_1_type+'\x01\x00'+dev_status_len+dev_1_status
msg_dev_2_stat = msg_header + msg_stamp + msg_id_gw+dev_2_mac+dev_2_type+'\x01\x00'+dev_status_len+dev_2_status
msg_dev_3_stat = msg_header + msg_stamp + msg_id_gw+dev_3_mac+dev_3_type+'\x01\x00'+dev_status_len+dev_3_status


msg_dev_1_ctrl = msg_header + msg_stamp + msg_id_gw+dev_1_mac+dev_1_type+'\x02\x00'+dev_status_len+dev_1_status
msg_dev_2_ctrl = msg_header + msg_stamp + msg_id_gw+dev_2_mac+dev_2_type+'\x02\x00'+dev_status_len+dev_2_status
msg_dev_3_ctrl = msg_header + msg_stamp + msg_id_gw+dev_3_mac+dev_3_type+'\x02\x00'+dev_status_len+dev_3_status

dev_theme_mac = '00000004'
dev_theme_status = '\x320000000'
dev_theme_status_len = '\x00\x08'
dev_theme_type = '\xca\x00'
msg_theme_learn_stat = msg_header + msg_stamp + msg_id_gw+dev_theme_mac+dev_theme_type+'\x01\x00'+dev_theme_status_len+dev_theme_status


LEN_BUFF_IO = 1000

serverAddress = ('localhost', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)


len = skt.send(msg_auth)
msg_bak = skt.recv(1024)
print msg_bak


for i in range(1, 200):
    len = skt.send(msg_dev_1_stat) 
    msg_bak = skt.recv(1024)
    print msg_bak

    len = skt.send(msg_dev_2_stat)
    msg_bak = skt.recv(1024)
    print msg_bak

    len = skt.send(msg_dev_3_stat)
    msg_bak = skt.recv(1024)
    print msg_bak

    skt.send(msg_dev_1_ctrl) 

    skt.send(msg_dev_2_ctrl)

    skt.send(msg_dev_3_ctrl)

    len = skt.send(msg_theme_learn_stat)
    msg_bak = skt.recv(1024)
    print msg_bak

