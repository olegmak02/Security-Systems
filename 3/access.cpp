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
#include <fstream>
#include <math.h>
#include <cmath>

#define CHECK_INTERVAL 240
#define MAX_LOGIN_LEN 20
#define MAX_PASSWORD_LEN 30
#define MAX_COMMAND_LEN 40
#define REPORT_INTERVAL 259200

char ADMIN_LOGIN[6] = "admin";
char ADMIN_PSWD[6] = "admin";
char USER_LOGIN[5] = "user";
char USER_PSWD[5] = "user";
char cur_dir[100];
char prog_dir[200];
char em_path[200];

using namespace std;

char* concat(char* s1, const char* s2) {
	char* res = new char[500];
	strcpy(res, s1);
	strcat(res, s2);
	return res;
}

char* enc(const char* t) {
	char key[10] = {'`','!','*','-','@','+','`','$', ',', '.'};
  char* buf = new char[strlen(t)];
  for(int i = 0; i < strlen(t); i++) {
  	buf[i] = t[i] ^ key[i % 10];
  }
  buf[strlen(t)] = '\0';
  return buf;
}

void block() {
	chmod(concat(em_path, "/syslog/pswd"), 0);
	chmod(concat(em_path, "/syslog/questions"), 0);
	chmod(concat(em_path, "/syslog/auth"), 0);
	chmod(concat(em_path, "/syslog/seclog"), 0);
	chmod(concat(em_path, "/syslog/rights"), 0);
	chmod(concat(em_path, "/request.txt"), 0);
	chmod(concat(em_path, "/syslog"), 0);
	chmod(em_path, 0);
}

void unblock() {
	chmod(em_path, S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/pswd"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/auth"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/seclog"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/questions"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/rights"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/request.txt"), S_IRWXU | S_IRWXG | S_IRWXO);
	
}

char* readFile(char* filename) {
	char* res = new char[1000];
	strcpy(res, "");
	ifstream file(filename);
  string line;
  while(getline(file, line)) {
    strcat(res, line.c_str());
    strcat(res, "\n");
  }
	return res;
}

void writeFile(const char* text, char* filename) {
	FILE *fp = NULL;
  if ((fp = fopen(filename, "w")) == NULL) {
   	perror("error"); 
   	exit(1);
  }
  fprintf(fp, text);
  fclose(fp);
}

void appendFile(const char* text, char* filename) {
	FILE *fp = NULL;
  if ((fp = fopen(filename, "a")) == NULL) {
   	perror("error"); 
   	exit(1);
  }
  fprintf(fp, text);
  fclose(fp);
}
vector<string> getLines(string text) {
	vector<string> v;
	string temp = "";
	for(int i=0;i<text.length();++i){
		if(text[i]=='\n'){
			v.push_back(temp);
			temp = "";
		}
		else {
			temp.push_back(text[i]);
		}
	}
	return v;
}

int auth(char login[], char pswd[]) {
	if (strcmp(ADMIN_LOGIN, login) == 0 && strcmp(ADMIN_PSWD, pswd) == 0) {
		return 0;
	} else if (strcmp(USER_LOGIN, login) == 0 && strcmp(USER_PSWD, pswd) == 0) {
		return 1;
	}
	return -1;
}

void access_denied(int status) {
	unblock();
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/seclog")));
	writeFile(lines[0].c_str(), concat(em_path, "/syslog/seclog"));
	char buf[500];
	strcpy(buf, lines[status].substr(0, lines[status].size()-3).c_str());
	strcat(buf, ":|:");
	strcat(buf, to_string(time(NULL)).c_str());
	strcat(buf, ":>:");
	lines[status] = buf;
	for (int i = 1; i < lines.size(); i++) {
		appendFile("\n", concat(em_path, "/syslog/seclog"));
		appendFile(lines[i].c_str(), concat(em_path, "/syslog/seclog"));
	}
	block();
	cout << "Access denied!" << endl;
}

int getRights(int status, const char* file) {
	if (status == 0) {
		return 1;
	}
	string rights = getLines(readFile(concat(em_path, "/syslog/rights")))[status];
	char buf[150];
	strcpy(buf, file);
	strcat(buf, "(");
	if(rights.find(buf) == -1) {
		return -1;
	}
	int ind = rights.find("(", rights.find(buf));
	if (rights.find(buf) == -1) {
		return -1;
	}
	return ((int)rights[ind+1]) - ((int)'0');
}

