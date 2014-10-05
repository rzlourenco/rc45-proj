#!/usr/bin/python2
import glob
import os
import random
import signal
import socket
from os import path
from random import randint, shuffle
from sys import argv

# Strip program filename
argv = argv[1:]

BUFFER_SIZE = 4096
DEBUG = 1
MAX_FILE_SIZE = 4*1024*1024 # 4 MiB
NG = 10

TCP_server = None
UDP_server = None

CS_port = 58000 + NG
SS_servers = []

def recvall(s):
    f = s.makefile('rb', BUFFER_SIZE)
    data = f.read(2*MAX_FILE_SIZE)
    f.close()
    return data

def debug(fmt, *args):
    global DEBUG
    if DEBUG > 0:
        print ("[DEBUG] " + fmt) % args

def parse_args():
    global CS_port
    for i in xrange(len(argv) - 1):
        if argv[i] == '-p':
            CS_port = int(argv[i+1], 10)

def read_storage_servers():
    global SS_servers

    with open('../serverlist', 'r') as f:
        for line in f:
            hostname, port = line.split(':')
            SS_servers.append((hostname, int(port, 10)))

def run_tcp_server():
    global TCP_server
    while True:
        conn, addr = TCP_server.accept()

        pid = os.fork()
        if pid != 0:
            conn.close()
            # # We need to handle this exception since the process may have
            # # already ended by the time waitpid is called.
            # try:
            #     os.waitpid(pid, 0)
            # except os.error as e:
            #     if e.errno != os.errno.ECHILD:
            #         raise e
            continue

        debug("forked %s", addr)
        data = conn.recv(BUFFER_SIZE).split()
        if len(data) != 2 or data[0] != 'UPR':
            conn.sendall('ERR\n')
            conn.close()
            exit(1)

        debug("UPR %s", addr)
        fileName = data[1]
        if path.exists(fileName):
            conn.sendall('AWR dup\n')
            conn.close()
            exit(0)

        err = False

        debug("file does not exist %s", addr)
        conn.sendall('AWR ok\n')
        data = recvall(conn).split(" ", 2)
        if len(data) != 3 or data[0] != 'UPC':
            conn.sendall('ERR\n')
            conn.close()
            exit(1)

        debug("received all %s", addr)
        try:
            fileSize = int(data[1], 10)
        except ValueError:
            conn.sendall('ERR\n')
            cnn.close()
            exit(1)

        if not (0 <= fileSize <= MAX_FILE_SIZE and fileSize == len(data[2])-1):
            conn.sendall('AWC nok\n')
            conn.close()
            exit(1)

        err = False
        for SS_name, SS_port in SS_servers:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_TCP)
            try:
                s.connect((SS_name, SS_port))
                s.sendall(''.join(['UPS', ' ',
                                   fileName, ' ',
                                   data[2][:fileSize], '\n']))
                resp = s.recv(BUFFER_SIZE).split()
                if len(resp) != 2 or resp[0] != 'AWS' or resp[1] not in ('nok', 'ok'):
                    err = True
                    print "Storage server %s:%d does not conform to protocol." % (SS_name, SS_port)
                    break
                elif resp[1] == 'nok':
                    err = True
                    break

            except socket.error as e:
                print "Storage server at %s:%d -> %s" % (SS_name, SS_port, e)
            finally:
                s.close()

        if err:
            conn.sendall('AWC nok\n')
            conn.close()
            exit(1)

        # We don't need to write anything, just create the file
        with open(fileName, 'ab') as f:
            pass

        conn.sendall('AWC ok\n')
        conn.close()
        exit(0)       

def run_udp_server():
    global UDP_server
    while True:
        data, addr = UDP_server.recvfrom(BUFFER_SIZE)

        pid = os.fork()
        if pid != 0:
            continue

        random.seed()

        if data == 'LST\n':
            # We take at most 30 files
            fileList = []
            for f in glob.iglob('*'):
                if len(fileList) >= 30:
                    break
                if path.isfile(f) and len(f) <= 20:
                    fileList.append(f)

            shuffle(fileList)

            server, port = SS_servers[randint(0, len(SS_servers)-1)]
            resp = ''.join(["AWL", " ",
                            server, " ",
                            str(port), " ",
                            str(len(fileList)), " ",
                            " ".join(fileList), "\n"])

            UDP_server.sendto(resp, addr)
        else:
            UDP_server.sendto('ERR\n', addr)

        exit(0)

def main():
    global TCP_server, UDP_server, CS_port

    signal.signal(signal.SIGCHLD, signal.SIG_IGN)

    try:
        read_storage_servers()

        UDP_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        UDP_server.bind(('', CS_port))

        TCP_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        TCP_server.bind(('', CS_port))
        TCP_server.listen(5)

        child_pid = os.fork()
        if child_pid == 0:
            run_tcp_server()

        run_udp_server()

        TCP_server.close()
        UDP_server.close()

    except socket.error as e:
        print e
    except os.error as e:
        print e


if __name__ == '__main__':
    main()

