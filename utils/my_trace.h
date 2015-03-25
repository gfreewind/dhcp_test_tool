#pragma once 

#include <stdio.h>
#include <fstream>
#include <string>

class Trace {
public:
    Trace() { noisy = 0; f = stdout;}
    Trace(FILE* ff) {noisy = 0; f = ff;}
    void print(char* s) {if (noisy) fprintf(f, "%s, %s %d\n", s, __FILE__, __LINE__);}
    void on() {noisy = 1;}
    void off() {noisy = 0;}
private:
    int noisy;
    FILE* f;
};

class LogFile{
public:
	LogFile(const char* filename):f(filename, std::ios_base::app), updated(false){}
	~LogFile()
	{
	    if (updated) {	        
            f<<"----------------------------- END ------------------------------"<<std::endl;
            f.close();
	    }
	}
	std::ostream& operator<< (DWORD value)
	{
        NoteDate();
		f << value;
	    return f;
	}

	std::ostream& operator<< (const char* str)
	{
	    NoteDate();
		f << str;
	    return f;
	}	
private:
    void NoteDate();
	std::fstream f;
	bool updated;
};

class ExitFlag{
public:
    ExitFlag(volatile bool& v):exit(v) 
    {
        exit = false;
    }
    ~ExitFlag() 
    {
        exit = true;
    }
private:
    volatile bool& exit;
};
