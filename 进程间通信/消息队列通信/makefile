.PHONY:all
all:client server
client:client.c comm.c
        gcc -o $@ $^
server:server.c comm.c
        gcc -o $@ $^
.PHONY:clean
clean:
        rm -f client server
