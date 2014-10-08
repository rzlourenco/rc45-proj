#!/usr/bin/python

from os import path
from sys import argv
import os
import signal
import socket

BUFFER_SIZE = 4096
HEADER_SIZE = 3

NG = 10
CS_ADDR = ('127.0.0.1', 58000 + NG)

def handle_REQ(conn, addr, data):
    data = data.strip()
    print "[SS] %s:%d" % addr, "requested file", data

    try:
        with open(data, 'rb') as f:
            fileSize = path.getsize(data)
            if fileSize >= MAX_FILE_SIZE:
                print "[SS] Large file", data, "requested by %s:%d" % addr
                conn.sendall('REP nok 1 \0\n')
            else:
                print "[SS] Sending file", data, "to %s:%d" % addr
                fileData = f.read(fileSize)
                conn.sendall(''.join(['REP ok %d ' % fileSize, fileData, '\n']))

    except IOError as e:
        print "[SS] Error reading file --", data, "-- for %s:%d" % addr, "->", e
        conn.sendall('REP nok 1 \0\n')

def handle_UPS(conn, addr, data):
    if addr != CS_ADDR:
        print "[SS] %s:%d is not the central server, but sent an UPS request" % addr
        conn.sendall('ERR\n')
        return

    conn.sendall('AWS nok\n')

def handle_client(conn, addr):
    msg = conn.recv(BUFFER_SIZE)
    print "[SS] Received", len(msg), "bytes from %s:%d" % addr
    msg = msg.split(" ", 1)

    if len(msg) < 2 or msg[0] not in ('REQ', 'UPS'):
        print "[SS] Invalid message from %s:%d" % addr
        conn.sendall('ERR\n')
    elif msg[0] == 'REQ':
        print "[SS] REQ from %s:%d" % addr
        handle_REQ(conn, addr, msg[1])
    elif msg[0] == 'UPS':
        print "[SS] UPS from %s:%d" % addr
        handle_UPS(conn, addr, msg[1])


def main(argv):
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)

    for i in xrange(len(argv)-1):
        if argv[i] == '-p':
            SS_PORT = int(argv[i+1], 10)

    try:
        tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        tcp_socket.bind(('', SS_PORT))
        tcp_socket.listen(5)

        while True:
            conn, addr = tcp_socket.accept()
            print "[SS] Connection from %s:%d" % addr

            pid = os.fork()
            if pid != 0:
                conn.close()
                continue

            try:
                handle_client(conn, addr)
            finally:
                conn.close()
                print "[SS] Closed connection %s:%d" % addr

            exit(0)

    except socket.error as e:
        print e
    except os.error as e:
        print e
    finally:
        tcp_socket.close()

if __name__ == '__main__':
    main(argv[1:])

