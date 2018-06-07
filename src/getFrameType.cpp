/*haode*/
//Mybs.h
//#pragma once
//#include "Mybs.h"
//#include "Information.h"
//Mybs.cpp

#include  <stdio.h>
#include <stdlib.h>
#include  <string.h>
#include "common.h"


//读取字节结构体
typedef struct Tag_bs_t
{
	unsigned char *p_start;	  //缓冲区首地址(这个开始是最低地址)
	unsigned char *p;	      //缓冲区当前的读写指针 当前字节的地址，这个会不断的++，每次++，进入一个新的字节
	unsigned char *p_end;	  //缓冲区尾地址		//typedef unsigned char   uint8_t;
	int i_left;			      // p所指字节当前还有多少 “位” 可读写 count number of available(可用的)位 
}bs_t;		

#if 0
/*H264一帧数据的结构体*/
typedef struct Tag_NALU_t
{
#if 0
	unsigned char forbidden_bit;           //! Should always be FALSE
	unsigned char nal_reference_idc;       //! NALU_PRIORITY_xxxx
	unsigned char nal_unit_type;           //! NALU_TYPE_xxxx  
	unsigned int  startcodeprefix_len;      //! 前缀字节数
	unsigned int  len;                     //! 包含nal 头的nal 长度，从第一个00000001到下一个000000001的长度
	unsigned int  max_size;                //! 最多一个nal 的长度
	unsigned char * buf;                   //! 包含nal 头的nal 数据
	unsigned char Frametype;               //! 帧类型
	unsigned int  lost_packets;            //! 预留
#endif
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx    
  char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
  int data_offset;
  unsigned char Frametype;               //! 帧类型
} NALU_t;
#endif

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

void bs_init( bs_t *s, void *p_data, int i_data )
{
	s->p_start = (unsigned char *)p_data;		//用传入的p_data首地址初始化p_start，只记下有效数据的首地址
	s->p       = (unsigned char *)p_data;		//字节首地址，一开始用p_data初始化，每读完一个整字节，就移动到下一字节首地址
	s->p_end   = s->p + i_data;	                //尾地址，最后一个字节的首地址?
	s->i_left  = 8;				                //还没有开始读写，当前字节剩余未读取的位是8
}


