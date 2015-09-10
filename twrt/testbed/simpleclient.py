import socket
import time

msg_auth = 'AABBCCDD'
LEN_BUFF_IO = 1000
numSkt = 5
serverAddress = ('192.168.1.106', 9091)
skts = []
for idx in range(numSkt):
	time.sleep(1)
	skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	skt.connect(serverAddress)
	print 'socket connected'
	skts.append(skt)

while(True):
	for idx in range(numSkt):
		len = skts[idx].send('hello\n')
		if len <= 0:
			skt.close()
			print 'disconnected'
			break
		else:
			print 'send sth to server'
			time.sleep(1)
	




