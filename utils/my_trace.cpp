#include <stdafx.h>
#include <iomanip>
#include <windows.h>
#include "my_trace.h"
#include "my_network_utils.h"
#include "my_misc_utils.h"

using namespace std;

void LogFile::NoteDate() 
{
	if (!updated) {
		SYSTEMTIME system;
		bzero(&system, sizeof(system));
	
		GetSystemTime(&system);
		updated = true;

		char fillc = f.fill('0');
		f<<"---------------------- "<<setw(4)<<system.wYear
		 <<"/"<<setw(2)<<system.wMonth
		 <<"/"<<setw(2)<<system.wDay
		 <<" "<<setw(2)<<system.wHour
		 <<":"<<setw(2)<<system.wMinute
		 <<":"<<setw(2)<<system.wMilliseconds/1000
		 <<" ---------------------"<<setfill(fillc)<<std::endl;;
	}
}