int bs_read( bs_t *s, int i_count )
{
	 static unsigned int i_mask[33] ={0x00,
                                  0x01,      0x03,      0x07,      0x0f,
                                  0x1f,      0x3f,      0x7f,      0xff,
                                  0x1ff,     0x3ff,     0x7ff,     0xfff,
                                  0x1fff,    0x3fff,    0x7fff,    0xffff,
                                  0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
                                  0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
                                  0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
                                  0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};
	/*
			  数组中的元素用二进制表示如下：

			  假设：初始为0，已写入为+，已读取为-
			  
			  字节:		1		2		3		4
				   00000000 00000000 00000000 00000000		下标

			  0x00:							  00000000		x[0]

			  0x01:							  00000001		x[1]
			  0x03:							  00000011		x[2]
			  0x07:							  00000111		x[3]
			  0x0f:							  00001111		x[4]

			  0x1f:							  00011111		x[5]
			  0x3f:							  00111111		x[6]
			  0x7f:							  01111111		x[7]
			  0xff:							  11111111		x[8]	1字节

			 0x1ff:						 0001 11111111		x[9]
			 0x3ff:						 0011 11111111		x[10]	i_mask[s->i_left]
			 0x7ff:						 0111 11111111		x[11]
			 0xfff:						 1111 11111111		x[12]	1.5字节

			0x1fff:					 00011111 11111111		x[13]
			0x3fff:					 00111111 11111111		x[14]
			0x7fff:					 01111111 11111111		x[15]
			0xffff:					 11111111 11111111		x[16]	2字节

		   0x1ffff:				0001 11111111 11111111		x[17]
		   0x3ffff:				0011 11111111 11111111		x[18]
		   0x7ffff:				0111 11111111 11111111		x[19]
		   0xfffff:				1111 11111111 11111111		x[20]	2.5字节

		  0x1fffff:			00011111 11111111 11111111		x[21]
		  0x3fffff:			00111111 11111111 11111111		x[22]
		  0x7fffff:			01111111 11111111 11111111		x[23]
		  0xffffff:			11111111 11111111 11111111		x[24]	3字节

		 0x1ffffff:	   0001 11111111 11111111 11111111		x[25]
		 0x3ffffff:	   0011 11111111 11111111 11111111		x[26]
		 0x7ffffff:    0111 11111111 11111111 11111111		x[27]
		 0xfffffff:    1111 11111111 11111111 11111111		x[28]	3.5字节

		0x1fffffff:00011111 11111111 11111111 11111111		x[29]
		0x3fffffff:00111111 11111111 11111111 11111111		x[30]
		0x7fffffff:01111111 11111111 11111111 11111111		x[31]
		0xffffffff:11111111 11111111 11111111 11111111		x[32]	4字节

	 */
    int      i_shr;			    //
    int i_result = 0;	        //用来存放读取到的的结果 typedef unsigned   uint32_t;

    while( i_count > 0 )	    //要读取的比特数
    {
        if( s->p >= s->p_end )	//字节流的当前位置>=流结尾，即代表此比特流s已经读完了。
        {						//
            break;
        }

        if( ( i_shr = s->i_left - i_count ) >= 0 )	//当前字节剩余的未读位数，比要读取的位数多，或者相等
        {											//i_left当前字节剩余的未读位数，本次要读i_count比特，i_shr=i_left-i_count的结果如果>=0，说明要读取的都在当前字节内
													//i_shr>=0，说明要读取的比特都处于当前字节内
			//这个阶段，一次性就读完了，然后返回i_result(退出了函数)
            /* more in the buffer than requested */
            i_result |= ( *s->p >> i_shr )&i_mask[i_count];//“|=”:按位或赋值，A |= B 即 A = A|B
									//|=应该在最后执行，把结果放在i_result(按位与优先级高于复合操作符|=)
									//i_mask[i_count]最右侧各位都是1,与括号中的按位与，可以把括号中的结果复制过来
									//!=,左边的i_result在这儿全是0，右侧与它按位或，还是复制结果过来了，好象好几步都多余
			/*读取后，更新结构体里的字段值*/
            s->i_left -= i_count; //即i_left = i_left - i_count，当前字节剩余的未读位数，原来的减去这次读取的
            if( s->i_left == 0 )	//如果当前字节剩余的未读位数正好是0，说明当前字节读完了，就要开始下一个字节
            {
                s->p++;				//移动指针，所以p好象是以字节为步长移动指针的
                s->i_left = 8;		//新开始的这个字节来说，当前字节剩余的未读位数，就是8比特了
            }
            return( i_result );		//可能的返回值之一为：00000000 00000000 00000000 00000001 (4字节长)
        }
        else	/* i_shr < 0 ,跨字节的情况*/
        {
			//这个阶段，是while的一次循环，可能还会进入下一次循环，第一次和最后一次都可能读取的非整字节，比如第一次读了3比特，中间读取了2字节(即2x8比特)，最后一次读取了1比特，然后退出while循环
			//当前字节剩余的未读位数，比要读取的位数少，比如当前字节有3位未读过，而本次要读7位
			//???对当前字节来说，要读的比特，都在最右边，所以不再移位了(移位的目的是把要读的比特放在当前字节最右)
            /* less(较少的) in the buffer than requested */
			i_result |= (*s->p&i_mask[s->i_left]) << -i_shr;	//"-i_shr"相当于取了绝对值
									//|= 和 << 都是位操作符，优先级相同，所以从左往右顺序执行
									//举例:int|char ，其中int是4字节，char是1字节，sizeof(int|char)是4字节
									//i_left最大是8，最小是0，取值范围是[0,8]
			i_count  -= s->i_left;	//待读取的比特数，等于原i_count减去i_left，i_left是当前字节未读过的比特数，而此else阶段，i_left代表的当前字节未读的比特全被读过了，所以减它
			s->p++;	//定位到下一个新的字节
			s->i_left = 8;	//对一个新字节来说，未读过的位数当然是8，即本字节所有位都没读取过
        }
    }

    return( i_result );//可能的返回值之一为：00000000 00000000 00000000 00000001 (4字节长)
}

