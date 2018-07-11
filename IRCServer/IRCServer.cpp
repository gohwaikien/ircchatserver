
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"
#include <string.h>
#include <cstring>
#include <string>
using namespace std;
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <iterator>

//users and password
map<string, string> users_pwd;
//rooms and vector of users
map<string, vector<string> > rooms_users;
//rooms and vector of messages
map<string, vector<string> > rooms_msgs;
//vector of rooms;
vector<string> rooms;
//vector of users;
vector<string> users;
//rooms and numbers
map<string, vector<string> > rooms_num;


//static int counter = 0;
//int countt = 0;
int QueueLength = 5;

//test

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

//	printf("The commandLine has the following format:\n");
//	printf("COMMAND <user> <password> <arguments>. See below.\n");
//	printf("You need to separate the commandLine into those components\n");
//	printf("For now, command, user, and password are hardwired.\n");

	string str = commandLine;
	int sp1 = str.find(" ");
	string str1 = str.substr(0, sp1);	

	int sp2 = str.find(" ", sp1 + 1);
	string str2 = str.substr(sp1+1, sp2-sp1-1);
	
	int sp3 = str.find(" ", sp2+1);
	string str3 = str.substr(sp2+1, sp3-sp2-1);
	
	//string str4 = str.substr(sp3+1, commandLineLength-sp3-1);	
	string str4 = str.substr(sp3+1);
	
	const char * command = str1.c_str();
	const char * user = str2.c_str();
	const char * password = str3.c_str();
	const char * args = str4.c_str();

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf("password=%s\n", password);
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-ROOMS")) {
		getAllRooms(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	// const char * msg =  "OK\n";
	// write(fd, msg, strlen(msg));

	close(fd);	
}

void
IRCServer::initialize()
{
	// Open password file

	// Initialize users in room
	
	// Initalize message list

}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	string temp_user = string(user);
	string temp_pwd = string(password);
	map<string, string>::iterator iter;
	for (iter = users_pwd.begin(); iter!= users_pwd.end(); iter++){
		if(iter->first.compare(temp_user) == 0){
			if(iter->second.compare(temp_pwd) == 0){
				return true;
			}
		}
	}
	return false;
}

