# virscient_coding_challenge

File server in c for virscient coding challenge

Uses TCP and just "normal system libraries". So far just tested on Linux. With some modification it might be able to get working on MinGW with Windows but I haven't looked into that.

Multiple client connections are done with fork rather than with threads. The maximum number of client can be specified with command line options. When the maximum is reached any new connection is accepted then closed.

The client can either list the files that the server can send or alternatively it can get one of the files from the server.

When the client gets a file from the server it can either output it to STDIO or to a local directory.

There is no encryption.

File names with ".." are not allowed.

Makefiles are generated with qmake and don't use the Qt framework.

Only stack is used.

Error handling leaves a little to be desired.

Extra credit question about incremental file changes not addressed.

## Building

On Linux.

If you have qmake then it's just...

```
./build
```

Else

```
mkdir bin
gcc client/*.c common/*.c -o bin/client
gcc server/*.c common/*.c -lm -o bin/server
```

## Client

The client's command line options are...

```
Usage: ./client [-h host] [-p port] [-d directory] [-s] [-l] [-t] [-f filename].
-h host: server host address. (default is localhost)
-p port: server host port. (default is 12345)
-d directory: path where files are saved. (default is '.')
-s: output file to stdio
-l: list files on server
-t: write over existing files
-f filename: get file from server called filename
```

Listing files outputs to STDIO. A Wireshare capture shows 4 packets are sent during a listing of a small directory... 

```
PROTO: 1
VERSION: 1
CMD: LIST

PROTO: 1
VERSION: 1
MSG: Listing
OK: 0

OK

          6	Dec 08 08:42	test2
          0	Dec 08 08:40	test

```

Getting test2 from the remote server a Wireshare capture shows 4 packets are transferred...

```
PROTO: 1
VERSION: 1
FILENAME: test2
CMD: GET

PROTO: 1
VERSION: 1
MSG: Here is test2 for you. It is 6 bytes
OK: 0

OK

dsga
```

Trying to get test3 from the remote server which doesn't exists results in the following Wireshare capture...

```
PROTO: 1
VERSION: 1
FILENAME: test3
CMD: GET

PROTO: 1
VERSION: 1
MSG: test3 doesn't exist
OK: -1
```

For listing files the client program outputs...

```
jontio@linux:~/git/virscient_coding_challenge/build-client-Desktop-Release$ ./client -l
Listing
          6     Dec 08 08:42    test2
          0     Dec 08 08:40    test
```

For getting a file the client program outputs...

```
jontio@linux:~/git/virscient_coding_challenge/build-client-Desktop-Release$ ./client -f test2 -t
connected to 127.0.0.1:12345
Here is test2 for you. It is 6 bytes
transfered 6 bytes
```

## Server

The server's command line options are...

```
Usage: ./server [-n name] [-p port] [-d directory] [-m max_nclients].
-n name: server name. (default is ANY)
-p port: server port. (default is 12345)
-d directory: path where files are served from. (default is '.')
-m max_nclients: max number of clients allowed. (default is 10)
```

## Testing

Setup server on VM in UK with digital ocean.

Client in Paekakariki on desktop on Intel I5 on 53MB/s Down / 13MB/s Up, ping 286ms

Client in Wellington on Raspberry Pi 3 unknown network 11MB/s Down / 11MB/s Up, ping 285ms

Started clients at same time and transferred a file of 451,839,315B.

Paekakariki took 53.4s (8.5MB/s) while Wellington took 148.4s (3.0MB/s)















