import argparse

import cake_utils as utils


class CakeEnd:
    def __init__(self, address: str, debug=False):
        self.address = utils.parse_udp_address(address)
        self.debug = debug

    def __peel_cake(self, cake: bytes) -> str:
        cake = utils.bytes_to_string(cake)
        self.return_address, message = utils.find_udp_address(
            cake)
        return message

    def handle_message(self):
        cake = utils.await_udp_message(self.address)
        message = self.__peel_cake(cake)
        utils.send_udp_message(self.return_address, b'ACK')
        if self.debug:
            print("Message acquired: ", message)

    def start(self):
        while True:
            self.handle_message()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='TCR (The Cake Router) end node',
        description='Waits for a message, then delivers an \'ACK\' back to the node it got the message from.',
        epilog='')
    parser.add_argument('address')
    parser.add_argument('-d', '--debug',
                        action='store_true')
    args = parser.parse_args()

    ce = CakeEnd(args.address, args.debug)
    ce.start()