void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	// Here add a new user. For now always return OK.
	const char * msg;

	string temp_user = string(user);	

	map <string, string>::iterator iter;
	
	for (iter = users_pwd.begin(); iter != users_pwd.end(); iter++){
		if ((iter->first.compare(temp_user)) == 0){
			msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	users_pwd.insert(make_pair(user, password));
	users.push_back(user);
	msg = "OK\r\n";
	write (fd, msg, strlen(msg));
	return;
}

void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;
	
	string temp_room = string(args);
	vector<string> temp_user;
	
	if (!checkPassword(fd, user, password)){
		msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	map <string, vector<string> >::iterator iter;
	
	for (iter = rooms_users.begin(); iter!= rooms_users.end(); iter++){
		if((iter->first.compare(temp_room)) == 0){
			msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	rooms_users.insert(make_pair(args, temp_user));
	rooms_msgs.insert(make_pair(args, temp_user));
	rooms.push_back(args);
	msg = "OK\r\n";
	write (fd, msg, strlen(msg));
	return;	
	
//	close(fd);
}

void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;

	string temp_room = string(args);
	vector<string> temp_user;
	string temp_user_str = string(user);

	if(!checkPassword(fd, user, password)){
		msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	for (int i = 0; i < rooms_users[temp_room].size(); i++){
		if ((rooms_users[temp_room][i]).compare(temp_user_str) == 0){
			msg = "OK\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}


	vector <string>::iterator iter;
	//vector <string>::iterator iter2;
	int flag = 0;

        for(iter = rooms.begin(); iter!= rooms.end(); iter++){
                if((*iter).compare(temp_room)==0){
			/*for (iter2 = users.begin(); iter2 != users.end(); iter2++){
				if((*iter2).compare(temp_user_str)==0){
					string temp_msg = *iter2 + temp_user_str + "hi" + "OK1\r\n";
					msg = temp_msg.c_str();
					write(fd, msg, strlen(msg));
					return;
				}
			}
			*///failed because they're always going to be equal..
		//	for (int i = 0; i < 10; i++ ){
			/*	if((rooms_users[temp_room][0]).compare(temp_user_str)==0){
					string temp_msg = temp_user_str + "hi" + "OK1\r\n";
                                        msg = temp_msg.c_str();
                                        write(fd, msg, strlen(msg));
                                        return;
				}
*/
		//	}
			rooms_users[temp_room].push_back(user);
                        users.push_back(user);
			//rooms.push_back(args); I think this is unnecessary or even wrong
                        flag = 1;
			msg = "OK\r\n";
                        write(fd, msg, strlen(msg));
			return;
		}
	}
	
	/*if(flag){
		vector <string>::iterator iter2;
	
		for (iter2 = users.begin(); iter2 != users.end(); iter2++){
			if((*iter2).compare(temp_user_str)==0){
				return;
			}
		}

		for (int i = 0; i < rooms_users.size()+5; i++){
			if ((rooms_users[args][i]).compare(temp_user_str)==0){
				write(fd, msg, strlen(msg));
				return;
			}
		}

		rooms_users[args].push_back(temp_user_str);
		users.push_back(user);
		write(fd, msg, strlen(msg));
		return;
	}
	*/	
	
	msg = "ERROR (No room)\r\n";
	write(fd, msg, strlen(msg));
	return;
}

void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;

	if(!(checkPassword(fd, user, password))){
		msg = "ERROR (Wrong password)\r\n";
		write(fd,msg,strlen(msg));
		return;
	}
	
	//vector <string>::iterator;
	string temp_user = string(user);
	//rooms_users[args].erase(temp_user);
	for (int i = 0; i < rooms_users[args].size(); i++){
		if ((rooms_users[args][i]).compare(temp_user)==0){
			rooms_users[args].erase(rooms_users[args].begin()+i);
			msg = "OK\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	msg = "ERROR (No user in room)\r\n";
	write (fd, msg, strlen(msg));
	return;
}

void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;
	
	string temp_user = string(user);

	//int counter = 1;

	if(!(checkPassword(fd, user, password))){
		msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	string temp_args = string(args);

	int sp1 = temp_args.find(" ");
	string temp_room = temp_args.substr(0, sp1);
	
	//int sp2 = temp_args.find(" ", sp1+1);
	string msgs = temp_args.substr(sp1+1);
	
	int j = 0;
/*	
	//string count_str = to_string(count);
	stringstream ss;
	ss << counter;
	string count_str = ss.str();

*/
	for (int i = 0; i < rooms_users[temp_room].size(); i++){		
		if ((rooms_users[temp_room][i]).compare(temp_user)==0){
			//if message count less than 100
			string returned_msgs = temp_user + " " + msgs + "\r\n";
			//counter++;
			rooms_msgs[temp_room.c_str()].push_back(returned_msgs.c_str());
			msg = "OK\r\n";
			write (fd, msg, strlen(msg));
			return;
		}
	}

/*	string string1 = "java-programming";
	string string2 = "mary"; 
	
	geek(string1);
	int count1 = 0;
	geek >> count1;	
	//int count1 = stoi(string1);
	

	geek(string2);
	int count2= 0 ;
	geek >> count2;
	//int count2 = stoi(string2);
*/
	//string message_temp = temp_user + temp_room + "ERROR (User not in room)\r\n";
	//msg = message_temp.c_str();	
	msg = "ERROR (user not in room)\r\n";
	write (fd, msg, strlen(msg));
	return;
}

void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;

	//int msgID = 0;

/*
	//string count_str = to_string(count);
        stringstream ss;
        ss << counter;
        string count_str = ss.str();
*/ //must iterate inside the loop instead

	string temp_user = string(user);
	
	if(!(checkPassword(fd, user, password))){
		msg = "ERROR (Wrong password)\r\n";
		write (fd, msg, strlen(msg));
		return;
	}
	
	string temp_args = string(args);
	
	int sp1 = temp_args.find(" ");
	string msg_from = temp_args.substr(0, sp1);
	//int msg_num = stoi(msg_from); //doesn't work cuz need C++ 11 or something	
	
	int msg_num = atoi(msg_from.c_str());

	/*if (msg_num > rooms_msgs[temp_room].size()){
		msg = "NO-NEW-MESSAGES\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	*/
 // must be inserted down below in the for loop
	int msgID = msg_num;

	//int sp2 = temp_args.find(" ", sp1 + 1);
	string temp_room = temp_args.substr(sp1 + 1);
	
	for (int i = 0; i < rooms_users[temp_room].size(); i++){
		if((rooms_users[temp_room][i]).compare(temp_user)==0){
			
			if (msg_num + 1 > rooms_msgs[temp_room].size()){
				msg = "NO-NEW-MESSAGES\r\n";
       	       			write(fd, msg, strlen(msg));
       	         		return;
       			}


			//map <string, vector<string> >::iterator iter;
			//for (iter = next(rooms_msgs.begin()); iter!=next(rooms_msgs.end()); iter++){
			for (int j = msg_num; j < rooms_msgs[temp_room].size() - 1; j++){
			//int j = 0;
			//while ((rooms_msgs[temp_room][j]).compare('\0') != 0){
				//if (!(rooms_msgs[temp_room][j]).empty()){	
				msgID++;
				stringstream ss;
        			ss << msgID;
        			string count_str = ss.str();

				string retmsgs = count_str + " " + rooms_msgs[temp_room][++msg_num];
				msg = retmsgs.c_str();
				write(fd, msg, strlen(msg));
				//}
				//j++;
			}
			msg = "\r\n";
			write (fd, msg, strlen(msg));
			return;
		}
	}
	msg = "ERROR (User not in room)\r\n";
	write (fd, msg, strlen(msg));
	return;

	/*
	//const char * temp_room2 = temp_room.c_str();
	int i = 0;
	//char * retmsgs = (char * ) malloc (sizeof(char)*100);
	
//	string message_temp = temp_room + "HI\r\n";
	string retmsgs = rooms_msgs[temp_room][i];
	msg = retmsgs.c_str();
	//msg = msg + "HELLO";
	write(fd, msg, strlen(msg));
	return;
	*/	
}

void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;

	if(checkPassword(fd, user, password)){
		//string temp_room = string(args);
		sort(rooms_users[args].begin(), rooms_users[args].end());

		//map<string, vector<string> >::iterator iter;
		//vector<string> temp_user;
	
		//int i = 0;
		/*for(iter = rooms_users.begin(); iter!= rooms_users.end(); iter++){
			string messg = rooms_users[args][i] + "\r\n";
			msg = messg.c_str();
			write(fd, msg, strlen(msg));
			i++;
			return;
		}
		*/
		for (int i = 0; i < rooms_users[args].size(); i++){
			string messg = rooms_users[args][i];
			messg = messg + "\r\n";
			msg = messg.c_str();
			write (fd, msg, strlen(msg));
		}

		msg = "\r\n";
		write (fd, msg, strlen(msg));
		return;
	}
	msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));
	return;
}

void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	const char * msg;
	
	if(checkPassword(fd, user, password)){	
		sort(users.begin(), users.end());

		vector<string>::iterator iter;

		for (iter = users.begin(); iter!=users.end(); iter++){
			string temp = *iter + "\r\n";
			msg = temp.c_str();
			write(fd, msg, strlen(msg));
		}
		write(fd, "\r\n", 2);
		return;
	}
	msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));
	return;	
}

void
IRCServer::getAllRooms(int fd, const char * user, const char * password, const char * args)
{
	const char * msg;
	
	if(!(checkPassword(fd, user, password))){
                msg = "ERROR (Wrong password)\r\n";
                write (fd, msg, strlen(msg));
                return;
        }

	string temp_args = string(args);

        int sp1 = temp_args.find(" ");
        string temp_room = temp_args.substr(sp1 + 1);

	vector<string>::iterator iter;
	for (iter = rooms.begin(); iter!=rooms.end(); iter++){
		string temp = *iter + "\r\n";
		msg = temp.c_str();
		write(fd, msg, strlen(msg));
	}
	write(fd, "\r\n", 2);
	return;
}
