import socket

UDP_IP = "10.0.0.97"
UDP_PORT = 5001

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
count = 0
while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    print("Message count: %d" % count)
    print("\nReceived message: %s" % data)
    count = count+1
    print("Message count: %d" % count)
