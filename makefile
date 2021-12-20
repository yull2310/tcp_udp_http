all:
	gcc server.c commfunc.c link_list.c -o myServer -g -lpthread
	gcc client.c commfunc.c -o client -g -lpthread
	gcc -g link_list.c test.c commfunc.c -o test

clean:
	rm -rf myServer client test hex_data
