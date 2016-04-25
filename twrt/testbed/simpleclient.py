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

x = 5
install_id = '00000000'
install_type = '\x02\x00\x00\x00'
install_name = 'switch001' + '\x00'*51
install_pos = 'hall' + '\x00'*56
install_postype = '\x02'+'\x00'*3

msg_set_install = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x06\x00'+'\x00\x88' + install_id+install_type+install_name+install_pos+install_postype

msg_get_install = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x07\x00'+'\x00\x08' + install_id

msg_finish_install = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0b\x00'+'\x00\x01' + '\x00'

scene_mac = '00000000'
scene_id_major = '00000011'
scene_id_minor = '00000001'
scene_type = '\x01\x00\x00\x00'
scene_name = 'scene'+'\x00'*55
scene_trigger_num = 1
scene_item_num = 1
scene_trigger = '00000011'*2
scene_item  = '00000012'*2

scene_mac2 = '00000111'
scene_id_major2 = '00000031'
scene_id_minor2 = '00000011'

msg_set_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0f\x00'+'\x00\x80' + scene_id_major+scene_id_minor+scene_mac+scene_type+scene_name+'\x01\x00\x00\x00'+'\x01\x00\x00\x00'+scene_trigger+scene_item

msg_set_scene2 = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0f\x00'+'\x00\x80' + scene_id_major2+scene_id_minor2+scene_mac2+scene_type+scene_name+'\x01\x00\x00\x00'+'\x01\x00\x00\x00'+scene_trigger+scene_item

msg_del_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x12\x00'+'\x00\x10' + scene_id_major+scene_id_minor

msg_get_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0e\x00'+'\x00\x10' + scene_id_major+scene_id_minor

msg_get_scene_null = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x0e\x00'+'\x00\x10' + '\x01'+'\x00'*7+scene_id_minor

msg_finish_scene = msg_header+msg_stamp+msg_id_gw+msg_id_dev+msg_devtype+'\x11\x00'+'\x00\x01' + '\x00'

LEN_BUFF_IO = 1000

serverAddress = ('localhost', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)

len = skt.send(msg_auth)
print len
msg_bak = skt.recv(1024)
print msg_bak

len = skt.send(msg_set_install)
print len
msg_bak = skt.recv(1024)
print msg_bak

len = skt.send(msg_get_install)
print len
msg_bak = skt.recv(1024)
print msg_bak


len = skt.send(msg_finish_install)
print len
msg_bak = skt.recv(1024)
print msg_bak

len = skt.send(msg_set_scene)
print len
msg_bak = skt.recv(1024)
print msg_bak


len = skt.send(msg_set_scene2)
print len
msg_bak = skt.recv(1024)
print msg_bak

len = skt.send(msg_get_scene)
print len
msg_bak = skt.recv(1024)
print msg_bak



len = skt.send(msg_get_scene_null)
print len
msg_bak = skt.recv(1024)
print msg_bak


len = skt.send(msg_finish_scene)
print len
msg_bak = skt.recv(1024)
print msg_bak

#len = skt.send(msg_del_scene)
#print len
#msg_bak = skt.recv(1024)
#print 'delete scene result' + msg_bak

#len = skt.send(msg_finish_scene)
#print len
#msg_bak = skt.recv(1024)
#print 'finish scene result' + msg_bak

#while True:
#	pass
