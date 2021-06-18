
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
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>


#include "IRCServer.h"

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
	std::string commandPassed;
	std::string command;
	std::string user;
	std::string password;
	std::string args;
	
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

	commandPassed = commandLine;
	

	std::cout<<"RECEIVED: "<<commandPassed<<std::endl;

	/*printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");*/
	


	int subi = 0;
	std::string tempcmd; 
	int cmdorder = 0;

	while(1){
		tempcmd = getWord(commandPassed);
		//std::cout<<tempcmd<<std::endl;
		
		subi = 0;
		subi += (tempcmd.length() + 1);
		if(cmdorder == 0){
			command = tempcmd;
		}
		else if(cmdorder == 1){
			user = tempcmd;
		}
		else if(cmdorder == 2){
			password = tempcmd;
		}
		else if(cmdorder == 3){
			args = tempcmd;
		}
		else{
			args = args+" "+tempcmd;
		}
		if(subi > commandPassed.length() - 1){
			//std::cout<<"Out of bound"<<std::endl;
			break;
		}
		commandPassed = commandPassed.substr(subi);
		cmdorder++;
	}

	if (!command.compare("ADD-USER")) {
		addUser(fd, user, password);
	}
	else if (!command.compare("ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!command.compare( "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!command.compare("SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!command.compare("GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!command.compare("GET-USERS-IN-ROOM")) {

		getUsersInRoom(fd, user, password, args);
	}
	else if (!command.compare("GET-ALL-USERS")) {
		getAllUsers(fd, user, password);
	}
	else if (!command.compare("LIST-ROOMS")){
		listRoom(fd, user, password);
	}
	else if(!command.compare("CREATE-ROOM")){
		createRoom(fd, user, password, args);
	}
	else if(!command.compare("CHECK-PASSWORD")){
	    checkPasswordWithResponse(fd, user, password);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	//std::cout<<"This is the username "<<user<<std::endl;
	

	close(fd);	
}

void
IRCServer::initialize()
{
	// Open password file
	file.open(PASSWORD_FILE, std::ios::in);
	int mapSize;
	std::string key;
	std::string value;
	passwordList.clear();
	if(file.fail()){
		file.open(PASSWORD_FILE, std::fstream::out);
	}
	else{
		while(std::getline(file, key)){
			std::getline(file, value);
			passwordList[key] = value;
			std::cout<<"User: "<<key<<std::endl;
			std::cout<<"Password: "<<value<<std::endl;
		}
	}

	

	// Initialize users in room
	roomList.clear();
	// Initalize message list
	chatMsgList.clear();
	
	file.close();
}

bool 
IRCServer::checkPasswordWithResponse(int fd, std::string user, std::string password){
	// Here check the password
	std::cout<<"Check User: "<<user<<std::endl;
	std::cout<<"Password Check: "<<password<<std::endl;
	//user = user + "\r";
	//password = password + "\r";
	for(std::map<std::string, std::string>::iterator itr = passwordList.begin(); itr != passwordList.end(); itr++){
		std::cout<<"User: "<<itr->first<<std::endl;
		std::cout<<"Password: "<<itr->second<<std::endl;
	}
	std::cout<<"Size: "<<passwordList.size()<<std::endl;
	if(passwordList.find(user) == passwordList.end()){
		std::cout<<"No User Found"<<std::endl;
	  	deniedMsg(fd);
		return false;
	}
	std::cout<<passwordList.find(user)->first<<std::endl;
	std::cout<<passwordList.find(user)->second<<std::endl;
	if(passwordList.find(user)->second.compare(password) != 0){
		std::cout<<"Wrong Password!"<<std::endl;
	  	deniedMsg(fd);
		return false;
	}
	okMsg(fd);
	return true;
}

bool
IRCServer::checkPassword(int fd, std::string user, std::string password) {
	// Here check the password
	std::cout<<"Check User: "<<user<<std::endl;
	std::cout<<"Password Check: "<<password<<std::endl;
	//user = user + "\r";
	//password = password + "\r";
	for(std::map<std::string, std::string>::iterator itr = passwordList.begin(); itr != passwordList.end(); itr++){
		std::cout<<"User: "<<itr->first<<std::endl;
		std::cout<<"Password: "<<itr->second<<std::endl;
	}
	std::cout<<"Size: "<<passwordList.size()<<std::endl;
	if(passwordList.find(user) == passwordList.end()){
		std::cout<<"No User Found"<<std::endl;
	  	//deniedMsg(fd);
		return false;
	}
	std::cout<<passwordList.find(user)->first<<std::endl;
	std::cout<<passwordList.find(user)->second<<std::endl;
	if(passwordList.find(user)->second.compare(password) != 0){
		std::cout<<"Wrong Password!"<<std::endl;
	  	//deniedMsg(fd);
		return false;
	}
	//okMsg(fd);
	return true;
}

void
IRCServer::addUser(int fd, std::string user, std::string password)
{
	// Here add a new user. For now always return OK.
	std::string msg;
	if(passwordList.find(user) != passwordList.end()){
		deniedMsg(fd);
		return;
	}
	passwordList[user] = password;
	file.open(PASSWORD_FILE, std::ios::app);

	file << user << "\n";
	file << password << "\n"; 
	okMsg(fd);
	file.close();


	
	return;		
}

void
IRCServer::enterRoom(int fd, std::string user, std::string password, std::string args)
{	
	std::vector<std::string>::iterator itr;
	std::vector<std::string> v;
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	if(roomList.find(args) == roomList.end()){
		writeResponse(fd, "ERROR (No room)\r\n");
		return;
	}
	v = roomList[args];
	if((itr = std::find(v.begin(), v.end(), user)) != v.end()){
		//writeResponse(fd, "Already in it\r\n");
		okMsg(fd);
		return;
	}
	else {
		okMsg(fd);
		roomList[args].insert(roomList[args].begin(), user);
		return;
		
	}
	

	return;
}

void
IRCServer::leaveRoom(int fd, std::string user, std::string password, std::string args)
{
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	else if(roomList.find(args) == roomList.end()){
		deniedMsg(fd);
		return;
	}
	else{
		std::vector<std::string> v;
		v = roomList[args];
		std::vector<std::string>::iterator itr;
		if((itr = std::find(v.begin(), v.end(), user)) != v.end()){
			v.erase(itr);
			roomList[args] = v;
			okMsg(fd);
			return;
		}
		else{
			writeResponse(fd, "ERROR (No user in room)\r\n");
			return;
		}

		
	}
}

void
IRCServer::sendMessage(int fd, std::string user, std::string password, std::string args)
{
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	std::string room;
	std::string localChatMsg;
	int subi = 0;
	std::string tempcmd; 
	int cmdorder = 0;
	while(1){
		tempcmd = getWord(args);
		//std::cout<<tempcmd<<std::endl;
		
		subi = 0;
		subi += (tempcmd.length() + 1);
		if(cmdorder == 0){
			room = tempcmd;
		}
		else if (cmdorder == 1){
			localChatMsg += tempcmd;
		}
		else{
			localChatMsg += " " + tempcmd;
		}
		if(subi > args.length() - 1){
			//std::cout<<"Out of bound"<<std::endl;
			break;
		}
		args = args.substr(subi);
		cmdorder++;
	}
	if(roomList.find(room) == roomList.end()){
		deniedMsg(fd);
		return;
	}
	std::vector<std::string> vUser;
	vUser = roomList[room];
	if(std::find(vUser.begin(), vUser.end(), user) != vUser.end()){
		std::map<std::string, std::string> msgMap;
		msgMap[user] = localChatMsg;
		chatMsgList[room].push_back(msgMap);
		okMsg(fd);
		if(chatMsgList[room].size() == 101){
			chatMsgList[room].erase(chatMsgList[room].begin());
		}
		return;
	}
	writeResponse(fd, "ERROR (user not in room)\r\n");
	

	return;
	
}

void
IRCServer::getMessages(int fd, std::string user, std::string password, std::string args)
{
	//std::cout<<"Never pass the password"<<std::endl;
	if(!checkPassword(fd,user, password)){
		//writeResponse(fd, "Password Check i doubt");
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	std::string room;
	std::string MsgNum;
	int subi = 0;
	std::string tempcmd; 
	int cmdorder = 0;
	while(1){
		tempcmd = getWord(args);
		//std::cout<<tempcmd<<std::endl;
		
		subi = 0;
		subi += (tempcmd.length() + 1);
		if(cmdorder == 1){
			room = tempcmd;
		}
		else if(cmdorder == 0){
			MsgNum = tempcmd;
		}
		else{
			//std::cout<<"Too many arguments"<<std::endl;
			break;
		}
		if(subi > args.length() - 1){
			//std::cout<<"Out of bound"<<std::endl;
			break;
		}
		args = args.substr(subi);
		cmdorder++;
	}

	//writeResponse(fd, room+"\r\n");
	//writeResponse(fd, MsgNum+"\r\n");
	std::vector<std::string> vUser;
	vUser = roomList[room];

	if(roomList.find(room) == roomList.end()){
		deniedMsg(fd);
		return;
	}
	if(std::find(vUser.begin(), vUser.end(), user) != vUser.end() && atoi(MsgNum.c_str()) < chatMsgList[room].size()){
		std::vector< std::map<std::string, std::string> > Msgs = chatMsgList[room];
		std::ostringstream convert;
		std::string index;
		for(int i = atoi(MsgNum.c_str()) + 1; i < chatMsgList[room].size();i++){
			convert<<i;
			index = convert.str();
			convert.str("");
			writeResponse(fd, index+" "+Msgs[i].begin()->first+": "+Msgs[i].begin()->second + "\r\n");
		}
		writeResponse(fd, "\r\n");
		return;
	}
	else if (std::find(vUser.begin(), vUser.end(), user) == vUser.end() ){
		writeResponse(fd, "ERROR (User not in room)\r\n");
		return;
	}
	else if (atoi(MsgNum.c_str()) >= chatMsgList[room].size()){
		writeResponse(fd, "NO-NEW-MESSAGES\r\n");
		return;
	}
	return;
	
}

void
IRCServer::getUsersInRoom(int fd, std::string user, std::string password, std::string args)
{
	//writeResponse(fd, "Before Function\r\n");
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	//writeResponse(fd, "After password check\r\n");
	if (roomList.find(args) == roomList.end()){
		deniedMsg(fd);
		writeResponse(fd, "No room\r\n");
		return;
	}
	else{
		//writeResponse(fd, "Sort begin\r\n");
		std::sort(roomList[args].begin(), roomList[args].end());
		//writeResponse(fd, "Sort came\r\n");
		for(std::vector<std::string>::iterator itr = roomList[args].begin(); itr != roomList[args].end(); itr++){
			writeResponse(fd, *itr + "\r\n");
		}
		writeResponse(fd, "\r\n");
		return;
	}
}

void
IRCServer::getAllUsers(int fd, std::string user, std::string password)
{	
	
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	std::map<std::string, std::string>::iterator ite;
	for(ite = passwordList.begin(); ite != passwordList.end(); ite++){
		writeResponse(fd, ite->first+"\r\n");
	}

	writeResponse(fd, "\r\n");

}

void
IRCServer::createRoom(int fd, std::string user, std::string password, std::string args){
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
		return;
	}
	else if(roomList.find(args) != roomList.end()){
		deniedMsg(fd);
		return;
	}
	else{
		std::vector<std::string> v;
		std::vector<std::map<std::string, std::string > > v2;
		v2.clear();
		v.clear();
		roomList[args] = v;
		chatMsgList[args]  = v2;
		okMsg(fd);
		std::cout<<"Success!"<<std::endl;
	}
}

void
IRCServer::listRoom(int fd, std::string user, std::string password){
	if(!checkPassword(fd, user, password)){
		writeResponse(fd, "ERROR (Wrong password)\r\n");
	}
	else{
		std::map<std::string, std::vector<std::string> >::iterator itr;
		for(itr = roomList.begin(); itr != roomList.end();itr++){
			writeResponse(fd, itr->first + "\r\n");
		}
	}
	writeResponse(fd, "\r\n");
}

//added functionality
std::string 
IRCServer::getWord(std::string command){
	int i = 0;
	std::string separated = "";
	int cmdLen = command.length();
	while(command[i] != 32 && command[i] != 10  && command[i] != '\r'){
		separated.push_back(command[i]);
		i++;
		if(i == cmdLen){
			break;
		}
	}

	return separated;
}

void
IRCServer::deniedMsg(int fd){
	msg = "DENIED\r\n";
	write(fd, msg.c_str(),msg.length());
}

void 
IRCServer::okMsg(int fd){
	msg = "OK\r\n";
	write(fd, msg.c_str(),msg.length());
}

void
IRCServer::writeResponse(int fd, std::string resp){
	write(fd, resp.c_str(), resp.length());
}

/*void 
IRCServer::sortVector(std::vector<std::string>& vUser){
	
	for(int i = 0; i < vUser.size() - 1;i++){
		for(int j = 0; j < vUser.size() - i - 1; j++){
			if(vUser[i].compare(vUser[i+1]) > 0){
				std::string temp;
				temp = vUser[i];
				vUser[i] = vUser[i+1];
				vUser[i+1] = temp;
			}
		}
	}
}*/
