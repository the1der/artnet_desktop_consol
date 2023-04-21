import socket
import struct
import time
def gen_artpollreply():
    
    header = bytearray(b"Art-Net\0")
    opCode = bytearray([0x00, 0x21])
    ipAddress = bytearray([192, 168, 32, 152])
    port=  bytearray([0x36,0x19])
    vers = bytearray([0x00, 0x00])
    unused1= bytearray([0x00, 0x00, 0x00, 0x00, 0x00])
    status1 = bytearray([0x00])
    unused2= bytearray([0x00, 0x00])
    shortname= bytearray(18)
    shortname[0:13]= bytearray(b"python tester")
    longname= bytearray(64)
    longname[0:17]= bytearray(b"python tester long")
    unused3= bytearray(92)
    mac=bytearray([0x4A, 0xD7, 0x9F, 0x23, 0xE1, 0x6C])
    bindipAddress = bytearray([192, 168, 32, 152])
    unused4 =  bytearray([0x00])
    status2 =  bytearray([0b01100000])
    unused5= bytearray(36)

    return header + opCode + ipAddress + port + vers + unused1 + status1 + unused2 + shortname + longname + unused3 + mac + bindipAddress + unused4 + status2 + unused5
    # return data

def genArtIpProg():
    header = bytearray(b"Art-Net\0")
    opCode = bytearray([0x00, 0xf9])
    vers = bytearray([0x00, 0x00])
    unused1= bytearray(4)
    ipAddress = bytearray([192, 168, 32, 152])
    subnet = bytearray([255, 255, 255, 0x00])
    port=  bytearray([0x19,0x36])
    status= bytearray([64])
    gw = bytearray([192, 168, 0, 0])
    unused2= bytearray(2)
    return header + opCode + vers + unused1 + ipAddress + subnet + port + status + gw + unused2



def main():
    # create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    # bind socket to Art-Net port
    sock.bind(('0.0.0.0', 1234))

    while 1:

        # receive Art-Net packet
        data, address = sock.recvfrom(1024)
        # extract opcode
        opcode = (data[9] << 8) + data[8]
        # print opcode
        print('Received Art-Net packet with opcode:', hex(opcode))
        if opcode  == 0x2000:
            print("ArtPoll recieved")
            data= gen_artpollreply()
            sock.sendto(data,("0.0.0.0", 6454))

        if opcode == 0xf800:
            print("IpProg recieved")
            data2 = genArtIpProg()
            time.sleep(0.2)
            sock2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock2.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            sock2.sendto(data2,address )


main()