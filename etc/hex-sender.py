import socket

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(("localhost", 5000))

keep_going = True
while keep_going:
    data = bytes.fromhex(input("Hex: "))
    if data:
        clientsocket.send(data)
        reply = clientsocket.recv(40)
        print("Reply:", reply.hex())
    else:
        keep_going = False

clientsocket.close()
