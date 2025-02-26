from argparse import ArgumentParser
from enum import IntEnum
from functools import reduce
from socket import AF_INET, SO_REUSEADDR, SOCK_STREAM, SOL_SOCKET, gethostname, socket
from struct import pack, unpack
from traceback import print_exc
from typing import Optional
from uuid import getnode

from faker import Faker

fake = Faker()
MAX_CHUNK_SIZE = 40
GROUP_ID = 0x01
MONITOR_ID = 0x01


class ResponseType(IntEnum):
    ACK = 0x06
    NACK = 0x15
    NAV = 0x18


def read_mac_address():
    mac_address = getnode()
    formatted_mac = ":".join(
        [
            "{:02x}".format((mac_address >> elements) & 0xFF)
            for elements in range(0, 8 * 6, 8)
        ][::-1]
    )
    return formatted_mac


def checksum(data):
    return reduce(lambda x, y: x ^ y, data, 0)


def pack_reply(control: int, group: Optional[int], data: bytes):
    # build message parts: control, group and data
    parts = bytearray([control or MONITOR_ID])
    if group is not None:
        parts.append(group or GROUP_ID)

    parts.extend(data)
    junk = fake.month_name()
    junk += fake.year()
    parts.extend(junk.encode())

    # compute message size and checksum
    msg_size = len(parts) + 2
    reply = bytearray([msg_size])
    reply.extend(parts)
    reply.append(checksum(reply))

    return bytes(reply)


def parse_message(received: bytes):
    hex_data = " ".join(f"{int(c):02x}" for c in received)
    print(">>>", hex_data)

    msg_len, control, group = unpack("BBB", received[:3])
    assert msg_len == len(received), "Message size mismatch"
    assert received[-1] == checksum(received[:-1]), "Checksum verification failed"

    response_data = pack("B", received[3])
    return pack_reply(control, group, response_data)


def on_receive(data: bytes):
    try:
        reply = parse_message(data)
    except NotImplementedError as err:
        msg = pack("BB", 0, ResponseType.NAV)
        reply = pack_reply(0, GROUP_ID, msg)
        print(err)
    except Exception:
        msg = pack("BB", 0, ResponseType.NACK)
        reply = pack_reply(0, GROUP_ID, msg)
        print_exc()
    return reply


def listen(host="127.0.0.1", port=5000):
    mac_address = read_mac_address()
    hostname = gethostname()

    print("Running on", hostname, "/", mac_address)

    with socket(AF_INET, SOCK_STREAM) as server_socket:
        server_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        server_socket.bind((host, port))
        server_socket.listen()
        print(f"SICP server listening on {host}:{port} ...")

        try:
            while True:
                client_socket, client_address = server_socket.accept()
                with client_socket:
                    print(client_address, " has opened a connection.")
                    while True:
                        data = client_socket.recv(MAX_CHUNK_SIZE)
                        if not data:
                            print(client_address, "has closed the connection.")
                            break

                        reply = on_receive(data)
                        hex_data = " ".join(f"{int(c):02x}" for c in reply)
                        print("<<<", hex_data)
                        client_socket.sendall(reply)

                client_socket.close()

        except KeyboardInterrupt:
            print("Emergency stop requested!")

        finally:
            print("Closing connection gracefully.")
            server_socket.close()


if __name__ == "__main__":
    parser = ArgumentParser(description="The most basic SICP server you've ever seen.")
    parser.add_argument("--port", type=int, default=5000)
    parser.add_argument("--host", type=str, default="localhost")
    args = parser.parse_args()

    listen(args.host, args.port)
