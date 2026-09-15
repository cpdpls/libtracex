import socket 
import threading 
import random
from pathlib import Path
import time

bind_ip = "0.0.0.0" 
bind_port = 5555

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((bind_ip, bind_port)) 
# we tell the server to start listening with 
# a maximum backlog of connections set to 5
server.listen(1) 

print(f"[+] Listening on port {bind_ip} : {bind_port}")                            

#client handling thread
def handle_client(client_socket):
    bytes_left = Path("r15b_all_int.trx").stat().st_size
    with open("r15b_all_int.trx", mode="rb") as f:
        while 1:
            bytes_count = random.randint(10, 100)
            bytes_s = f.read(bytes_count)
            if not bytes_s:
                break
            client_socket.send(bytes_s)
            bytes_left = bytes_left - bytes_count
            print ("Sended {} | left {}".format(bytes_count, bytes_left))
            time.sleep(0.1)
while True: 
    # When a client connects we receive the 
    # client socket into the client variable, and 
    # the remote connection details into the addr variable
    client, addr = server.accept() 
    print(f"[+] Accepted connection from: {addr[0]}:{addr[1]}")
    #spin up our client thread to handle the incoming data 
    handle_client(client)