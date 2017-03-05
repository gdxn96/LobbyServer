import socket
import sys
import celery_wrapper
from models import Lobby, Client
from socket import SOL_SOCKET, SO_REUSEADDR
import json

class UdpSocket():
    def __init__(self, HOST, PORT):
        self.sock = None
        self.HOST = HOST
        self.PORT = PORT

    def __enter__(self):
        self.sock = setup_socket(self.HOST, self.PORT)
        return self.sock

    def __exit__(self, type, value, traceback):
        self.sock.close()

def setup_socket(HOST, PORT):
    # Datagram (udp) socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print 'Socket created'
    except socket.error, msg :
        print 'Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
     
     
    # Bind socket to local host and port
    try:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
    except socket.error , msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
         
    print 'Socket bind complete'
    return s

def send_to_host_by_lobby_id(lobby_id, msg, config, socket=None):
    if socket:
        lobby = Lobby.query.get(lobby_id)
        ip_address = lobby.host.ip_address
        socket.sendto(msg, (ip_address, config["CLIENT_PORT"]))
    else:
        with UdpSocket(config["UDP_HOST"], config["UDP_PORT"]) as s:
            send_to_host_by_lobby_id(lobby_id, msg, config, s)

def send_to_all_by_lobby_id(lobby_id, msg, config, socket=None):
    if socket:
        lobby = Lobby.query.get(lobby_id)
        for client in lobby.clients:
            ip_address = client.ip_address
            socket.sendto(msg, (ip_address, config["CLIENT_PORT"]))
    else:
        with UdpSocket(config["UDP_HOST"], config["UDP_PORT"]) as s:
            send_to_all_by_lobby_id(lobby_id, msg, config, s)


def listen_blocking(config):
    HOST = config["UDP_HOST"]
    PORT = config["UDP_PORT"]
    with UdpSocket(HOST, PORT) as s:
        #now keep talking with the client
        while True:
            # receive data from client (data, addr)
            d = s.recvfrom(4096)
            data = d[0]
            addr = d[1]
            
            if not data: 
                break

            ip = addr[0]
            print data
            data = json.loads(data)

            if data["message_type"] == "ping":
                data["message_type"] = "pong"
                s.sendto(json.dumps(data), addr)
                print "sent to " + addr[0]
            elif data["message_type"] == "send_to_all":
                client = get_or_create(db.session, Client, ip_address=ip)
                send_to_all_by_lobby_id(client.lobby_id, data.data)
            elif data["message_type"] == "send_to_host":
                client = get_or_create(db.session, Client, ip_address=ip)
                send_to_host_by_lobby_id(client.lobby_id, data.data)
            elif data["message_type"] == "disonnect":
                client = get_or_create(db.session, Client, ip_address=ip)
                # do something about that

            
            print 'Message[' + addr[0] + ':' + str(addr[1]) + '] - ' + str(data)

def listen_nonblocking(config):
    celery_wrapper.start_daemon_task(listen_blocking, config)