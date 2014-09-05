/*
manager
must:
parse manager.ini
get ftok by file "server"
	create semaphore set ftok 's'
	create shared memory ftok 'm' size = servnum*sizeof(char)
	set mem by 0
	set sem to 1
create socket
loop
	listen port
	on connect 
	check client auth (need to think how)
		get token (one int or longlong, for room data)
		fork()
			execv server -port -token, may be something else
			//write on server app
			//server after start get shmem and semaphore 
			//set shmem[port] to 1 and before exit to 0
			//when change use semaphore
	else
		drop connection

///////
messages and commands must be described in this file or another
*/