int bs_read1( bs_t *s )
{

	if( s->p < s->p_end )	
	{
		unsigned int i_result;

		s->i_left--;                           //当前字节未读取的位数少了1位
		i_result = ( *s->p >> s->i_left )&0x01;//把要读的比特移到当前字节最右，然后与0x01:00000001进行逻辑与操作，因为要读的只是一个比特，这个比特不是0就是1，与0000 0001按位与就可以得知此情况
		if( s->i_left == 0 )                   //如果当前字节剩余未读位数是0，即是说当前字节全读过了
		{
			s->p++;			                   //指针s->p 移到下一字节
			s->i_left = 8;                     //新字节中，未读位数当然是8位
		}
		return i_result;                       //unsigned int
	}

	return 0;                                  //返回0应该是没有读到东西
}

int bs_read_ue( bs_t *s )
{
	int i = 0;

	while( bs_read1( s ) == 0 && s->p < s->p_end && i < 32 )	//条件为：读到的当前比特=0，指针未越界，最多只能读32比特
	{
		i++;
	}
	return( ( 1 << i) - 1 + bs_read( s, i ) );	
}

/*
函数名称：
函数功能：初始化结构体
参    数：
返 回 值：无返回值,void类型
思    路：
资    料：
		  
*/
//void bs_init( bs_t *s, void *p_data, int i_data );

/*
该函数的作用是：从s中读出i_count位，并将其做为uint32_t类型返回
思路:
	若i_count>0且s流并未结束，则开始或继续读取码流；
	若s当前字节中剩余位数大于等于要读取的位数i_count，则直接读取；
	若s当前字节中剩余位数小于要读取的位数i_count，则读取剩余位，进入s下一字节继续读取。
补充:
	写入s时，i_left表示s当前字节还没被写入的位，若一个新的字节，则i_left=8；
	读取s时，i_left表示s当前字节还没被读取的位，若一个新的字节，则i_left＝8。
	注意两者的区别和联系。

	00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 0000000
	-------- -----000 00000000 ...
	写入s时：i_left = 3
	读取s时：i_left = 5

我思：
	字节流提前放在了结构体bs_s的对象bs_t里了，可能字节流不会一次性读取/分析完，而是根据需要，每次都读取几比特
	bs_s里，有专门的字段用来记录历史读取的结果,每次读取，都会在上次的读取位置上进行
	比如，100字节的流，经过若干次读取，当前位置处于中间一个字节处，前3个比特已经读取过了，此次要读取2比特

	00001001
	000 01 001 (已读过的 本次要读的 以后要读的 )
	i_count = 2	(计划去读2比特)
	i_left  = 5	(还有5比特未读，在本字节中)
	i_shr = s->i_left - i_count = 5 - 2 = 3
	*s->p >> i_shr，就把本次要读的比特移到了字节最右边(未读，但本次不需要的给移到了字节外，抛掉了)
	00000001
	i_mask[i_count] 即i_mask[2] 即0x03:00000011
	( *s->p >> i_shr )&i_mask[i_count]; 即00000001 & 00000011 也就是00000001 按位与 00000011
	结果是：00000001
	i_result |= ( *s->p >> i_shr )&i_mask[i_count];即i_result |=00000001 也就是 i_result =i_result | 00000001 = 00000000 00000000 00000000 00000000 | 00000001 =00000000 00000000 00000000 00000001
	i_result =
	return( i_result ); 返回的i_result是4字节长度的，是unsigned类型 sizeof(unsigned)=4
*/
int bs_read( bs_t *s, int i_count );

/*
函数名称：
函数功能：从s中读出1位，并将其做为uint32_t类型返回。
函数参数：
返 回 值：
思    路：若s流并未结束，则读取一位
资    料：
		毕厚杰：第145页，u(n)/u(v)，读进连续的若干比特，并将它们解释为“无符号整数”
		return i_result;	//unsigned int
*/
int bs_read1( bs_t *s );

