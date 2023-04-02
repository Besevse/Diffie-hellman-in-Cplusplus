This is a program for P2P file sending with diffie-hellman key exchange and AES encryption.
I made it for a school project long ago but recently decided to clean it up a little.
The frontend is made in QT.

MADE WITH THE HELP OF: https://www.cryptopp.com/wiki/Diffie-Hellman#Using_Diffie-Hellman_to_generate_an_AES_key


How to use:

Receiver opens instance of the program and types in the ip address of sender and ports.
Receiver enters full file path name he wants to receive
Sender does the same, receiver's receiving port is sender's sending port and vice versa.
Sender also enters the full path of the file he wants to send.
Receiver clicks receive first, and then sender clicks send, the file should start getting sent.