all: echo_server2

echo_server2: server.o tcpsocket.o echo-server2.o 
	    gcc -o echo-server2 echo-server2.o server.o tcpsocket.o
clean:
	    rm *o 

