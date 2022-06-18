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
#include <conio.h>

using namespace std;

char em_path[100];

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
  return buf;
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

void writeFile(const char* text, char* filename) {
	FILE *fp = NULL;
  if ((fp = fopen(filename, "w")) == NULL) {
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

vector<string> getNamesPswd(string text) {
	vector<string> names;
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
	if(v.size() < 2) {
		return names;
	}
	for (int i = 1; i < v.size(); i++) {
		string name;
		string str = enc(v[i].substr(3).c_str());
		int j = str.find(":|:"); 
		name = str.substr(0, j);
		names.push_back(name);
	}
	return names;
}

vector<string> getNames(string text) {
	vector<string> names;
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
	for (int i = 0; i < v.size(); i++) {
		string name;
		int j = v[i].find(":|:"); 
		name = v[i].substr(0, j);
		names.push_back(name);
	}
	return names;
}

char* readFile(char* filename) {
	char* res = new char[1000];
	strcpy(res, "");
	std::ifstream file(filename);
	if (file.is_open()) {
    string line;
    while(getline(file, line)) {
    	strcat(res, line.c_str());
    	strcat(res, "\n");
    }
  	file.close();
	}
	return res;
}

int findUserNameRequest(char* name) {
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/request.txt"));
	vector<string> names = getNames(text);
	for(int i = 0; i < names.size(); i++) {
		if (strcmp(names[i].c_str(), name) == 0) {
			return i;
		}
	}
	return -1;
}

int findUserNamePswd(char* name) {
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/syslog/pswd"));
	vector<string> names = getNamesPswd(text);
	for(int i = 0; i < names.size(); i++) {
		if (strcmp(names[i].c_str(), name) == 0) {
			return i;
		}
	}
	return -1;
}

void unregistrateUser() {
	int ind = 0;
	FILE *fp = NULL;
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/request.txt"));
  if ((fp = fopen(concat(cwd, "/emulator/request.txt"), "w")) == NULL) {
   	perror("error"); 
   	exit(1);
  }
  vector<string> lines = getLines(text);
  char* buf = new char[1000];
  strcpy(buf, "");
  for(int i = 0; i < lines.size(); i++) {
  	if (i != ind) {
	  	strcat(buf, lines[i].c_str());
	  	strcat(buf, "\n");
	  }
  }
  fprintf(fp, buf);
}


string readNextCred() {
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/request.txt"));
	int i = 0;
	string line;
	if ((int)text[i] == 0) {
		return line;
	};
	while(text[i] != '\n') {
		line.push_back(text[i]);
		i++;
	}
	return line;
}

bool showCred(string line) {
	if (line.size() == 0) {
		cout << "There is no any users." << endl;
		return false;
	}
	int name_ind = line.find(":|:");
	int pswd_ind = line.find(":|:", name_ind+1);
	int q1_ind = line.find(":|:", pswd_ind+1);
	int ans1_ind = line.find(":|:", q1_ind+1);
	int q2_ind = line.find(":|:", ans1_ind+1);
	
	printf("name: %s; pswd: %s;\nanswer 1: %s; question 1: %s;\nanswer 2: %s; question 2: %s\n", line.substr(0, name_ind).c_str(), line.substr(name_ind+3, pswd_ind-name_ind-3).c_str(), line.substr(pswd_ind+3, q1_ind-pswd_ind-3).c_str(), line.substr(q1_ind+3, ans1_ind-q1_ind-3).c_str(), line.substr(ans1_ind+3, q2_ind-ans1_ind-3).c_str(), line.substr(q2_ind+3).c_str());
	return true;
}

bool validation() {
	string inputpswd;
	char ch;
	while((int)(ch = getch()) != 10) {
		inputpswd.push_back(ch);
		_putch('*');
	}
	
	if(inputpswd.length() != 10) {
		cout << endl;
		exit(0);
	}

	string cred = readFile("/home/oleg/emulator/syslog/pswd");
	string cr = enc(cred.substr(3).c_str());
	int ind = cr.find("-");
	string pswd = cr.substr(ind+1, 10);
	return (strcmp(pswd.c_str(), inputpswd.c_str()) == 0);
}



int validateUser(char* name, char* pswd) {
	char cr[100];
	strcat(cr, name);
	strcat(cr, ":|:");
	strcat(cr, pswd);
	char encrypted[140];
	strcpy(encrypted, "");
	strcat(encrypted, ":<:"); 
	strcat(encrypted, enc(cr));
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/syslog/pswd"));
	vector<string> lines = getLines(text);
	for(int i = 0; i < lines.size(); i++) {
		string decLine = enc(lines[i].substr(3).c_str());
		if (lines[i].find(encrypted) != -1) {
			if(decLine.find(":|:", decLine.find(":|:")+1)-decLine.find(":|:")-3 == strlen(pswd))
				return i;
		}
	}
	return -1;
}

bool validPswd(char* pswd) {
	for (int i = 0; i < strlen(pswd); i++) {
		if ((int)pswd[i] >= 48 && (int)pswd[i] <= 57 || (int)pswd[i] >= 65 && (int)pswd[i] <= 90 || (int)pswd[i] >= 97 && (int)pswd[i] <= 122) {}
		else {
			return false;
		}
	}
	return true;
}

void changePswd(char* pswd, int ind) {
	if (!validPswd(pswd)) {
		cout << "Password should only have letters and digits." << endl;
		return;
	}
	if(strlen(pswd) < 5 || strlen(pswd) > 10) {
		cout << "Password should have 5-10 symbols" << endl;
		return;
	}
	char cwd[100], res[200], buf[100], wr[400];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/syslog/pswd"));
	vector<string> lines = getLines(text);
	int fin = lines[ind].find(":>:");
	string nwline = enc(lines[ind].substr(3, fin-4).c_str());
	int beg = nwline.find(":|:");
	int end = nwline.find(":|:", beg+3);
	strcpy(res, "");
	strcpy(buf, "");
	strcat(res, ":<:");
	strcat(buf, nwline.substr(0, beg).c_str());
	strcat(buf, ":|:");
	strcat(buf, pswd);
	strcat(buf, nwline.substr(end, lines[ind].size()-end).c_str());
	strcat(res, enc(buf));
	strcat(res, ":>:");
	writeFile(lines[0].c_str(), concat(cwd, "/emulator/syslog/pswd"));
	appendFile("\n", concat(cwd, "/emulator/syslog/pswd"));
	if(lines.size() < 2) {
		return;
	}
	for(int i = 1; i < lines.size(); i++) {
		if(ind == i) {
			appendFile(res, concat(cwd, "/emulator/syslog/pswd"));
		} else {
			appendFile(lines[i].c_str(), concat(cwd, "/emulator/syslog/pswd"));
		}
		appendFile("\n", concat(cwd, "/emulator/syslog/pswd"));
	}
}

void change() {
	char name[20], pswd[20];
	cout << "Input your name: " << endl;
	cin.getline(name, 20);
	cout << "Input your old password: " << endl;
	cin.getline(pswd, 20);
	int ind = validateUser(name, pswd);
	if (ind != -1) {
		cout << "Input your new password: " << endl;
		cin.getline(pswd, 20);
		changePswd(pswd, ind);
	};
} 

void registration() {
	char credentials[170];
	char name[15], pswd[10], qs1[40], qs2[40], ans1[20], ans2[20];
	cout << "Input your name:" << endl;
	cin.getline(name, 15);
	while(findUserNameRequest(name) != -1 || strlen(name) < 1 || findUserNamePswd(name) != -1) {
		cout << "Name should be unique and has at least 1 symbol." << endl;
		cin.getline(name, 15);
	}
	while(true) {
		cout << "Input your password (length should be 5-10 symbols: letters and digits):" << endl;
		cin.getline(pswd, 10);
		if (strlen(pswd) > 4 && strlen(pswd) < 11 && validPswd(pswd))
			break;
	}
	while(true) {
		cout << "Input 1st question that will be used for authenticated:" << endl;
		cin.getline(qs1, 40);
		if (strlen(qs1) > 0)
			break;
	}
	while(true) {
		cout << "Input answer on 1st question:" << endl;
		cin.getline(ans1, 20);
		if (strlen(ans1) > 0)
			break;
	}
	while(true) {
		cout << "Input 2nd question that will be used for authenticated:" << endl;
		cin.getline(qs2, 40);
		if (strlen(qs2) > 0)
			break;
	}
	while(true) {
		cout << "Input answer on 2nd question:" << endl;
		cin.getline(ans2, 20);
		if (strlen(ans2) > 0)
			break;
	}
	cout << "Your credentials were recorded. Wait until admin registrate in system." << endl;
	cout << "Remember the function y=tg(a*x), a = 14, x will be sent you and you should type rounded y as answer to complete authentication." << endl;
	strcpy(credentials, "");
	strcat(credentials, name);
	strcat(credentials, ":|:");
	strcat(credentials, pswd);
	strcat(credentials, ":|:");
	strcat(credentials, qs1);
	strcat(credentials, ":|:");
	strcat(credentials, ans1);
	strcat(credentials, ":|:");
	strcat(credentials, qs2);
	strcat(credentials, ":|:");
	strcat(credentials, ans2);
	strcat(credentials, "\n");
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	appendFile(credentials, concat(cwd, "/emulator/request.txt"));
}

void registrateUser(string line) {
	char* buf = new char[400];
	strcpy(buf, "");
	char* res = new char[200];
	strcpy(res, "");
	int name_ind = line.find(":|:");
	int pswd_ind = line.find(":|:", name_ind+1);
	int q1_ind = line.find(":|:", pswd_ind+1);
	int ans1_ind = line.find(":|:", q1_ind+1);
	int q2_ind = line.find(":|:", ans1_ind+1);
	
	string name = line.substr(0, name_ind).c_str();
	string pswd = line.substr(name_ind+3, pswd_ind-name_ind-3).c_str();
	string ans1 = line.substr(pswd_ind+3, q1_ind-pswd_ind-3).c_str();
	string q1 = line.substr(q1_ind+3, ans1_ind-q1_ind-3).c_str();
	string ans2 = line.substr(ans1_ind+3, q2_ind-ans1_ind-3).c_str();
	string q2 = line.substr(q2_ind+3).c_str();
	
	unregistrateUser();
	
	strcat(buf, name.c_str());
	strcat(buf, ":|:");
	strcat(buf, pswd.c_str());
	strcat(buf, ":|:");
	strcat(buf, ans1.c_str());
	strcat(buf, ":|:");
	strcat(buf, ans2.c_str());
	
	char a[2], b[2];
	while(true) {
		cout << "Input rights of access to A directory: 0 - access denied, 1 - owner, 2 - minimal rights, 3 - medium" << endl;
		cin.getline(a, 2);
		if(strcmp(a, "1") == 0 || strcmp(a, "2") == 0 || strcmp(a, "3") == 0 || strcmp(a, "0") == 0) {
			break;
		}
	}
	while(true) {
		cout << "Input rights of access to C directory: 0 - access denied, 1 - owner, 2 - minimal rights, 3 - medium" << endl;
		cin.getline(b, 2);
		if(strcmp(b, "1") == 0 || strcmp(b, "2") == 0 || strcmp(b, "3") == 0 || strcmp(b, "0") == 0) {
			break;
		}
	}
	
	strcat(res, ":<:");
	strcat(res, enc(buf));
	strcat(res, ":>:\n");
	
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	appendFile(res, concat(cwd, "/emulator/syslog/pswd"));
	
	strcpy(res, "");
	strcpy(buf, "");
	
	strcat(buf, q1.c_str());
	strcat(buf, ":|:");
	strcat(buf, q2.c_str());
	
	strcat(res, ":<:");
	strcat(res, enc(buf));
	strcat(res, ":>:\n");
	appendFile(res, concat(cwd, "/emulator/syslog/questions"));
	char n[400];
	char path[150];
	strcpy(path, cwd);
	strcat(path, "/emulator/");
	strcpy(n, name.c_str());
	strcat(n, ":|:");
	strcat(n, concat(path, "A("));
	strcat(n,  a);
	strcat(n, "):|:");
	
	strcat(n, concat(path, "C("));
	strcat(n,  b);
	strcat(n, "):|:");
	appendFile(concat(n, "\n"), concat(cwd, "/emulator/syslog/rights"));
	char auth_cred[150];
	strcpy(auth_cred, ":<:");
	strcat(auth_cred, name.c_str());
	strcat(auth_cred, ":|:");
	strcat(auth_cred, "0");
	strcat(auth_cred, ":>:");
	appendFile(auth_cred, concat(cwd, "/emulator/syslog/auth"));
}

int findUser(char* name) {
	char cr[100];
	strcpy(cr, "");
	strcat(cr, name);
	strcat(cr, ":|:");
	char encrypted[140];
	strcpy(encrypted, "");
	strcat(encrypted, ":<:");
	strcat(encrypted, enc(cr));
	
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	char* text = readFile(concat(cwd, "/emulator/syslog/pswd"));
	vector<string> lines = getLines(text);
	for(int i = 0; i < lines.size(); i++) {
		if (lines[i].find(encrypted) != -1) {
			return i;
		}
	}
	return -1;
}

void deleteUser(char* name) {
	int ind = findUser(name);
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	vector<string> lines = getLines(readFile(concat(cwd, "/emulator/syslog/pswd")));
	vector<string> rights = getLines(readFile(concat(cwd, "/emulator/syslog/rights")));
	vector<string> questions = getLines(readFile(concat(cwd, "/emulator/syslog/questions")));
	writeFile(lines[0].c_str(), concat(cwd, "/emulator/syslog/pswd"));
	appendFile("\n", concat(cwd, "/emulator/syslog/pswd"));
	writeFile(rights[0].c_str(), concat(cwd, "/emulator/syslog/rights"));
	appendFile("\n", concat(cwd, "/emulator/syslog/rights"));
	writeFile(questions[0].c_str(), concat(cwd, "/emulator/syslog/questions"));
	appendFile("\n", concat(cwd, "/emulator/syslog/questions"));
	if(lines.size() < 2) {
		return;
	}
	
	for(int i = 1; i < lines.size(); i++) {
		if(ind == i) {
			appendFile("", concat(cwd, "/emulator/syslog/pswd"));
			appendFile("", concat(cwd, "/emulator/syslog/rights"));
			appendFile("", concat(cwd, "/emulator/syslog/questions"));
		} else {
			appendFile(lines[i].c_str(), concat(cwd, "/emulator/syslog/pswd"));
			appendFile("\n", concat(cwd, "/emulator/syslog/pswd"));
			appendFile(rights[i].c_str(), concat(cwd, "/emulator/syslog/rights"));
			appendFile("\n", concat(cwd, "/emulator/syslog/rights"));
			appendFile(questions[i].c_str(), concat(cwd, "/emulator/syslog/questions"));
			appendFile("\n", concat(cwd, "/emulator/syslog/questions"));
		}
	}
}

bool isFull() {
	int num = 0;
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	vector<string> lines = getLines(readFile(concat(cwd, "/emulator/syslog/pswd")));
	for(int i = 0; i < lines.size(); i++) {
		if(lines[i].find(":<:") > -1) {
			num++;
		};
	}
	return num == 4;
}

void admin() {
	char input[8];
	cout << "You are authenticated as admin." << endl;
	while(true) {
		cout << "Type next/n to get next user's credentials for regestration; remove/r to remove user from register list; quit/q to quit from this program." << endl;
		cin.getline(input, 8);
		if(strcmp(input, "q") == 0 || strcmp(input, "quit") == 0) {
			exit(0);
		}	else if(strcmp(input, "n") == 0 || strcmp(input, "next") == 0) {
			if(isFull()) {
				cout << "Register list is filled. You can't add users." << endl;
				continue;
			}
			char answer[5];
			string userCred = readNextCred();
			if(showCred(userCred)) {
				cout << "To register that user type y/yes; to remove type n/no" << endl;
				cin.getline(answer, 5);
				if(strcmp(answer, "n") == 0 || strcmp(answer, "no") == 0) {
					unregistrateUser();
				} else if (strcmp(answer, "y") == 0 || strcmp(answer, "yes") == 0) {
					registrateUser(userCred);
				}
			}
		} else if(strcmp(input, "r") == 0 || strcmp(input, "remove") == 0) {
			cout << "Type name of user you want to delete." << endl;
			char name[20];
			cin.getline(name, 20);
			deleteUser(name);
		}
	}
}

void user() {
	char input[5];
	cout << "You was authenticated as user." << endl;
	cout << "If you want to registrate, type 'r'; if you want to change your password, type 'c'; if you want to quit, type 'q'" << endl;
	cin.getline(input, 5);
	if (strcmp(input, "r") == 0) {
		registration();
	}	else if (strcmp(input, "c") == 0) {
		change();
	}	else if (strcmp(input, "q") == 0) {
		chmod(concat(em_path, "/request.txt"), 0);
		chmod(concat(em_path, "/syslog/rights"), 0);
		chmod(concat(em_path, "/syslog/pswd"), 0);
		chmod(concat(em_path, "/syslog/auth"), 0);
		chmod(concat(em_path, "/syslog/questions"), 0);
		chmod(concat(em_path, "/syslog"), 0);
		chmod(em_path, 0);
		exit(0);
	}
}

int main() {
	getcwd(em_path, 100);
	strcat(em_path, "/emulator");
	char ans[3], pswd[10];
	
	chmod(em_path, S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/request.txt"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/rights"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/pswd"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/questions"), S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(concat(em_path, "/syslog/auth"), S_IRWXU | S_IRWXG | S_IRWXO);
	
	std::cout << "Are you admin? (type y/yes or n/no)" << std::endl;
	std::cin.getline(ans, 3);
	if (strcmp(ans, "y") == 0 || strcmp(ans, "yes") == 0) {
		if (geteuid() != 0) {
			cout << "Access denied. Run with root rights." << endl;
			exit(0);
		}
		cout << "Input emulator admin password" << endl;
		if(validation()) {
			admin();
		} else {
			chmod(concat(em_path, "/request.txt"), 0);
			chmod(concat(em_path, "/syslog/rights"), 0);
			chmod(concat(em_path, "/syslog/pswd"), 0);
			chmod(concat(em_path, "/syslog/questions"), 0);
			chmod(concat(em_path, "/syslog/auth"), 0);
			chmod(concat(em_path, "/syslog"), 0);
			chmod(em_path, 0);
			exit(0);
		}
	} else {
		user();
	}
	chmod(concat(em_path, "/request.txt"), 0);
	chmod(concat(em_path, "/syslog/rights"), 0);
	chmod(concat(em_path, "/syslog/pswd"), 0);
	chmod(concat(em_path, "/syslog/questions"), 0);
	chmod(concat(em_path, "/syslog/auth"), 0);
	chmod(concat(em_path, "/syslog"), 0);
	chmod(em_path, 0);
	return 0;
}

