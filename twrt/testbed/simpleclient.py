import socket
import time

msg_auth = 'AADD0000carbonx10000000000\x32\x00\x42\x00\x08TWRT2015'
msg_set_install = 'AADD0000carbonx100000001\x32\x00\x06\x00\x0a\x01\x00\TWRT2015'

LEN_BUFF_IO = 1000
numSkt = 5
serverAddress = ('localhost', 9091)

skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
skt.connect(serverAddress)
len = skt.send(msg_auth)
msg_bak = skt.recv(1024)
print msg_bak
len = skt.send(msg_set_install)
msg_bak = skt.recv(1024)
print msg_bak


#skts = []
#for idx in range(numSkt):
#time.sleep(1)
#	skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#	skt.connect(serverAddress)
#	print 'socket connected'
#	skts.append(skt)

#while(True):
#	for idx in range(numSkt):
#		len = skts[idx].send('hello')
#		if len <= 0:
#			skt.close()
#			print 'disconnected'
#			break
#		else:
#			print 'send sth to server'
#			time.sleep(1)
	




