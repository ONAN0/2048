CC=gcc -o

program1=./socket_monitoring.sh
program2=./server
program3=./highscore
program4=./PID_monitor.sh
program5=./2048


compile:
	$(CC) 2048 2048_linux.c -lncurses -lm
	$(CC) server server.c
	$(CC) highscore highscore.c
	
	clear
start:
	gnome-terminal --tab --title="Server" -- bash -c "$(program2); exec bash"
	sleep 1
	gnome-terminal --tab --title="Highscore" -- bash -c "$(program3); exec bash"
	sleep 1
	gnome-terminal --tab --title="2048" -- bash -c "$(program5); exec bash"
	clear
		
monitor:	
	gnome-terminal --title="Socket monitoring" -- bash -c "$(program1); exec bash"
	sleep 1
	gnome-terminal --title="PID monitoring" -- bash -c "$(program4); exec bash"
	clear
dev:
	gnome-terminal --title="Socket monitoring" -- bash -c "$(program1); exec bash"
	sleep 1
	gnome-terminal --tab --title="Server" -- bash -c "$(program2); exec bash"
	sleep 1
	gnome-terminal --tab --title="Highscore" -- bash -c "$(program3); exec bash"
	sleep 1
	gnome-terminal --title="PID monitoring" -- bash -c "$(program4); exec bash"
	sleep 1
	gnome-terminal --tab --title="2048" -- bash -c "$(program5); exec bash"
	clear 
