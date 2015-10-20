import socket
import time

HOST = ''
PORT = 8089
skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
	skt.bind( (HOST, PORT) )
except socket.error as msg:
	print 'Bind failed. Error Code: ' + str(msg[0]) + 'Message' + msg[1]
	sys.exit()

skt.listen(10)

while(True):
	conn, addr = skt.accept()
	time.sleep(5)
	conn.close()
	




