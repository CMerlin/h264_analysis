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
//#include "SpecialVH264.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "NALParse.h"
//#include "common.h"
#include "getFrameType.h"

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

FILE *bits = NULL;                //!< the bit stream file
//extern int GetFrameType(NALU_t * nal);

static int FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1
	else return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1
	else return 1;
}

//static bool flag = true;
static int info2=0, info3=0;
static int count_nal = 0;

//-------------------
//CSpecialVH264Dlg *dlg;


/***************************************************************************
* Brief:检查数据的开始码是3个字节（00 00 01）还是4个字节（00 00 00 01）
* Inparam:buf-数据区 len-数据区的长度 pPos-开始码开始的地方
* Return:0-没有找到开始码 3-开码是3个字节 4-开始码是4个字节
****************************************************************************/
int getStartCodeLen(unsigned char *buf, int len, int *pPos){
	int pos = 0;
	while(2 < len){
		*pPos = pos; /*开始码开始的位置*/
		if((0x00==buf[pos]) && (0x00==buf[pos+1]) && (0x00==buf[pos+2]) && (0x01==buf[pos+3])){
			return 4;
			break;
		}else if((0x00==buf[pos]) && (0x00==buf[pos+1]) && (0x01==buf[pos+2])){
			return 3;
			break;
		}
		pos++;
		len--;
	}
	return 0;
}


//这个函数输入为一个NAL结构体，主要功能为得到一个完整的NALU并保存在NALU_t的buf中，获取他的长度，填充F,IDC,TYPE位。
//并且返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
int GetAnnexbNALU (NALU_t *nalu)
{
  int pos = 0;
  int StartCodeFound, rewind;
  unsigned char *Buf;

  if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL) 
	  printf ("GetAnnexbNALU: Could not allocate Buf memory\n");
#if 1
  nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节
   if (3 != fread (Buf, 1, 3, bits)){
   	free(Buf); //从码流中读3个字节
	return 0;
   }
   info2 = FindStartCode2 (Buf);//判断是否为0x000001 
   if(info2 != 1){
	//如果不是，再读一个字节
    if(1 != fread(Buf+3, 1, 1, bits))//读一个字节
		{
		 free(Buf);
		 return 0;
		}
    info3 = FindStartCode3 (Buf);//判断是否为0x00000001
    if (info3 != 1)//如果不是，返回-1
		{ 
		 free(Buf);
		 return -1;
		}
    else 
		{
		//如果是0x00000001,得到开始前缀为4个字节
		 pos = 4;
		 nalu->startcodeprefix_len = 4;
		}
   }
   
   else
	   {
	   //如果是0x000001,得到开始前缀为3个字节
		nalu->startcodeprefix_len = 3;
		pos = 3;
	   }
 #endif
 #if 0
 	printf("[PH264][DEBUG][%s]:begin line:%d\n", __func__,  __LINE__);
 	int BufLen = 1024, ret = 0;
 	nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节
 	if (3 != fread (Buf, BufLen, 3, bits)){
		free(Buf); //从码流中读3个字节
		return 0;
	}
	//BufLen = nalu->max_size - 21;
 	if(0 >= (ret = getStartCodeLen(Buf, BufLen, &pos))){
		 printf("[PH264][ERROR][%s]:can not find start code! pos=%d line:%d\n", __func__, pos, __LINE__);
	}
	//fsetpos(bits, (fpos_t*)&pos);