void addRights(int status, const char* file, int right) {
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/rights")));
	writeFile(lines[0].c_str(), concat(em_path, "/syslog/rights"));
	appendFile("\n", concat(em_path, "/syslog/rights"));
	if(lines.size() < 2) {
		return;
	}
	for(int i = 1; i < lines.size(); i++) {
		if(i == status) {
			char buf[200];
			char f[150];
			strcpy(buf, lines[i].c_str());
			strcat(buf, concat(concat(getcwd(f, 200), "/"), file));
			strcat(buf, "(");
			strcat(buf, to_string(right).c_str());
			strcat(buf, ")");
			strcat(buf, ":|:\n");
			appendFile(buf, concat(em_path, "/syslog/rights"));
		} else {
			if(strcmp(lines[i].c_str(), "") == 0) continue;
			char tmp[150];
			strcpy(tmp, lines[i].c_str());
			appendFile(concat(tmp, "\n"), concat(em_path, "/syslog/rights"));
		}
	}
}

void deleteRights(int status, const char* file) {
	char f[200];
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/rights")));
	writeFile(lines[0].c_str(), concat(em_path, "/syslog/rights"));
	appendFile("\n", concat(em_path, "/syslog/rights"));
	if(lines.size() < 2) {
		return;
	}
	for(int i = 1; i < lines.size(); i++) {
		if(i == status) {
			char buf[500];
			strcpy(buf, "");
			char b[200];
			strcpy(b, file);
			int start = lines[i].find(concat(b, "("));
			int end = lines[i].find(":|:", start);
			strcpy(buf, lines[i].substr(0, start).c_str());
			strcat(buf, lines[i].substr(end+3).c_str());
			strcat(buf, "\n");
			appendFile(buf, concat(em_path, "/syslog/rights"));
			
		} else {
			if(strcmp(lines[i].c_str(), "") == 0) continue;
			char tmp[150];
			strcpy(tmp, lines[i].c_str());
			appendFile(concat(tmp, "\n"), concat(em_path, "/syslog/rights"));
		}
	}
}

