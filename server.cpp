#include <iostream>
#include <string>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <fstream>

#define FAILED	(-1)
#define OK			(0)

using std::cout;
using std::endl;

using std::string;
using std::map;
using std::fstream;

class Led {
public:
	Led() {
		state = false;
		color = RED;
		rate = 0;
	}

	typedef enum {
		OFF,
		RED,
		GREEN,
		BLUE,
	} Color;
	
	int setState(bool st) { 
		state = st;
		return OK;
	}
	bool getState() const{ return state;}

	int setColor(Color col) { 
		if (state && col !=OFF) 
			color = col;
		else return FAILED;
		return OK;
	}
	Color getColor() const{ if (state) return color;else return OFF;}

	int setRate(float rt) { 
		if (rt >= MIN_RATE && rt <= MAX_RATE) 
			rate = rt; 
		else return FAILED;
		return OK;
	}
	float getRate() const { if (state) return rate; else return FAILED;}

private:
	bool state;
	Color color;
	static const float MIN_RATE = 0;
	static const float MAX_RATE = 4;
	float rate;
};

string* setState(Led& pled, string& com)
{
	if ((com == "on" && pled.setState(true) == OK)
		|| (com == "off" && pled.setState(false) == OK))
		com = "OK\n";
	else com = "FAILED\n";
	return &com;
}

string* getState(Led& pled, string& com)
{
	if (pled.getState()) com = "OK on\n";
	else com = "OK off\n";
	return &com;
}

string* setColor(Led& p, string& com)
{
	if ( (com == "red" && p.setColor(Led::RED) == OK)
			|| (com == "green" && p.setColor(Led::GREEN) == OK)
			|| (com == "blue" && p.setColor(Led::BLUE) == OK))
			com = "OK\n";
	else com = "FAILED\n";

	return &com;
}

string* getColor(Led& p, string& com)
{
	switch(p.getColor()) {
		case Led::RED: com = "OK red\n"; break;
		case Led::GREEN: com = "OK green\n"; break;
		case Led::BLUE: com = "OK blue\n"; break;
		default: com = "FAILED\n"; break;
	}
	return &com;
}

string* setRate(Led& p, string& com)
{
	float f;
	std::stringstream data(com);
	data >> f;
	if (p.setRate(f) == OK) com = "OK\n";
	else com = "FAILED\n";

	return &com;
}

string* getRate(Led& p, string& com)
{
	if (p.getRate() != FAILED) {
		std::ostringstream s;
		s << p.getRate();
		com = "OK " + s.str() + "\n";
	}
	else com = "FAILED\n";

	return &com;
}

int main(int argc, char *argv[])
{
	Led led;
	std::ostream *output = &cout;
	std::istream *input = &std::cin;

	int fd = 0;
	
	if (argc > 1) {
		unlink(argv[1]);
		if ((mkfifo(argv[1], 0666)) == -1) cout << "Error create NAME_FIFO_IN: " << argv[1];
		else {
			fstream *fin = new fstream(argv[1]);
			if (fin->is_open()) {
				input = fin;
				cout << "Input good!" << endl;
			}
			else cout << "Error open NAME_FIFO_IN: " << argv[1];
		}

		if (argc > 2) {
			unlink(argv[2]);
			if ((mkfifo(argv[2], 0666)) == -1) cout << "Error create NAME_FIFO_OUT: " << argv[2];
			else {
				if ((fd = open(argv[2], O_WRONLY)) != -1) {
					cout << "Output good!" <<endl;
				}
				else cout << "Error open NAME_FIFO_OUT: " << argv[2];
			}
		}
		else cout << "NAME_FIFO_OUT: standart out\n";
	}
	else cout << "USE: server NAME_FIFO_IN NAME_FIFO_OUT\n";
	
	map<string, string* (*)(Led&, string&)> command;
	command["set-led-state"] = setState;
	command["get-led-state"] = getState;
	command["set-led-color"] = setColor;
	command["get-led-color"] = getColor;
	command["set-led-rate"]  = setRate;
	command["get-led-rate"]  = getRate;
	
	string str = "";
	long int count_command = 0;
	while(1) {
		// VIEW in Server
		cout << "\nState: ";
		if (led.getState()) cout << "ON";
		else cout << "OFF";
		cout << endl;
		cout << "Color: ";
		switch(led.getColor()) {
			case Led::RED: cout << "RED"; break;
			case Led::GREEN: cout << "GREEN"; break;
			case Led::BLUE: cout << "BLUE"; break;
			default: cout << "-"; break;
		}
		cout << endl; 
		cout << "Rate: ";
		if (led.getRate() != FAILED) cout << led.getRate();
		else cout << "-";
		cout << endl;

		// Read command
		cout << "\nWait command...\n";
		getline( *input, str);
		cout << "Command[" << ++count_command << "]: ";// << str.c_str() << endl;
		//if (str == "exit") break;

		size_t pos = str.find(' ');
		map<string, string* (*)(Led&, string&)>::iterator it;

		string *s;
		if ((it = command.find(str.substr(0, pos))) != command.end()) {
			string* (*pf)(Led&, string&);
			pf = it->second;
			s = new string(str.substr(pos+1));
			s = pf(led, *s);
		}
		else {
			s = new string("FAILED\n");
			cout << "BAD COMMAND\n";
		}
		if (fd != -1) write(fd, s->c_str(), s->size());
		else *output << *s;
		delete s;
	}

	cout << "EXIT";
	return 0;
}
