if [ ! -d "server-file" ]; then
    mkdir -p server-file
fi

if [ ! -d "client-file" ]; then
    mkdir -p client-file
fi

if [ ! -d "user-info" ]; then
    mkdir -p user-info
fi

echo mxb1324212023>user-info/mxb
echo 45582>user-info/mxb2
echo 123>user-info/123

cd MyFileServer

gcc server.c socket.c tools.c server_api.c -o ../server -Wall
gcc client.c socket.c tools.c client_api.c -o ../client -Wall

