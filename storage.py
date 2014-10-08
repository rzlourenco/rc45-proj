#!/usr/bin/python

from common import *
from os import path
from sys import argv
import os
import signal
import socket

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
                print "[SS] Sending file --", data, "-- to %s:%d" % addr
                fileData = f.read(fileSize)
                conn.sendall(''.join(['REP ok %d ' % fileSize, fileData, '\n']))

    except IOError as e:
        print "[SS] Error reading file --", data, "-- for %s:%d" % addr, "->", e
        conn.sendall('REP nok 1 \0\n')

def handle_UPS(conn, addr, data):
    if addr[0] != CS_ADDR[0]:
        print "[SS] %s:%d is not the central server, but sent an UPS request" % addr
        conn.sendall('ERR\n')
        return

    # filename filesize data
    data = data.split(" ", 2)
    if len(data) != 3:
        print "[SS] Invalid protocol from central server at %s:%d" % addr
        return

    fileName = data[0].strip()
    if path.exists(fileName):
        print "[SS] file --", fileName, "-- already exists, clobbering"
    else:
        print "[SS] file --", fileName, "-- does not exist, creating"

    try:
        fileSize = int(data[1], 10)
    except ValueError:
        print "[SS] protocol error: expected integer in size field in UPS from %s:%d" % addr
        conn.sendall('ERR\n')
        return

    readBytes = len(data[2])
    pieces = [data[2]]
    while readBytes < fileSize:
        newBytes = conn.recv(BUFFER_SIZE)
        if not newBytes:
            break

        readBytes += len(newBytes)
        pieces.append(newBytes)

    if readBytes == fileSize + 1 and pieces[-1][-1] == '\n':
        pieces[-1] = pieces[-1][:-1]
        readBytes -= 1
    else:
        print "[SS] central server at %s:%d does not conform to protocol (eol missing)" % addr
        conn.sendall('ERR\n')
        return

    print "[SS] file --", fileName, "-- from %s:%d has" % addr, readBytes, "bytes"

    fileData = ''.join(pieces)
    try:
        with open(fileName, 'wb') as f:
            f.write(fileData)
    except IOError as e:
        print "[SS] could not write --", fileName, "-- from %s:%d ->" % addr, e
        conn.sendall('AWS nok\n')
        return

    conn.sendall('AWS ok\n')

def handle_client(conn, addr):
    msg = conn.recv(BUFFER_SIZE).split(" ", 1)

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

