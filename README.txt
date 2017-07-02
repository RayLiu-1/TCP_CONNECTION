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
3.1. Client:
3.1.1. The macro definition 'BUFFERSIZE' limit the length of single command
3.1.2. The client read the user's inputs. 
		If the user input 'pingSites' ,'showHandleStatus'or 'showHandles', the client 
			send the command to server. And receive the results from server and print 
			the results.
		If the user input other commands, the client will print the results.

3.2. Server:
3.2.1  Multithread:
	   The server spawns a recerving_handler thread when a client is connected to the server. 
	     The recerving_handler thread recerves requests from the client and return results
		 to it.
	   The server Spawns 5 thread ping_handler that open tcp sockets to sites and record the time.
3.2.2  DataStructure:
	   The server use a graph to store the sites:
	   Node: Every site is encapsulated in a 'Node' Structure. The 'Node's members are:
	     site: a char array that store the site name.
	     next: a 'Node' pointer points to next 'Node' in 'requests' que.
		 nextInHandle: a' Node pointer points to next 'Node' in same handle.
		 handle, min, max, avery, curStatus: the handle, mininum ping time, maximum ping
		 time , avery ping time, status of the Node.
	   requests: a que that store all the request Node before processing.
	   Handles: a Node* array that store all the head of list that formed by the Nodes with
	            same handle linked together.
3.2.3  Request reply:
	   pingSites:When the server received a 'pingSites' request. The sites are encapsulated in 'Node's. 
	             Then the 'Node's are push into 'requests' que and 'Handles[handle]' list.
	   showHandles:When the server received a 'showHandles' request. The server check Handles arrery and 
	               get heads for each handle list and retrieve the Node one by one by their 'nextInHandle'
				   pointer. And return the handle and site name for each handle.
	   showHandlesStatus: The server's working process is similiar with showHandes. The different is that 
						  it not only returns the site's handle and name, but also returns other statuses
	   
