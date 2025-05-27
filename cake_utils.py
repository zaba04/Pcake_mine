import socket
import re
from typing import Tuple

from udp_address import UDPAddress


def string_to_bytes(s: str) -> bytes:
    return bytes(s, encoding="ascii")


def bytes_to_string(b: bytes) -> str:
    return b.decode(encoding="ascii")


def parse_udp_address(udp_address) -> UDPAddress:
    pattern = r'^((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})|(localhost)):(\d{1,5})$'
    match = re.match(pattern, udp_address)

    if match:
        if match.group(2):
            ip_address = match.group(2)
        else:
            ip_address = '127.0.0.1'
        port_number = int(match.group(4))
        return UDPAddress(ip_address, port_number)
    else:
        return None


def find_udp_address(cake: str) -> Tuple[UDPAddress, str]:
    pattern = r"\b(?:localhost|(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})):(\d{1,5});\b"
    match = re.search(pattern, cake)
    if match:
        address = match.group(1) or "localhost"
        port = int(match.group(2))
        rest = cake[match.end():]
        return (UDPAddress(address, port), rest)
    else:
        return None


def send_udp_message(address: UDPAddress, message_body: bytes):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.sendto(message_body, (address.IP, address.PORT))
    s.close()


def await_udp_message(address: UDPAddress) -> bytes:
    s = socket.socket(socket.AF_INET,
                      socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((address.IP, address.PORT))
    data = s.recvfrom(1024)[0]
    s.close()
    return data