/**********************************************************************************
* Brief:
* fileName:
**********************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#if 1
/*nal类型*/
enum nal_unit_type_e
{
	NAL_UNKNOWN     = 0,
	NAL_SLICE       = 1,
	NAL_SLICE_DPA   = 2,
	NAL_SLICE_DPB   = 3,
	NAL_SLICE_DPC   = 4,
	NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
	NAL_SEI         = 6,    /* ref_idc == 0 */
	NAL_SPS         = 7,
	NAL_PPS         = 8
	/* ref_idc == 0 for 6,9,10,11,12 */
};

/*帧类型*/
enum Frametype_e
{
	FRAME_I  = 15,
	FRAME_P  = 16,
	FRAME_B  = 17
};
#endif

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

