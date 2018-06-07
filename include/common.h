/**********************************************************************************
* Brief:
* fileName:
**********************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#if 1
/*H264一帧数据的结构体*/
typedef struct _nalu_t_
{
  int startcodeprefix_len;      //! 数据开始码的长度，是3个字节（00 00 01）还是4个字节（00 00 00 01）
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx    
  char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
  int data_offset;              //! 
  unsigned char Frametype;      //! 帧类型
}NALU_t;
#endif

//#include "NALParse.h"
int GetFrameType(NALU_t * nal);
int printTest();

#endif

