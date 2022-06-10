#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_LOGIN_LEN 20
#define MAX_PASSWORD_LEN 30
#define MAX_COMMAND_LEN 15

char ADMIN_LOGIN[6] = "admin";
char ADMIN_PSWD[6] = "admin";
char USER_LOGIN[5] = "user";
char USER_PSWD[5] = "user";
char cur_dir[100];

char* concat(char* s1, const char* s2) {
	char* res = new char[50];
	strcpy(res, s1);
	strcat(res, s2);
	return res;
}

int auth(char login[], char pswd[]) {
	if (strcmp(ADMIN_LOGIN, login) == 0 && strcmp(ADMIN_PSWD, pswd) == 0) {
		return 0;
	} else if (strcmp(USER_LOGIN, login) == 0 && strcmp(USER_PSWD, pswd) == 0) {
		return 1;
	}
	return -1;
}

void access_denied() {
	std::cout << "Access denied!" << std::endl;
}

void split(std::string s, std::vector<std::string> &v){
	std::string temp = "";
	for(int i=0;i<s.length();++i){
		if(s[i]==' '){
			v.push_back(temp);
			temp = "";
		}
		else {
			temp.push_back(s[i]);
		}
	}
	v.push_back(temp);
}

void pwd() {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
	char s[100];
	printf("%s\n", getcwd(s, 100));
	chmod(concat(getenv("HOME"), "/emulator"), 0);
}

void quit() {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
}

void cd(std::string dir) {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
	chdir(dir.c_str());
	chmod(concat(getenv("HOME"), "/emulator"), 0);
}

void create_dir(std::string dir) {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
	mkdir(dir.c_str(), S_IRWXU | S_IRWXU | S_IRWXO);
	chmod(concat(getenv("HOME"), "/emulator"), 0);
}

void remove_dir(std::string file) {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
	remove(file.c_str());
	chmod(concat(getenv("HOME"), "/emulator"), 0);
}

void cat(std::string file) {
	int i = fork();
	if (i == 0) {
		chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
		execl("/bin/cat", "/bin/cat", file.c_str(), NULL);
		exit(0);
	} else {
		int status;
		wait(&status);
		chmod(concat(getenv("HOME"), "/emulator"), 0);
	}
}

void vi(std::string file) {
	int i = fork();
	if (i == 0) {
		chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
		execl("/bin/vi", "/bin/vi", file.c_str(), NULL);
		exit(0);
	} else {
		int status;
		wait(&status);
		chmod(concat(getenv("HOME"), "/emulator"), 0);
	}
}

void inter_handler(int sig) {
	if (sig == 2) {
		std::cout << std::endl;
		quit();
		exit(0);
	}
}

void ls() {
	chmod(concat(getenv("HOME"), "/emulator"), S_IRWXU | S_IRWXG | S_IRWXO);
	DIR *dir;
	struct dirent *file;
	dir = opendir(".");
	while((file = readdir(dir)) != NULL) {
		printf("%s\n", file->d_name);
	}
	closedir(dir);
	chmod(concat(getenv("HOME"), "/emulator"), 0);
}

int main() {
	chmod(concat(getenv("HOME"), "/emulator"), 0);
	char login[MAX_LOGIN_LEN], pswd[MAX_PASSWORD_LEN], line[MAX_COMMAND_LEN], command[15];
	cd("emulator");
	
	(void)signal(SIGINT, inter_handler);
	
	std::cout << "Enter your login:\n";
	std::cin.getline(login, MAX_LOGIN_LEN);
	std::cout << "Enter your password:\n";
	std::cin.getline(pswd, MAX_PASSWORD_LEN);
	
	int status = auth(login, pswd);
	
	if(status == -1) {
		quit();
		return 0;
	}
	
	while(true) {
		std::vector<std::string> words = {};
		printf("%s> ", getcwd(cur_dir, 100));
		
		std::cin.getline(line, MAX_COMMAND_LEN);
			
		split(line, words);
		strcpy(command, words[0].c_str());
		
		if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
			quit();
			break;
		} else if (strcmp(command, "ls") == 0) {
			ls();
		} else if (strcmp(command, "pwd") == 0) {
			pwd();
		} else if (strcmp(command, "cd") == 0) {
			if (words.size() > 1)
				cd(words[1]);
		} else if (strcmp(command, "mkdir") == 0) {
			if (status != 0) {
				access_denied();
			}	else if (words.size() > 1) {
				create_dir(words[1]);
			}
		} else if (strcmp(command, "rm") == 0) {
			if (status != 0)
				access_denied();
			else if (words.size() > 1)
				remove_dir(words[1]);
		} else if (strcmp(command, "vi") == 0) {
			if (status != 0) {
				access_denied();
			} else if (words.size() > 1) {
				if (words.size() > 1) {
					vi(words[1]);
				}
			}
		} else if (strcmp(command, "cat") == 0) {
			if (words.size() > 1) {
				cat(words[1]);
			}
		}
	}
}