#endif
	//查找下一个开始字符的标志位
   StartCodeFound = 0;
   info2 = 0;
   info3 = 0;

  printf("[PH264][Debug][%s]:StartCodeFound=%d line:%d\n", __func__, StartCodeFound, __LINE__);
  while (!StartCodeFound)
  {
    if (feof (bits))//判断是否到了文件尾
    {
      printf("[PH264][Debug][%s]:line:%d\n", __func__, __LINE__);
      nalu->len = (pos-1)-nalu->startcodeprefix_len;
      memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);     
      nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	  nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
      free(Buf);
      return pos-1;
    }
    Buf[pos++] = fgetc (bits);//读一个字节到BUF中
    info3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
    if(info3 != 1)
      info2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
    StartCodeFound = (info2 == 1 || info3 == 1);
  }
  

 
  // Here, we have found another start code (and read length of startcode bytes more than we should
  // have.  Hence, go back in the file
  rewind = (info3 == 1)? -4 : -3;

  if (0 != fseek (bits, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾
  {
    free(Buf);
	printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
  }

  // Here the Start code, the complete NALU, and the next start code is in the Buf.  
  // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
  // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

  nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
  memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
  nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
  nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
  //GetFrameType((unsigned char*)nalu);
  //GetFrameType(nalu);
  count_nal++;
  free(Buf);
  printf("[PH264][Debug][%s]:nal_unit_type=%d FType=%d count_nal=%d line:%d\n", __func__, nalu->nal_unit_type, (int)(nalu->Frametype), count_nal, __LINE__);
  return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}

/***********************************************************************************
* Brief:解析H264裸数据，把数据存储到NALU_t结构体中（此版本在历史实现）
* Inparam:fileurl-h264文件存储的位置
* Return:
***********************************************************************************/
int h264_nal_parse(char *fileurl)
{
	bits=fopen(fileurl, "r+b");
	if ( bits== NULL){
		//AfxMessageBox(_T("Error open file"));
		return -1;
	}
	
	NALU_t *n;
	int buffersize=800000;

	if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL){
		//AfxMessageBox(_T("Error AllocNALU: n"));
		return -1;
	}
	n->max_size=buffersize;
	if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL){
		free (n);
		//AfxMessageBox (_T("Error AllocNALU: n->buf"));
		return -1;
	}

	printf("[PH264][Debug][%s]:fileurl=%s line:%d\n", __func__, fileurl, __LINE__);
	//------------------
	int data_offset=0;
	//赋值-----------------
	//dlg=(CSpecialVH264Dlg *)lparam;
	//----------
	int nal_num=0;
	//----------
	while(!feof(bits)) {
		int data_lenth;
		data_lenth=GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		n->data_offset=data_offset;
		data_offset=data_offset+data_lenth;
		//输出NALU长度和TYPE
		int nal_reference_idc=n->nal_reference_idc>>5;
		//dlg->AppendNLInfo(nal_reference_idc,n->nal_unit_type,n->len,n->len+n->startcodeprefix_len,n->data_offset);

		//判断是否选择了“只分析5000条”，如果选择了就不再分析了
		//if(dlg->m_vh264nallistmaxnum.GetCheck()==1&&nal_num>5000){
		//	break;
		//}
#if 0
		if(100<=nal_num){
			printf("[PH264][Debug][%s]:parse data over nal_num=%d line:%d\n", __func__, nal_num, __LINE__);
			break;
		}
#endif
		//printf("[PH264][Debug][%s]:parse data over nal_num=%d line:%d\n", __func__, nal_num, __LINE__);
		nal_num++;
	}

	if (n){
		if (n->buf){
			free(n->buf);
			n->buf=NULL;
		}
		free (n);
	}
	return 0;
}

int printCDATA(unsigned char *buf, int len){
	int pos = 0;
	while(0 < len){
		printf("%02X ", buf[pos]);
		len--;
		pos++;
	}
	printf("\n");
	return 0;
}

/********************************************************************************************
* Brief:获取NALU_t数据
* Return:-1-没有找到开始码 -2-没有找到第二个开始码
*********************************************************************************************/
int getNaluData(S_BUFFER *sbuf, NALU_t *nalu){
	int lastPos = 0, curPos = 0; /*上一个出现开始码的位置*/
	int ret = 0, pos = 0, len = sbuf->wPos - sbuf->rPos;
	/*寻找第一个开始码*/
	//printf("rPos=%d ", sbuf->rPos);
	//printCDATA(sbuf->buf+(sbuf->rPos), 8);
	ret = getStartCodeLen(sbuf->buf+(sbuf->rPos), len, &pos);
	if(0>=ret){
		printf("[PH264][Error][%s]:not find start code line:%d\n", __func__, __LINE__);
		sbuf->flag = 1;
		return -1;
	}
	/*寻找第二个开始码*/
	curPos = sbuf->rPos + pos + ret; /*下一个搜索点的位置*/
	lastPos = curPos;
	len = sbuf->wPos - curPos; /*搜索区域的长度*/
	//printf("[PH264][Debug][%s]:len=%d pos=%d curPos=%d-0X%02X startCodeLen=%d line:%d\n", __func__, len, pos, curPos , (*((sbuf->buf+curPos))), ret, __LINE__);
	ret = getStartCodeLen(sbuf->buf+curPos, len, &pos);
	if(0>=ret){
		printf("[PH264][Error][%s]:not find start code line:%d\n", __func__, __LINE__);
		sbuf->flag = 1;
		return -2;
	}
	/*获取nual相关的信息*/
	nalu->startcodeprefix_len = ret; /*开始码的长度*/
	curPos = lastPos + pos; /*下一个搜索点的位置*/
	//printf("[PH264][Debug][%s]:lastPos=%d end=%d-0x%02X pos=%d line:%d\n", __func__, lastPos, curPos, (sbuf->buf[curPos]), pos, __LINE__);
	nalu->len = curPos - lastPos; /*NALU数据的长度，不包含开始码*/
	memcpy((nalu->buf), (sbuf->buf+lastPos), (nalu->len)); /*NALU数据，不包含开始码*/
	sbuf->rPos = curPos; /*移动sbuf的读写指针*/
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	//printf("[PH264][%s]:rPos=%d Data=%02X line:%d\n", __func__, (sbuf->rPos), (sbuf->buf[sbuf->rPos]), __LINE__);
	GetFrameType(nalu);
	
	return 0;
}


