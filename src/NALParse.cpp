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
FILE *h264_fp = NULL;

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

/*********************************************************************************
* Brif: 将指定类型的数据存储到文件中
* Return:
**********************************************************************************/
int writeDataToFile(NALU_t *nalu, int type){
	int ret = 0;
	unsigned char sCode1[4] = {0x00, 0x00, 0x01, 0x00}, sCode2[4] = {0x00, 0x00, 0x00, 0x01};
	if((ret = access("./doc/H264_file",0)) != 0){
		if(NULL != h264_fp){
			printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
			fclose(h264_fp);
			sleep(3);
		}
		printf("[PH264][Error][%s]:info=%s line:%d\n", __func__, strerror(errno), __LINE__);
		h264_fp = NULL;
		sleep(3);
		return -1;
	}
	if(NULL == h264_fp){
		h264_fp = fopen("./doc/H264_file.h264","wb");
	}
	/*将指定数据存储到文件中*/
	if(type == (nalu->Frametype)){
		return 0;
	}
	if(NULL !=h264_fp){
		if(3==nalu->startcodeprefix_len){
			memmove((nalu->buf+3), (nalu->buf), (nalu->len));
			memcpy((nalu->buf), sCode1, 3);
		}else if(4==nalu->startcodeprefix_len){
			memmove((nalu->buf+4), (nalu->buf), (nalu->len));
			memcpy((nalu->buf), sCode2, 4);
		}
		//printf("[PH264][Debug][%s]:Type=%d-%d %02X-%02X-%02X line:%d\n", __func__, (nalu->Frametype) , (type), (nalu->buf[0]), (nalu->buf[1]), (nalu->buf[2]), __LINE__);
		fwrite((nalu->buf), (nalu->len+(nalu->startcodeprefix_len)),1,h264_fp);
		fflush(h264_fp);
	}

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
			//sleep(1);
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
			writeDataToFile(nalu, FRAME_B);
			getResolution(nalu);
			printf("[PH264][Debug][%s]:nal_count=%d sLen=%d Type=%d line:%d\n", __func__, nal_count, (nalu->startcodeprefix_len), (nalu->Frametype), __LINE__);
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

#if 1 /*get H264 resolution*/
#include <stdio.h>  
#include <stdint.h>  
#include <string.h>  
#include <math.h>  
  
typedef  unsigned int UINT;  
typedef  unsigned char BYTE;  
typedef  unsigned long DWORD;  
  
UINT Ue(BYTE *pBuff, UINT nLen, UINT &nStartBit)  
{  
    //计算0bit的个数  
    UINT nZeroNum = 0;  
    while (nStartBit < nLen * 8)  
    {  
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余  
        {  
            break;  
        }  
        nZeroNum++;  
        nStartBit++;  
    }  
    nStartBit ++;  
  
  
    //计算结果  
    DWORD dwRet = 0;  
    for (UINT i=0; i<nZeroNum; i++)  
    {  
        dwRet <<= 1;  
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))  
        {  
            dwRet += 1;  
        }  
        nStartBit++;  
    }  
    return (1 << nZeroNum) - 1 + dwRet;  
}  
  
  
int Se(BYTE *pBuff, UINT nLen, UINT &nStartBit)  
{  
    int UeVal=Ue(pBuff,nLen,nStartBit);  
    double k=UeVal;  
    int nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00  
    if (UeVal % 2==0)  
        nValue=-nValue;  
    return nValue;  
}  
  
  
DWORD u(UINT BitCount,BYTE * buf,UINT &nStartBit)  
{  
    DWORD dwRet = 0;  
    for (UINT i=0; i<BitCount; i++)  
    {  
        dwRet <<= 1;  
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))  
        {  
            dwRet += 1;  
        }  
        nStartBit++;  
    }  
    return dwRet;  
}  
  
/** 
 * H264的NAL起始码防竞争机制 
 * 
 * @param buf SPS数据内容 
 * 
 * @无返回值 
 */  
void de_emulation_prevention(BYTE* buf,unsigned int* buf_size)  
{  
    int i=0,j=0;  
    BYTE* tmp_ptr=NULL;  
    unsigned int tmp_buf_size=0;  
    int val=0;  
  
    tmp_ptr=buf;  
    tmp_buf_size=*buf_size;  
    for(i=0;i<(tmp_buf_size-2);i++)  
    {  
        //check for 0x000003  
        val=(tmp_ptr[i]^0x00) +(tmp_ptr[i+1]^0x00)+(tmp_ptr[i+2]^0x03);  
        if(val==0)  
        {  
            //kick out 0x03  
            for(j=i+2;j<tmp_buf_size-1;j++)  
                tmp_ptr[j]=tmp_ptr[j+1];  
  
            //and so we should devrease bufsize  
            (*buf_size)--;  
        }  
    }  
}  
  
/** 
 * 解码SPS,获取视频图像宽、高和帧率信息 
 * 
 * @param buf SPS数据内容 
 * @param nLen SPS数据的长度 
 * @param width 图像宽度 
 * @param height 图像高度 
 
 * @成功则返回true , 失败则返回false 
 */  
