/*******************************************************************
* brief：入口程序
**********************************************************************/
#include "print.h"
//#include "h264_analyze.h"
#include "NALParse.h"
//#include "common.h"

#if 1
int main(int argc, const char *argv[])
{
	//NALU_t naluData;
	char filePath[256] = {"./doc/cuc_ieschool.h264"};
	//char array[32][32] = {0};
	const int TIME = 1;
	printInfo print(USERD, TIME);
	print.print(USERD, "Hello!\n");
	//h264_analyze(argc, (char**)argv);
	//h264_analyze(argc, argv);
	//h264_nal_parse(filePath); 
	parseNalH264(filePath);
	//GetFrameType(&naluData);
	printTest();
	return 0;
}
#endif