/***********************************************************************************
* Brief:解析H264裸数据，把数据存储到NALU_t结构体中（此版本在历史实现）
* Inparam:fileurl-h264文件存储的位置
* Return:
***********************************************************************************/
int parseNalH264(char *filePath){
	FILE *bitfd = NULL;
	NALU_t *nalu = NULL;
	int nal_count=0, ret = 0, pos = 0, count = 0;
	S_BUFFER buffer;

	printf("[PH264][Debug][%s]:filePath=%s line:%d\n", __func__, filePath, __LINE__);
	bitfd=fopen(filePath, "r+b");
	if (NULL == bitfd){
		printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	/*给nulu分配空间*/
	if ((nalu = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL){
		printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	nalu->max_size = 1024*1024;
	if ((nalu->buf = (char*)calloc((nalu->max_size), sizeof (char))) == NULL){
		free(nalu);
		printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	/*buffer初始化*/
	memset(&buffer, 0, sizeof(S_BUFFER));
	buffer.size = 1024*1024;
	if ((buffer.buf = (unsigned char*)calloc((buffer.size), sizeof (char))) == NULL){
		free(buffer.buf);
		printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}

	printf("[PH264][Debug][%s]:rPos=%d line:%d\n", __func__, (buffer.rPos), __LINE__);
	/*读取并分析数据*/
	while(!feof(bitfd)){
		/*移动缓存区的指针*/
		if(buffer.rPos > (buffer.size/2)){
			printf("[PH264][Debug][%s]:moveData rPos=%d len=%d line:%d\n", __func__, (buffer.rPos), (buffer.wPos-buffer.rPos), __LINE__);
			memmove(buffer.buf, (buffer.buf+buffer.rPos), (buffer.wPos-buffer.rPos));
			buffer.wPos = buffer.wPos - buffer.rPos;
			buffer.rPos = 0;
			memset((buffer.buf+buffer.wPos), 0, (buffer.size-buffer.wPos));
			printf("[PH264][Debug][%s]:moveData rPos=%d len=%d line:%d\n", __func__, (buffer.rPos), (buffer.wPos-buffer.rPos), __LINE__);
			sleep(1);
		}
		ret = fread((void*)(buffer.buf+buffer.wPos), 1, 2048, bitfd);
		if(ret<0){
			printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
			break;
		}else if(0==ret){
			printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
			continue;
		}
		
		buffer.flag = 0;
		buffer.wPos += ret;
		count += ret;
		printf("[PH264][Error][%s]:rPos=%d wPos=%d begin parse buffer! line:%d\n", __func__, (buffer.rPos), (buffer.wPos), __LINE__);
		//sleep(2);
		/*解析缓存区的数据*/
		while(1 !=buffer.flag){
			ret = getNaluData(&buffer, nalu);
			if((-1==ret) || (-2==ret)){
				printf("[PH264][Error][%s]:ret=%d line:%d\n", __func__, ret, __LINE__);
				break;
			}
			nal_count++;
			printf("[PH264][Debug][%s]:nal_count=%d Type=%d line:%d\n", __func__, nal_count, (nalu->Frametype), __LINE__);
		}
	}

	/*释放内存*/
	if (NULL != nalu){
		if (NULL != (nalu->buf)){
			free(nalu->buf);
			nalu->buf=NULL;
		}
		free(nalu);
		nalu = NULL;
	}
	return 0;
}
