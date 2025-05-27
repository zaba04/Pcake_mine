class UDPAddress:
    def __init__(self, ip, port):
        self.IP = ip
        self.PORT = port

    def __repr__(self):
        return f"{self.IP}:{self.PORT}"