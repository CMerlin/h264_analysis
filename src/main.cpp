/*******************************************************************
* brief：入口程序
**********************************************************************/
#include "print.h"
#include "h264_analyze.h"

#if 0
int main(int argc, const char *argv[])
{
	//char array[32][32] = {0};
	const int TIME = 1;
	printInfo print(USERD, TIME);
	print.print(USERD, "Hello!\n");
	//h264_analyze(argc, (char**)argv);
	h264_analyze(argc, argv);
	 
	return 0;
}
#endif
