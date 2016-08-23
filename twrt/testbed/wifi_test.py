import socket
import time

msg_header = 'AADD'
msg_stamp = '\x00\x00\x00\x00'
msg_id_gw = '00AA00DD'
msg_id_dev = '00000000'
msg_devtype = '\x00\x00'

msg_auth_key = '88888888' 
msg_auth_datatype = '\x1c\x00'

msg_auth = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_auth_datatype+'\x00\x08'+msg_auth_key


msg_set_ap_datatype = '\x6a\x00'
msg_set_sta_datatype = '\x6b\x00'
msg_req_server_conn_datatype = '\x1d\x00'
msg_ack_server_conn_datatype = '\x47\x00'

ap_ssid = 'TWRT_1'+'\x00'*26
ap_ssid_len = '\x06'
ap_key = '12345678'+'\x00'*24
ap_key_len = '\x08'

sta_ssid = 'WSN_520_1'+'\x00'*23
sta_ssid_len = '\x09'
sta_key = 'wsn520520520'+'\x00'*20
sta_key_len = '\x0c'

msg_set_ap = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_set_ap_datatype+'\x00\x42'+ap_ssid_len+ap_ssid+ap_key_len+ap_key
msg_set_sta = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_set_sta_datatype+'\x00\x42'+sta_ssid_len+sta_ssid+sta_key_len+sta_key


msg_req_server_conn = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+msg_req_server_conn_datatype+'\x00\x01'+'\x00'

LEN_BUFF_IO = 5000 

serverAddress = ('192.168.0.102', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)


length = skt.send(msg_auth)
msg_bak = skt.recv(1024)
print msg_bak


length = skt.send(msg_set_sta)
print length

#time.sleep(15)

#length = skt.send(msg_req_server_conn)
#msg_bak = skt.recv(1024)
#print msg_bak

#skt.close()
while(True):
    pass