void split(string s, vector<string> &v){
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

vector<string> split_delim(string str, string lim) {
	vector<string> lines;
	int len = lim.size();
	int i = 0;
	int prev = 0;
	while((i = str.find(lim, i+1)) != -1) {
		lines.push_back(str.substr(prev+3, i-prev-len));
		prev = i;
	}
	lines.push_back(str.substr(prev+len));
	lines[lines.size()-1] = lines[lines.size()-1].substr(0, lines[lines.size()-1].size()-3);
	return lines;
}

char* pwd() {
	char s[200];
	strcpy(s, "");
	return getcwd(s, 200);
}

void quit() {
	chmod(concat(em_path, "/A"), 0);
	chmod(concat(em_path, "/C"), 0);
	unblock();
}

void cd(string dir, int status) {
	unblock();
	if(strcmp(dir.c_str(), "..") != 0 && getRights(status, dir.c_str()) < 1) {
		access_denied(status);
		block();
		return;
	}
	chmod(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	chdir(dir.c_str());
	block();
}

void create_dir(string dir, int status) {
	unblock();
	char p[150];
	if(getRights(status, getcwd(p, 150)) != 1 && getRights(status, getcwd(p, 150)) != 4) {
		access_denied(status);
		return;
	}
	mkdir(dir.c_str(), S_IRWXU | S_IRWXU | S_IRWXO);
	if (status != 0)
		addRights(status, dir.c_str(), 1);
	block();
}

void create_file(string file, int status) {
	unblock();
	char p[150];
	if(getRights(status, getcwd(p, 150)) != 1 && getRights(status, getcwd(p, 150)) != 4) {
		access_denied(status);
		return;
	}
	int i = fork();
	if (i == 0) {
		execl("/bin/touch", "/bin/touch", file.c_str(), NULL);
		exit(0);
	} else {
		int status;
		wait(&status);
		chmod(file.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	}
	addRights(status, file.c_str(), 1);
	block();
}

void remove_dir(string file, int status) {
	unblock();
	char p[150];
	char* path = concat(concat(getcwd(p, 150), "/"), file.c_str());
	if(getRights(status, path) != 1 && getRights(status, path) != 4)  {
		access_denied(status);
		block();
		return;
	}
	for(int i = 0; i < getLines(readFile(concat(em_path, "/syslog/rights"))).size(); i++) {
		if (status == 0) break;
		if (i == 0) continue;
		deleteRights(i, path);
	}
	
	remove(file.c_str());
	block();
}

void cat(string file, int status) {
	unblock();
	
	char buf[150];
	getcwd(buf, 150);
	strcat(buf, "/");
	strcat(buf, file.c_str());
	if (getRights(status, buf) < 1 && status != 0) {
		access_denied(status);
		return;
	}
	int i = fork();
	if (i == 0) {
		execl("/bin/cat", "/bin/cat", file.c_str(), NULL);
		exit(0);
	} else {
		int status;
		wait(&status);
		block();
	}
}

void vi(string file, int status) {
	unblock();
	char buf[150];
	getcwd(buf, 150);
	strcat(buf, "/");
	strcat(buf, file.c_str());
	if (getRights(status, buf) < 1 || getRights(status, buf) == 2 && status != 0) {
		access_denied(status);
		return;
	}
	int i = fork();
	if (i == 0) {
		execl("/bin/vi", "/bin/vi", file.c_str(), NULL);
		exit(0);
	} else {
		int status;
		wait(&status);
		block();
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
	chmod(em_path, S_IRWXU | S_IRWXG | S_IRWXO);
	DIR *dir;
	struct dirent *file;
	dir = opendir(".");
	while((file = readdir(dir)) != NULL) {
		printf("%s\n", file->d_name);
	}
	closedir(dir);
	chmod(em_path, 0);
}

int validateUser(char* name, char* pswd) {
	if(strcmp(name, ADMIN_LOGIN) == 0 & strcmp(pswd, ADMIN_PSWD) == 0) {
		return 0;
	}
	char s[100];
	char cr[100];
	strcat(cr, name);
	strcat(cr, ":|:");
	strcat(cr, pswd);
	char encrypted[140];
	strcpy(encrypted, "");
	strcat(encrypted, ":<:");
	strcat(encrypted, enc(cr));
	unblock();
	
	char* text = readFile(concat(getcwd(s, 100), "/syslog/pswd"));
	vector<string> lines = getLines(text);
	for(int i = 0; i < lines.size(); i++) {
		string decLine = enc(lines[i].substr(3).c_str());
		if (lines[i].find(encrypted) != -1) {
			if(decLine.find(":|:", decLine.find(":|:")+1)-decLine.find(":|:")-3 == strlen(pswd))
				return i;
		}
	}
	block();
	
	return -1;
}

int findUser(char* name) {
	if(strcmp(name, ADMIN_LOGIN) == 0) {
		return 0;
	}
	char s[100];
	char cr[100];
	strcpy(cr, name);
	strcat(cr, ":|:");
	char encrypted[140];
	strcpy(encrypted, "");
	strcat(encrypted, ":<:");
	strcat(encrypted, enc(cr));
	unblock();
	
	char* text = readFile(concat(em_path, "/syslog/pswd"));
	vector<string> lines = getLines(text);
	for(int i = 0; i < lines.size(); i++) {
		string decLine = enc(lines[i].substr(3).c_str());
		if (lines[i].find(encrypted) != -1) {
			return i;
		}
	}
	block();
	
	return -1;
}

void changeRights(string right, string file, string user, int status) {
	unblock();
	char name[100];
	strcpy(name, user.c_str());
	int user_ind = findUser(name);
	if (user_ind == -1) {
		cout << "There is no such user" << endl;
		block();
		return;
	}
	char p[150];
	getcwd(p, 150);
	strcat(p, "/");
	if (getRights(status, concat(p, file.c_str())) != 1) {
		access_denied(status);
		block();
		return;
	}
	if (status == 0) {
		block();
		return;
	}
	if (getRights(user_ind, concat(p, file.c_str())) != -1) {
		deleteRights(user_ind, concat(p, file.c_str()));
	}
	addRights(user_ind, file.c_str(), stoi(right));
	block();
}

void deleteUser(int status) {
	unblock();
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/pswd")));
	vector<string> rights = getLines(readFile(concat(em_path, "/syslog/rights")));
	vector<string> questions = getLines(readFile(concat(em_path, "/syslog/questions")));
	vector<string> authent = getLines(readFile(concat(em_path, "/syslog/auth")));
	writeFile(lines[0].c_str(), concat(em_path, "/syslog/pswd"));
	appendFile("\n", concat(em_path, "/syslog/pswd"));
	writeFile(rights[0].c_str(), concat(em_path, "/syslog/rights"));
	appendFile("\n", concat(em_path, "/syslog/rights"));
	writeFile(questions[0].c_str(), concat(em_path, "/syslog/questions"));
	appendFile("\n", concat(em_path, "/syslog/questions"));
	writeFile(authent[0].c_str(), concat(em_path, "/syslog/auth"));
	appendFile("\n", concat(em_path, "/syslog/auth"));
	if(lines.size() < 2) {
		block();
		return;
	}
	
	for(int i = 1; i < lines.size(); i++) {
		if(status == i) {
			appendFile("", concat(em_path, "/syslog/pswd"));
			appendFile("", concat(em_path, "/syslog/rights"));
			appendFile("", concat(em_path, "/syslog/questions"));
			appendFile("", concat(em_path, "/syslog/auth"));
		} else {
			appendFile(lines[i].c_str(), concat(em_path, "/syslog/pswd"));
			appendFile("\n", concat(em_path, "/syslog/pswd"));
			appendFile(rights[i].c_str(), concat(em_path, "/syslog/rights"));
			appendFile("\n", concat(em_path, "/syslog/rights"));
			appendFile(questions[i].c_str(), concat(em_path, "/syslog/questions"));
			appendFile("\n", concat(em_path, "/syslog/questions"));
			appendFile(questions[i].c_str(), concat(em_path, "/syslog/auth"));
			appendFile("\n", concat(em_path, "/syslog/auth"));
		}
	}
	block();
}

bool auth_check(int status) {
	if (status == 0) {
		return true;
	}
	time_t start = time(NULL);
	unblock();
	string cred = getLines(readFile(concat(em_path, "/syslog/pswd")))[status];
	string cr = enc(cred.substr(3, cred.size()-6).c_str());
	int l = cr.find_last_of(":|:", cr.find_last_of(":|:")-3);
	cr = cr.substr(l+1);
	srand(time(NULL));
	if (rand() % 100 >= 50) {
		int req = rand() % 100;
		int ans = (int)tan(req*14);
		cout << "Input answer according to this number: " << req << endl;
		char inp[20];
		cin.getline(inp, 20);
		if ((time(NULL) - start) >= 8 || atoi(inp) != ans) {
			block();
			return false;
		}
	} else {
		chmod(concat(em_path, "/syslog/questions"), S_IRWXU | S_IRWXG | S_IRWXO);
		string line = getLines(readFile(concat(em_path, "/syslog/questions")))[status];
		string quest = enc(line.substr(3, line.size()-7).c_str());
		
		int lim_q = cr.find(":|:");
		int lim_a = quest.find(":|:");
		char answer[150];
		strcpy(answer, "");
		cout << "Input answer on that question" << endl;
		char check[150];
		if (rand() % 100 >= 50) {
			cout << quest.substr(0, lim_q) << endl;
			strcpy(check, cr.substr(0, lim_a).c_str());
		} else {
			cout << quest.substr(lim_q+3) << endl;
			strcpy(check, cr.substr(lim_a+3).c_str());
		}
			
		cin.getline(answer, 150);
		if ((time(NULL) - start) >= 8 || strcmp(answer, check) != 0) {
			block();
			return false;
		}
		chmod(concat(em_path, "/syslog/questions"), 0);
	}
	block();
	return true;
}

void reset_count_denied_access(int status) {
	unblock();
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/auth")));
	string line = lines[status];
	char buf[200];
	strcpy(buf, line.substr(0, line.find(":|:")+3).c_str());
	strcat(buf, "0");
	strcat(buf, line.substr(line.find(":|:")+4).c_str());
	line = buf;
	lines[status] = line;
	writeFile("", concat(em_path, "/syslog/auth"));
	for(int i = 0; i < lines.size()-1; i++) {
		appendFile(lines[i].c_str(), concat(em_path, "/syslog/auth"));
		appendFile("\n", concat(em_path, "/syslog/auth"));
	}
	appendFile(lines[lines.size()-1].c_str(), concat(em_path, "/syslog/auth"));
	block();
}

void increase_count_denied_access(int status) {
	unblock();
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/auth")));
	string line = lines[status];
	int count = stoi(line.substr(line.find(":|:")+3, 1));
	if (count == 3) {
		deleteUser(status);
		block();
		return;
	}
	char buf[200];
	strcpy(buf, line.substr(0, line.find(":|:")+3).c_str());
	strcat(buf, to_string(count+1).c_str());
	strcat(buf, line.substr(line.find(":|:")+4).c_str());
	line = buf;
	lines[status] = line;
	writeFile("", concat(em_path, "/syslog/auth"));
	for(int i = 0; i < lines.size()-1; i++) {
		appendFile(lines[i].c_str(), concat(em_path, "/syslog/auth"));
		appendFile("\n", concat(em_path, "/syslog/auth"));
	}
	appendFile(lines[lines.size()-1].c_str(), concat(em_path, "/syslog/auth"));
	
	block();
}

void authentication(time_t* last_check, int status) {
	if (time(NULL) - *last_check >= CHECK_INTERVAL) {
		if (!auth_check(status)) {
			access_denied(status);
			increase_count_denied_access(status);
			quit();
			exit(0);
		}
		reset_count_denied_access(status);
		*last_check = time(NULL);
	}
}

void assess_danger(int amount, long interval) {
	double v = (double)amount / ((double)interval / (24 * 60 * 60));
	cout << "dangerousness of this user is " << round(v / 5) << endl;
}

void getReport() {
	unblock();
	vector<string> lines = getLines(readFile(concat(em_path, "/syslog/seclog")));
	time_t last_report = stoi(lines[0]);
	if (time(NULL) - last_report < REPORT_INTERVAL && lines.size() < 2) {
		block();
		return;
	}
	cout << "\nREPORT ON SUSPICIOUS ACTIONS FROM MONITORING LOGS" << endl;
	cout << "since ";
	cout << ctime(&last_report);
	cout << endl;
	
	for (int i = 1; i < lines.size(); i++) {
		vector<string> user = split_delim(lines[i], ":|:");
		
		if (user.size() < 2) {
			cout << user[0] << " :\n" << endl;
			cout << "Here is no records about actions of this user" << endl;
			cout << "--------------------------------\n" << endl;
			continue;
		}
		for (int i = 0; i < user.size(); i++) {
			if (i == 0) {
				cout << user[i];
				cout << " :\n" << endl;	
			} else {
				time_t l = stoi(user[i]);
				cout << ctime(&l);
			}
		}
		assess_danger(user.size()-1, time(NULL) - last_report);
		cout << "-----------------------" << endl;
		cout << endl;
	}
	for (int i = 0; i < lines.size(); i++) {
		if (i == 0) {
			writeFile(to_string(time(NULL)).c_str(), concat(em_path, "/syslog/seclog"));
		} else {
			char buf[100];
			strcpy(buf, "\n:<:");
			strcat(buf, split_delim(lines[i], ":|:")[0].c_str());
			strcat(buf, ":>:");
			appendFile(buf, concat(em_path, "/syslog/seclog"));
		}
	}
	block();
}

int main() {
	time_t last_check = time(NULL);
	getcwd(prog_dir, 200);
	strcpy(em_path, concat(prog_dir, "/emulator"));
	chmod(em_path, S_IRWXU | S_IRWXG | S_IRWXO);
	char login[MAX_LOGIN_LEN], pswd[MAX_PASSWORD_LEN], line[MAX_COMMAND_LEN], command[15];
	cd("emulator", 0);
	
	(void)signal(SIGINT, inter_handler);
	
	std::cout << "Enter your login:\n";
	std::cin.getline(login, MAX_LOGIN_LEN);
	std::cout << "Enter your password:\n";
	std::cin.getline(pswd, MAX_PASSWORD_LEN);
	
	int status = validateUser(login, pswd);
	
	if(status == -1) {
		quit();
		return 0;
	}
	
	if (status == 0) {
		getReport();
	}
	
	while(true) {
		authentication(&last_check, status);
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
			printf("%s\n", pwd());
		} else if (strcmp(command, "cd") == 0) {
			if (words.size() > 1)
				cd(words[1], status);
		} else if (strcmp(command, "mkdir") == 0) {
			if (words.size() > 1) 
				create_dir(words[1], status);
		} else if (strcmp(command, "touch") == 0) {
			if (words.size() > 1) 
				create_file(words[1], status);
		} else if (strcmp(command, "rm") == 0) {
			if (words.size() > 1)
				remove_dir(words[1], status);
		} else if (strcmp(command, "vi") == 0) {
			if (words.size() > 1) {
				if (words.size() > 1) {
					vi(words[1], status);
				}
			}
		} else if (strcmp(command, "cat") == 0) {
			if (words.size() > 1) {
				cat(words[1], status);
			}
		} else if (strcmp(command, "chmod") == 0) {
			if (words.size() == 4) {
				changeRights(words[1], words[2], words[3], status);
			}
		}
	}
}

