echo "Run nc -l 5555 ..."
Pause
serial 1 nullmodem server:127.0.0.1 port:5555 transparent:1 rxdelay:0 txdelay:0
echo Connected > COM1
