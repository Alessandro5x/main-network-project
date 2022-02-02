all:
	gcc -Wall -c common.c
	gcc -Wall cliente.c common.o -o cliente
	gcc -Wall -lpthread servidor.c common.o -o servidor -lpthread

clean:
	rm common.o client server servidor
