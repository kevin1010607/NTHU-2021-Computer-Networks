# NTHU 2021 Computer Networks

## Lab1

### Description
- Write a program which can show all the hyperlinks in a given web page. 
- The main objective is to practice socket programming and try to use the HTTP protocol.

### Requirements
- Input: The URL of the desired webpage without “http://”.
  e.g. For http://can.cs.nthu.edu.tw/index.php, you only need to type can.cs.nthu.edu.tw/index.php.
- Output: Print all hyperlinks and the number of hyperlinks in the given webpage. Note that only \<a href=“xxx”> should be counted, whereas \<link href=“xxx”> shouldn’t.

### Example
- There are 10 hyperlinks in http://can.cs.nthu.edu.tw/.
![](https://i.imgur.com/HdDc1Xx.png)

## Lab2

### Description
- Write a server program and a client program. 
- The client can download a video file
from the server using stop-and-wait mechanism through a UDP socket.

### Requirements
- For server program
    - Create a UDP socket of port 9999 and then wait for a request from a client.
    - The request should be “download fileName”, and the server should make a response to the client.
    - If the desired file exists, the server sends the video file to the client using the stop-and-wait mechanism.
    - During file transmission, the client should send acknowledgements to the server and the server should keep receiving ACKs from the client.
        - If the ACK is not received in 100 milliseconds (#define TIMEOUT 100), the missing packets should be retransmitted.
        - Hint: use clock()*1000/CLOCKS_PER_SEC+TIMEOUT to record the expired time in milliseconds.
- For client program
    - Create an UDP socket and assign the server address.
    - The user can make a command “download fileName” to the server.
    - If the response shows the file exists, then the client can start to receive the file.
    - To simulate packet loss, the client ignores each received packet with probability 0.5.
    - After successfully receiving a packet, the client should reply an ACK and append the receiving data to the end of a file if the sequence number associated with the packet is valid.

### Example
- Server is on the left part, client is on the right part.
![](https://i.imgur.com/mVvanWr.png)
