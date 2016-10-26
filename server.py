#!/usr/bin/python3

import socketserver


class LogUDPHandler(socketserver.BaseRequestHandler):

    def handle(self):
        data = self.request[0].strip().decode('utf-8')
        socket = self.request[1]
        print('{who} sent: {what}'.format(who=self.client_address[0], what=str(data)))


if __name__ == "__main__":
    HOST, PORT = "localhost", 9999
    try:
        server = socketserver.UDPServer((HOST, PORT), LogUDPHandler)
        server.serve_forever()
    except (IOError, SystemExit):
        raise
    except KeyboardInterrupt:
        print ("Crtl+C Pressed. Shutting down.")