/*
函数名称：
函数功能：从s中解码并读出一个语法元素值
参    数：
返 回 值：
思    路：
		从s的当前位读取并计数，直至读取到1为止；
		while( bs_read1( s ) == 0 && s->p < s->p_end && i < 32 )这个循环用i记录了s当前位置到1为止的0的个数，并丢弃读到的第一个1；
		返回2^i-1+bs_read(s,i)。
		例：当s字节中存放的是0001010时，1前有3个0，所以i＝3；
		返回的是：2^i-1+bs_read(s,i)即：8－1＋010＝9
资    料：
		毕厚杰：第145页，ue(v)；无符号指数Golomb熵编码
		x264中bs.h文件部分函数解读 http://wmnmtm.blog.163.com/blog/static/382457142011724101824726/
		无符号整数指数哥伦布码编码 http://wmnmtm.blog.163.com/blog/static/38245714201172623027946/
*/
int bs_read_ue( bs_t *s );

/************************/
int GetFrameType(NALU_t * nal)
{	
	bs_t s;
	int frame_type = 0; 
	unsigned char * OneFrameBuf_H264 = NULL;
	if ((OneFrameBuf_H264 = (unsigned char *)calloc(nal->len + 4,sizeof(unsigned char))) == NULL){
		printf("[PH264][Error][%s]:Error malloc OneFrameBuf_H264\n", __func__);
		return getchar();
	}
	
	OneFrameBuf_H264[0] = 0x00;
	OneFrameBuf_H264[1] = 0x00;
	if (nal->startcodeprefix_len == 3){
		OneFrameBuf_H264[2] = 0x01;
		memcpy(OneFrameBuf_H264 + 3,nal->buf,nal->len);
	}
	else if (nal->startcodeprefix_len == 4){
		OneFrameBuf_H264[2] = 0x00;
		OneFrameBuf_H264[3] = 0x01;
		memcpy(OneFrameBuf_H264 + 4,nal->buf,nal->len);
	}
	else{
		printf("[PH264][Error][%s]:wrong startcodeprefix_len=%d\n", __func__, (nal->startcodeprefix_len));
		return -1;
	}
	
	bs_init( &s,OneFrameBuf_H264 + nal->startcodeprefix_len + 1  ,nal->len - 1 );
	if (nal->nal_unit_type == NAL_SLICE || nal->nal_unit_type ==  NAL_SLICE_IDR ){
		bs_read_ue( &s ); /* i_first_mb */
		frame_type =  bs_read_ue( &s ); /* picture type */
		switch(frame_type)
		{
		case 0: case 5: /* P */
			nal->Frametype = FRAME_P;
			break;
		case 1: case 6: /* B */
			nal->Frametype = FRAME_B;
			break;
		case 3: case 8: /* SP */
			nal->Frametype = FRAME_P;
			break;
		case 2: case 7: /* I */
			nal->Frametype = FRAME_I;
			break;
		case 4: case 9: /* SI */
			nal->Frametype = FRAME_I;
			break;
		}
	}
	else if (nal->nal_unit_type == NAL_SEI){
		nal->Frametype = NAL_SEI;
	}
	else if(nal->nal_unit_type == NAL_SPS){
		nal->Frametype = NAL_SPS;
	}
	else if(nal->nal_unit_type == NAL_PPS){
		nal->Frametype = NAL_PPS;
	}
	if (NULL != OneFrameBuf_H264){
		free(OneFrameBuf_H264);
		OneFrameBuf_H264 = NULL;
	}
	return 1;
}

int printTest(){
	printf("[%s][%s]:line:%d\n", __FILE__, __func__, __LINE__);
	return 0;
}

#if 0
int main(int argc, const char *argv[])
{
	NALU_t naltData;
	//int ret = GetFrameType(&naltData);
	//printf("[%s]:NALU_t=%d NALU_tA=%d line:%d\n", __func__, sizeof(NALU_t), sizeof(NALU_tA), __LINE__);
	return 0;
}
#endif
