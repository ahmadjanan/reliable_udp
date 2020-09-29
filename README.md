# Reliable User Datagram Protocol (RUDP)

First run Receiver.c then Sender.c


Receiver.c (Server side) linux execution:

gcc Receiver.c

./a.out "receiving-filename" "port no"
  
For example,

gcc Receiver.c

./a.out cartoon.mp4 1280


Sender.c (client Side) linux execution:

gcc Sender.c

./a.out "sending-filename" "port no"

For example,

gcc Sender.c

./a.out cartoon.mp4 1280
