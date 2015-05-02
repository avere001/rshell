#include <iostream>

#ifndef ANSI_H
#define ANSI_H

using namespace std;

void moveCursor(int,int);
void setCursor(int,int);
void setColor(int,int);
void resetColor();
void clearScreen();
void showCursor();
void hideCursor();
void setFG(int);
void setFG(double,double,double);
void setFG(int,int,int);
void setBG(int);
void setBG(double,double,double);
void setBG(int,int,int);


const int DARK 		= 8;

const int BLACK 	= 0;
const int RED 		= 1;
const int GREEN 	= 2;
const int YELLOW 	= 3;
const int BLUE 		= 4;
const int MAGENTA	= 5;
const int CYAN 		= 6;
const int WHITE 	= 7;

const string ANSI_CLEAR = "\033[2J";

const string ANSI_ESC 		= "\033[";
const string ANSI_SEPERATOR	= ";";
const string ANSI_END		= "m";

const string FG_BLACK 	= "30";
const string FG_RED 	= "31";
const string FG_GREEN 	= "32";
const string FG_YELLOW 	= "33";
const string FG_BLUE 	= "34";
const string FG_MAGENTA	= "35";
const string FG_CYAN 	= "36";
const string FG_WHITE 	= "37";

const string BG_BLACK 	= "40";
const string BG_RED 	= "41";
const string BG_GREEN 	= "42";
const string BG_YELLOW 	= "43";
const string BG_BLUE 	= "44";
const string BG_MAGENTA	= "45";
const string BG_CYAN 	= "46";
const string BG_WHITE 	= "47";

void setColor(int FG, int BG) {
	cout << ANSI_ESC;
	cout << 22 << 'm';
	
	setFG(FG);
	setBG(BG);
}

void setFG(int FG) {
	cout << ANSI_ESC;
	cout << "38;5;";
	cout << FG << "m";
}

void setFG(double R, double G, double B) {
	
	int r = static_cast<int>(R * 5);
	int g = static_cast<int>(G * 5);
	int b = static_cast<int>(B * 5);
	
	setFG(r,g,b);
}

void setFG (int r, int g, int b) {
	int color = 16 + 36*r + 6*g + b;
	
	setFG(color);
}

void setBG(int FG) {
	cout << ANSI_ESC;
	cout << "48;5;";
	cout << FG << "m";
}

void setBG(double R, double G, double B) {
		
	int r = static_cast<int>(R * 5);
	int g = static_cast<int>(G * 5);
	int b = static_cast<int>(B * 5);
	
	setBG(r,g,b);
}

void setBG (int r, int g, int b) {
	int color = 16 + 36*r + 6*g + b;
	
	setBG(color);
}

void resetColor() {
	showCursor();
	cout << ANSI_ESC;
	cout << 0 << ANSI_SEPERATOR;
	cout << 39 << ANSI_SEPERATOR;
	cout << 10 << ANSI_SEPERATOR;
	cout << 49 << ANSI_END;
}

void hideCursor() {
	cout << ANSI_ESC << "?25l";
}
void showCursor() {
	cout << ANSI_ESC << "?25h";
}

void clearScreen() 
{
	cout << ANSI_CLEAR << endl;
	setCursor(1,1);
}

void moveCursor(int x, int y) 
{	
	if (y > 0) {
		cout << "\033[" << y << "A";
	} else if (y < 0) {
		cout << "\033[" << -y << "B"; 
	}
	
	if (x > 0) {
		cout << "\033[" << x << "C";
	} else if (x < 0) {
		cout << "\033[" << -x << "D";
	}
}

void setCursor(int x, int y) {
	cout << "\033[" 
		 << y << ANSI_SEPERATOR
		 << x << "H"; 
}

#endif
