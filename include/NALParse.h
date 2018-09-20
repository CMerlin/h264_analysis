/*新的*/
/* 
 * H.264 分析器
 * H.264 Analysis
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 * 
 * H.264码流分析工具
 * H.264 Stream Analysis Tools
 *
 */
//#include "stdafx.h"
//#include "SpecialVH264Dlg.h"
#include <errno.h>
#include <unistd.h>
#include "common.h"

typedef struct _resolution{
	int width;
	int height;
	int fRate;
}S_RESOLUTION, *PS_RESOLUTION;

#if 0
/*H264一帧数据的结构体*/
typedef struct
{
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx    
  char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
  int data_offset;
  unsigned char Frametype;      //! 帧类型
} NALU_t;
#endif

typedef struct _buffer{
	unsigned char *buf; /*缓存区*/
	int size; /*缓存区的长度*/
	int rPos; /*读指针，从0开始*/
	int wPos; /*写指针，从0开始*/
	int flag; /*缓存中是否存在目标数据 1-没有目标数据了 */
}S_BUFFER, *PS_BUFFER;

//int GetFrameType(NALU_t * nal);
int h264_nal_parse(char *fileurl);
int parseNalH264(char *filePath);
int getResolution(NALU_t *pnalu);





