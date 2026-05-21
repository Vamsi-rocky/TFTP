TFTP File Transfer Application in C

A simple TFTP (Trivial File Transfer Protocol) based client-server application developed in C using UDP socket programming on Linux. This project enables reliable file transfer between client and server systems while implementing core TFTP functionalities.

Features
Client-Server communication using UDP sockets
File Upload (put) and Download (get) support
Supports TFTP operations:
RRQ (Read Request)
WRQ (Write Request)
DATA
ACK
ERROR packets
Multiple transfer modes:
Default Mode
Octet Mode
Netascii Mode
IP Address validation
Packet sequencing and block number verification
Error handling and retransmission support
Modular code structure
Technologies Used
C Programming
Linux System Programming
UDP Socket Programming
Networking Protocols
File Handling
Project Structure
.
├── tftp_client.c
├── tftp_client.h
├── tftp_server.c
├── tftp.c
├── tftp.h
├── file1.txt
├── file2.txt
└── README.md
How It Works
Client connects to the server using UDP sockets.
Client sends:
RRQ for downloading files
WRQ for uploading files
Server responds with ACK packets.
File data is transferred in DATA packets.
Client and server verify block numbers for reliable transfer.
ERROR packets are sent when failures occur.
Compilation
Compile Server
gcc tftp_server.c tftp.c -o server
Compile Client
gcc tftp_client.c tftp.c -o client
Execution
Run Server
./server
Run Client
./client
Client Commands
Connect to Server
connect <server_ip>

Example:

connect 127.0.0.1
Upload File
put filename.txt
Download File
get filename.txt
Change Transfer Mode
mode default
mode octet
mode net-ascii
Learning Outcomes
Understanding of UDP socket programming
Hands-on experience with networking protocols
Linux system call usage
File handling in C
Client-server architecture implementation
Protocol packet design and error handling
Future Improvements
Timeout and retransmission mechanism
Multi-client support using threads
Secure file transfer
Dynamic port handling
Logging system
GUI support
Author

Developed by Vamsi
