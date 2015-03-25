#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

/*-------------------------------------------------------------------------------------------------------------*/

//Os Selection
#define MY_WIN32
//#define MY_ANSI_C

#if defined(MY_WIN32)&&defined(MY_ANSI_C)
	#error "Don't define multi-os"
#endif

#if !defined(MY_WIN32)&&!defined(MY_ANSI_C)
	#error "Please select one os"
#endif



/*-------------------------------------------------------------------------------------------------------------*/
//unicode 
#define MY_UNICODE




#define MY_TEST_CODES



#endif
