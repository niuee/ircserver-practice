#include<string>
#include<map>
#include<fstream>
#include <vector>
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"
#define chatMsgMax 100
class IRCServer {
	// Add any variables you need

private:
	int open_server_socket(int port);
	std::map<std::string, std::string> passwordList;
	std::map<std::string, std::vector<std::string> > roomList; // contain all the rooms and users in it
	std::map<std::string, std::vector<std::map<std::string, std::string> > > chatMsgList; // contain all the rooms and their message
	std::fstream file;
	std::string msg;
	
public:
	void initialize();
	bool checkPasswordWithResponse(int fd, std::string user, std::string password);
	bool checkPassword(int fd, std::string user, std::string password);
	void processRequest( int socket );
	void addUser(int fd, std::string user, std::string password);
	void enterRoom(int fd, std::string user, std::string password, std::string args);
	void leaveRoom(int fd, std::string user, std::string password, std::string args);
	void sendMessage(int fd, std::string user, std::string password, std::string args);
	void getMessages(int fd, std::string user, std::string password, std::string args);
	void getUsersInRoom(int fd, std::string user, std::string password, std::string args);
	void getAllUsers(int fd, std::string user, std::string password);
	void createRoom(int fd, std::string user, std::string password, std::string args);
	void listRoom(int fd, std::string user, std::string password);
	void runServer(int port);

	// added function
	std::string getWord(std::string command);
	void deniedMsg(int fd);
	void okMsg(int fd);
	void writeResponse(int fd, std::string resp);
	//void sortVector(std::vector<std::string>& vUser);
};

#endif