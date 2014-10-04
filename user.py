#!/usr/bin/python2
import os
import socket
from os import path
from sys import argv

# Strip program filename
argv = argv[1:]

BUFFER_SIZE = 4096
DEBUG = 1
MAX_FILE_SIZE = 4*1024*1024 # 4 MiB
NG = 10

CS_name = 'localhost'
CS_port = 58000 + NG

SS_name = None
SS_port = None

def recvall(s):
    f = s.makefile('rb', BUFFER_SIZE)
    data = f.read()
    f.close()
    return data

def debug(fmt, *args):
    global DEBUG
    if DEBUG > 0:
        print ("[DEBUG] " + fmt) % args

def parse_args():
    global CS_name, CS_port
    for i in xrange(len(argv) - 1):
        if argv[i] == '-n':
            CS_name = argv[i+1]
        if argv[i] == '-p':
            CS_port = int(argv[i+1], 10)

def show_help():
    print "Invalid command! Valid commands:"
    print "\texit"
    print "\tlist"
    print "\tretrieve file"
    print "\tupload file"
    print

def list_command():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    s.connect((CS_name, CS_port))
    s.send('LST\n')

    # UDP sends by datagram, limited to 64k
    resp = s.recv(64 * 1024)
    s.close()

    words = resp.split()
    if len(words) < 4 or words[0] != 'AWL':
        debug("%r", words)
        print "Invalid server response!"
        return

    global SS_name, SS_port
    SS_name = words[1]
    SS_port = int(words[2], 10)

    debug("storage server is %s:%d", SS_name, SS_port)

    numFiles = int(words[3], 10)
    words = words[4:]

    if numFiles != len(words):
        print "Warning: server reported %d files, sent %d file names" % (numFiles, len(words))

    for i, e in enumerate(words, 1):
        print "%4d. %s" % (i, e)
    print


def upload_command(fileName):
    fileSize = path.getsize(fileName)
    if fileSize >= MAX_FILE_SIZE:
        print "Will not upload files larger than %.2f MiB" % (MAX_FILE_SIZE/1024/1024.0)
        return
    
    fileData = None
    with open(fileName, mode='rb') as f:
        fileData = f.read()

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    s.connect((CS_name, CS_port))
    s.send('UPR %s\n' % fileName)

    resp = recvall(s).split()
    if len(resp) != 2 or resp[0] != 'AWR' or resp[1] == 'ERR':
        debug("%r", resp)
        print "Unexpected response from server"
        s.close()
        return

    if resp[1] == 'dup':
        print "File already exists in server"
        s.close()
        return

    s.send('UPC %d ' % fileSize + fileData + '\n')
    resp = s.recv(BUFFER_SIZE).split()
    if len(resp) != 2 or resp[0] != 'AWC' or resp[1] != 'ok':
        debug("%r", resp)
        print "Unexpected response from server"

    print "File upload completed successfuly"
    
    s.close()

def retrieve_command(fileName):
    if SS_name == None or SS_port == None:
        print "Please use the list command to receive a storage server"
        return

    if path.exists(fileName) and path.isfile(fileName):
        print "File already exists, not clobbering"
        return

    err = False
    with open(fileName, 'wb') as f:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        s.connect((SS_name, SS_port))

        s.send('REQ %s\n' % fileName)
        resp = recvall(s).split(" ", 3)
        if len(resp) != 4 or resp[0] != 'REP' or resp[1] != 'ok':
            debug("%r", resp[:3])
            print "Unexpected response from server"
            err = True
        else:
            numBytes = int(resp[2], 10)
            if numBytes >= len(resp[3]):
                print "Server reports wrong file size: declared %d, read %d" % (numBytes, len(resp[3]))
                err = True
            else:
                f.write(resp[3][:numBytes])

        s.close()

    if err:
        os.remove(fileName)

    

def main():
    parse_args()
    while True:
        try:
            line = raw_input('> ')
        except EOFError:
            exit()

        line = line.strip()

        if len(line) == 0:
            continue

        words = line.split()
        words_len = len(words)

        try:
            if words_len >= 1:
                if words[0] == "exit":
                    break

                if words[0] == "list":
                    list_command()
                    continue

            if words_len >= 2:
                if words[0] == "retrieve":
                    retrieve_command(words[1])
                    continue
                elif words[0] == "upload":
                    upload_command(words[1])
                    continue

            show_help()
        except socket.error as e:
            print e
        except os.error as e:
            print e
                

if __name__ == '__main__':
    main()


