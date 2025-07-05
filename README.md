Как запустить:
На одной машине в режиме сервера:
bash
Copy code
./program server 12345
На другой машине или в другом терминале в режиме клиента:
bash
Copy code
./program client 54321 <ip_сервера>:12345
или, если оба на одной машине:

bash
Copy code
./program server 12345
./program client 127.0.0.1:12345
