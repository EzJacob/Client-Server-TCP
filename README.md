# Client-Server-TCP
Measuring the time it takes for the server to receive a file of at least 2MB.

**How to run:**
If you use the make file then make sure to adjust the arguments in the make file for your needs.
You can use the the make file to run TCP_receiver.c and TCP_Sender.c in the following way:
make run_receiver
make run_sender

or you can run manually like so:
The receiver: ./TCP_Receiver -p [PORT] -algo [reno/cubic]
The sender: ./TCP_Sender -ip [IP] -p [PORT] -algo [reno/cubic]

By default the server works on local host.
