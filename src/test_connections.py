import socket
import random
from time import sleep

# how many total connections?
HOW_MANY = 60
NUM_CONCUR = 60

pairs = ["myles","root"]

my_sockets = []

total_good = 0
total_bad = 0

red = "\033[91m"
gre = "\033[92m"
yel = "\033[93m"
c = "\033[0m"

def test_socket(sock, username):
    global total_good, total_bad
    sent = sock.send(bytes(username + "\n", "utf-8"))

    for i in range(20):
        try:
            recvstr = 'start'
            # recv all the data
            while recvstr is not '':
                chunk = sock.recv(15)
                recvstr = chunk.decode('utf-8')
                # print(recvstr) # uncomment this to see the output.
            print(gre + "good" + c)
            total_good += 1
            return
        except socket.error as e:
            if e.args[0] == 11:
                sleep(0.05)
    print(red + "bad -> " + username + c)
    total_bad+=1


def make_new_conn():
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect("tpf_unix_sock.server")
    s.setblocking(0)
    return s


def setup(count):
    for i in range(count):
       my_sockets.append(make_new_conn())

# let's pick a random socket

setup(NUM_CONCUR) # start with NUM_CONCUR connections

for i in range(HOW_MANY):
    rando = random.randint(0, len(my_sockets)-1)
    test_socket(my_sockets[rando],pairs[random.randint(0,len(pairs) - 1)])
    my_sockets[rando].close()
    my_sockets.pop(rando)
    my_sockets.append(make_new_conn())

while len(my_sockets) > 0:
    rando = random.randint(0, len(my_sockets)-1)
    test_socket(my_sockets[rando],pairs[random.randint(0,len(pairs) - 1)])
    my_sockets[rando].close()
    my_sockets.pop(rando)

print("\n" + gre + "good:", total_good, c, red + "bad:", total_bad,c);
print("Success %: ", (total_good/(total_good + total_bad)) * 100);