bool h264_decode_sps(unsigned      char * buf,unsigned int nLen,int *width,int *height,int *fps)  
{  
    UINT StartBit=0;
    de_emulation_prevention(buf,&nLen);  
  
    int forbidden_zero_bit=u(1,buf,StartBit);  
    int nal_ref_idc=u(2,buf,StartBit);  
    int nal_unit_type=u(5,buf,StartBit);  
    if(nal_unit_type==7)  
    {  
        int profile_idc=u(8,buf,StartBit);  
        int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;  
        int constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;  
        int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;  
        int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;  
        int reserved_zero_4bits=u(4,buf,StartBit);  
        int level_idc=u(8,buf,StartBit);  
  
        int seq_parameter_set_id=Ue(buf,nLen,StartBit);  
  
        if( profile_idc == 100 || profile_idc == 110 ||  
            profile_idc == 122 || profile_idc == 144 )  
        {  
            int chroma_format_idc=Ue(buf,nLen,StartBit);  
            if( chroma_format_idc == 3 )  
                int residual_colour_transform_flag=u(1,buf,StartBit);  
            int bit_depth_luma_minus8=Ue(buf,nLen,StartBit);  
            int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit);  
            int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);  
            int seq_scaling_matrix_present_flag=u(1,buf,StartBit);  
  
            int seq_scaling_list_present_flag[8];  
            if( seq_scaling_matrix_present_flag )  
            {  
                for( int i = 0; i < 8; i++ ) {  
                    seq_scaling_list_present_flag[i]=u(1,buf,StartBit);  
                }  
            }  
        }  
        int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);  
        int pic_order_cnt_type=Ue(buf,nLen,StartBit);  
        if( pic_order_cnt_type == 0 )  
            int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);  
        else if( pic_order_cnt_type == 1 )  
        {  
            int delta_pic_order_always_zero_flag=u(1,buf,StartBit);  
            int offset_for_non_ref_pic=Se(buf,nLen,StartBit);  
            int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);  
            int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);  
  
            int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];  
            for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )  
                offset_for_ref_frame[i]=Se(buf,nLen,StartBit);  
            delete [] offset_for_ref_frame;  
        }  
        int num_ref_frames=Ue(buf,nLen,StartBit);  
        int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);  
        int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);  
        int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);  
  
        (*width)=(pic_width_in_mbs_minus1+1)*16;  
        (*height)=(pic_height_in_map_units_minus1+1)*16;  
  
        int frame_mbs_only_flag=u(1,buf,StartBit);  
        if(!frame_mbs_only_flag)  
            int mb_adaptive_frame_field_flag=u(1,buf,StartBit);  
  
        int direct_8x8_inference_flag=u(1,buf,StartBit);  
        int frame_cropping_flag=u(1,buf,StartBit);  
        if(frame_cropping_flag)  
        {  
            int frame_crop_left_offset=Ue(buf,nLen,StartBit);  
            int frame_crop_right_offset=Ue(buf,nLen,StartBit);  
            int frame_crop_top_offset=Ue(buf,nLen,StartBit);  
            int frame_crop_bottom_offset=Ue(buf,nLen,StartBit);  
        }  
        int vui_parameter_present_flag=u(1,buf,StartBit);  
        if(vui_parameter_present_flag)  
        {  
            int aspect_ratio_info_present_flag=u(1,buf,StartBit);  
            if(aspect_ratio_info_present_flag)  
            {  
                int aspect_ratio_idc=u(8,buf,StartBit);  
                if(aspect_ratio_idc==255)  
                {  
                    int sar_width=u(16,buf,StartBit);  
                    int sar_height=u(16,buf,StartBit);  
                }  
            }  
            int overscan_info_present_flag=u(1,buf,StartBit);  
            if(overscan_info_present_flag)  
                int overscan_appropriate_flagu=u(1,buf,StartBit);  
            int video_signal_type_present_flag=u(1,buf,StartBit);  
            if(video_signal_type_present_flag)  
            {  
                int video_format=u(3,buf,StartBit);  
                int video_full_range_flag=u(1,buf,StartBit);  
                int colour_description_present_flag=u(1,buf,StartBit);  
                if(colour_description_present_flag)  
                {  
                    int colour_primaries=u(8,buf,StartBit);  
                    int transfer_characteristics=u(8,buf,StartBit);  
                    int matrix_coefficients=u(8,buf,StartBit);  
                }  
            }  
            int chroma_loc_info_present_flag=u(1,buf,StartBit);  
            if(chroma_loc_info_present_flag)  
            {  
                int chroma_sample_loc_type_top_field=Ue(buf,nLen,StartBit);  
                int chroma_sample_loc_type_bottom_field=Ue(buf,nLen,StartBit);  
            }  
            int timing_info_present_flag=u(1,buf,StartBit);  
  
            if(timing_info_present_flag)  
            {  
                int num_units_in_tick=u(32,buf,StartBit);  
                int time_scale=u(32,buf,StartBit);  
                (*fps)=time_scale/num_units_in_tick;  
                int fixed_frame_rate_flag=u(1,buf,StartBit);  
                if(fixed_frame_rate_flag)  
                {  
                    (*fps)=(*fps)/2;  
                }  
            }  
        }  
        return true;  
    }  
    else  
        return false;  
} 



/********************************************************************
* Breief:获取分辨率 码率
**********************************************************************/
int getResolution(NALU_t *pnalu){
	int width, height, fRate, ret = 1024;
	if(NAL_SPS != (pnalu->Frametype)){
		return 0;
	}
	h264_decode_sps((unsigned char*)(pnalu->buf), (pnalu->len), &width, &height, &fRate);
	printf("[%s][%d]:resoulution=%dx%d@%d\n", __func__, __LINE__, width, height, fRate);
	
	return 0;
}
#endif
