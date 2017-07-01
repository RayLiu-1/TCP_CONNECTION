1. Compiling instructions:
Server compiling commands:
gcc server.c -o server -lpthread

Client compiling commands:
gcc client.c -o client

2. Using instructions:
Server running commands:
./server <TCP port>

Client running commands:
./client <server's IP address> <server's TCP port>

where <TCP port> is the 'server''s tcp port for the connection.For example 8888 8889;
Command examples:

server:./server 8888

client:./client 172.0.0.1 8888

3. Design:
3.1 Client:
3.1.1 The macro definition 'BUFFERSIZE' limit the length of single command
3.1.2 The client read the user's inputs. 
		If the user input 'pingSites' or 'showHandles', the client send the command to server.
		If the user input ''

