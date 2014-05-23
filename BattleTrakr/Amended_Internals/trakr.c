#include "trakr.h"
#include "JAPI.h"

#define LOGGING 0
#define JAPI_GetWchar(a) SVT_Char_To_WChar(a)

#define MOTOR_LEFT_ON 0x0010
#define MOTOR_LEFT_DIRECTION 0x0080
#define MOTOR_RIGHT_ON 0x0001
#define MOTOR_RIGHT_DIRECTION 0x0008

#define sizeofBITMAPFILEHEADER 14
typedef struct tagBITMAPFILEHEADER
{
   unsigned short  bfType; // type of file, must be BM.
   unsigned int  bfSize; // the size of the file.
   unsigned short  bfReserved1; //reserved
   unsigned short  bfReserved2; //reserved
   unsigned int  bfOffBits; //the bit data's offset.
} BITMAPFILEHEADER;
#define sizeofBITMAPINFOHEADER 40
typedef struct tagBITMAPINFOHEADER{
   unsigned int biSize; //the size of this struct
   long   biWidth; //width of pixels.
   long   biHeight; //height of pixels
   unsigned short biPlanes; //must be 1
   unsigned short biBitCount; //the bits per pixel, it must be 1(2-colors)
   unsigned int biCompression; //must be 0(uncompressed);
   unsigned int biSizeImage; //the bitmap's size.
   long   biXPelsPerMeter; //horizontal-pixel per meter
   long   biYPelsPerMeter; //vertical-pixel per meter
   unsigned int biClrUsed;//the number of used colors
   unsigned int biClrImportant;//the number of the important colors
} BITMAPINFOHEADER;

SVT_FileHandle Log_Handle = -1;

void SVT_PrepareApp( void )
{
  Log_Handle = -1;
}

void SVT_UnprepareApp( void )
{
  // Close in case it was open
  SVT_Log_Close();
  SVT_Osd_Close();
  JAPI_Exit();
}

void SVT_OpenIRControl()
{
  JAPI_GetIRCtrl();
}

void SVT_SetIRState( bool state )
{
  if ( state )
    JAPI_IRon( );
  else
    JAPI_IRoff( );
}

#define LOG_FILENAME "A:\\Test\\Trakr.log"
char Log_Buffer[ 256 ];

extern int kprintf(char *buf, const char *fmt, va_list args);
void SVT_CloseIRControl()
{
  JAPI_QuitIRCtrl( );
}

uint16 SVT_GetBatteryVoltage()
{
  return JAPI_ReadBatteryVoltage();
}

uint16 SVT_Motor_GetLeftCurrent()
{
  return JAPI_ReadADC( 2 );
}

uint16 SVT_Motor_GetRightCurrent()
{
  return JAPI_ReadADC( 3 );
}

int SVT_Key_GetStatus()
{
  return JAPI_GetCtrlStatus() >> 8;
}

void SVT_ResetTimer()
{
  JAPI_ResetTimer2();
}

uint16 SVT_GetTimer()
{
  return JAPI_GetTimer2();
}

unsigned short* JAPI_GetWchar(const char *string );
static int Graphics_FrameCount;
void Graphics_Frame( unsigned long YUV, unsigned long ulWidth, unsigned long ulHeight );

//TagOsd osd;

void SVT_Graphics_Open()
{
  Graphics_FrameCount = 0;


  JAPI_AttachVideoProc(Graphics_Frame);

  // Open low level
  //SVT_ROsd_Open();

}

void SVT_Graphics_Clear( )
{

}

void SVT_Graphics_DrawBitmap( uint8 index, uint8 x, uint8 y )
{
  //Osd_Bitmap( index, x, y );
}

void SVT_Graphics_Show( )
{
  //Osd_Show();
}

int SVT_Graphics_GetFrameCount()
{
  return Graphics_FrameCount;
}

void SVT_Graphics_Close()
{
  JAPI_FreeVideoProc();
}

void Graphics_Frame( unsigned long YUV, unsigned long ulWidth, unsigned long ulHeight )
{
  //unsigned short* pYUV = (unsigned short*)YUV;

//  if(osd.Flag!=osd.Old_Flag)
//    while(osd.Osd_Fresh(&osd)==0);

  Graphics_FrameCount++;

  // Draw_Text( (unsigned char* )pYUV, "DAVID", 10, 40 );

}

void SVT_Sleep( uint32 us )
{
  JAPI_Sleep( us / 6 );
}

#define Debug_TransFile  JAPI_TransFile
void SVT_Log_Open()
{
  SVT_FileSystem_Open();
  SVT_File_Delete( LOG_FILENAME );
  Log_Handle = SVT_File_Create( LOG_FILENAME );
}

void SVT_Log(String text, ... )
{
  if ( Log_Handle == -1 )
    SVT_Log_Open();

//  uint32 written;
//  SVT_File_Write( Log_Handle, "Log", 3, &written );
//  SVT_File_Write( Log_Handle, "\r\n", 2, &written );

  va_list argp;
  va_start(argp, text );
  uint32 len = kprintf( Log_Buffer, text, argp );

  uint32 written;
  SVT_File_Write( Log_Handle, Log_Buffer, len, &written );
  SVT_File_Write( Log_Handle, "\r\n", 2, &written );
  SVT_File_Flush( Log_Handle );
}

void SVT_Log_Close()
{
  if ( Log_Handle != -1 ) {
    SVT_File_Close( Log_Handle );
    Log_Handle = -1;
  }
}

static int Motor_State;

void SVT_Motor_Open()
{
  JAPI_MotorOpen();
  Motor_State = 0;
  JAPI_MotorCtrl( Motor_State );
}

void SVT_Motor_SetLeft( int speed )
{
  if ( speed != 0 ) {
    Motor_State |= MOTOR_LEFT_ON;
    if ( speed > 0 )
      Motor_State |= MOTOR_LEFT_DIRECTION;
    else
      Motor_State &= ~MOTOR_LEFT_DIRECTION;
  }
  else
    Motor_State &= ~MOTOR_LEFT_ON;

  JAPI_MotorCtrl( Motor_State );
}

void SVT_Motor_SetRight( int speed )
{
  if ( speed != 0 ) {
    Motor_State |= MOTOR_RIGHT_ON;
    if ( speed > 0 )
      Motor_State |= MOTOR_RIGHT_DIRECTION;
    else
      Motor_State &= ~MOTOR_RIGHT_DIRECTION;
  }
  else
    Motor_State &= ~MOTOR_RIGHT_ON;

  JAPI_MotorCtrl( Motor_State );
}

void SVT_Motor_Close()
{

}


void SVT_FileSystem_Open()
{
  JAPI_FS_Open();
}

void SVT_FileSystem_Close()
{
  JAPI_FS_Close();
}

long SVT_File_Create( char* pathName )
{
  unsigned short* wName = JAPI_GetWchar( pathName );
  JAPI_FS_FileCreate( 0, wName );
  return JAPI_FS_FileOpen( wName );
}

long SVT_File_Delete( char* pathName )
{
  return JAPI_FS_FileDelete( JAPI_GetWchar( pathName ) );
}

SVT_FileHandle SVT_File_Open( char* filename )
{
  return JAPI_FS_FileOpen( JAPI_GetWchar( filename ) );
}

void SVT_File_Close( SVT_FileHandle fh )
{
  JAPI_FS_FileClose( fh );
}

long SVT_FS_FileSeek( long fh, long pos )
{
  return JAPI_FS_FileSeek( fh, pos );
}

long SVT_File_Read( SVT_FileHandle fh, void* buffer, uint32 bufferLength, uint32* bytesRead )
{
  return JAPI_FS_FileRead( fh, buffer, bufferLength, bytesRead );
}

long SVT_File_Write( SVT_FileHandle fh, void* buffer, uint32 bufferLength, uint32* bytesWritten )
{
  return JAPI_FS_FileWrite( fh, buffer, bufferLength, bytesWritten );
}

long SVT_File_Flush( SVT_FileHandle fh )
{
  return JAPI_FS_FileFlush( fh );
}
/***********************************************************************
************************************************************************
String function
************************************************************************
***********************************************************************/
static unsigned short Wide_Char_Buf[512];

/***********************************************************************
Name:       SVT_Char_To_WChar
Created:    Kim
Date:       2010.04.24
***********************************************************************/
unsigned short* SVT_Char_To_WChar(const char* str)
{
  unsigned short* ps;
  ps=Wide_Char_Buf;
  while(*str!=0)
  {
    *ps++=(unsigned short)*str++;
  }
  *ps='\0';
  return Wide_Char_Buf;
}
/***********************************************************************
Name:       SVT_Str_Cmp
Created:    Kim
Date:       2010.04.24
***********************************************************************/
bool SVT_Str_Cmp(const char* cmp1,const char* cmp2)
{
  unsigned int i=0;
  while(*(cmp1+i))
  {
    if(*(cmp1+i)!=*(cmp2+i))
      return false;
    i++;
  }
  if(*(cmp2+i)!='\0')
    return false;
  return true;
}
/***********************************************************************
Name:       SVT_Str_Cpos
Created:    Kim
Date:       2010.04.24
***********************************************************************/
int SVT_Str_Cpos(char *dest, char pos)
{
  int ret=-1;
  while(*dest)
  {
    ret++;
    if(*dest++==pos)
      return ret;
  }
  return -1;
}
/***********************************************************************
Name:       SVT_Str_Pos
Created:    Kim
Date:       2010.04.24
***********************************************************************/
int SVT_Str_Pos(const char *dest, char *pos)
{
  int ret=-1;
  char *destBuf;
  char *destBuf2;
  char *posBuf;
  if(*pos==0)
    return ret;
  destBuf2=(char*)dest;
  while(*destBuf2)
  {
    ret++;
    if(*destBuf2==*pos)
    {
      destBuf=destBuf2;
      posBuf=pos;
      while(*posBuf)
      {
        if(*destBuf++!=*posBuf)
          break;
        posBuf++;
      }
      if(*posBuf==0)
        return ret;
    }
    destBuf2++;
  }
  return -1;
}

/***********************************************************************
Name:       SVT_Str_Length
Created:    Kim
Date:       2010.04.24
***********************************************************************/
int SVT_Str_Length(const char * src)
{
  int ret=0;
  while(*(src+ret))
    ret++;
  return ret;
}

char stdout[512]__attribute__((aligned(16)));

#define ZEROPAD	    (0x1<<00)
#define SIGN	      (0x1<<01)
#define PLUS	      (0x1<<02)
#define SPACE	      (0x1<<03)
#define LEFT	      (0x1<<04)
#define SPECIAL	    (0x1<<05)
#define SMALL	      (0x1<<06)

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define do_div(n,base) ({int tmp=n;n=tmp/base;tmp%=base;})

static int skip_atoi(const char **s)
{
    int i = 0;
    while (is_digit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

static char * number(char * str, int num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base<2 || base>36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	if (type&SIGN && num<0) {
		sign='-';
		num = -num;
	} else
		sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type&SPECIAL)
	{
		if (base==16) size -= 2;
		else if (base==8) size--;
    }
	i=0;
	if (num==0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i>precision) precision=i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type&SPECIAL)
	{
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
    }
	if (!(type&LEFT))
		while(size-->0)
			*str++ = c;
	while(i<precision--)
		*str++ = '0';
	while(i-->0)
		*str++ = tmp[i];
	while(size-->0)
		*str++ = ' ';
	return str;
}

int kprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			break;

		case 's':
			s = va_arg(args, char *);
			len = SVT_Str_Length(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;

		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;

		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buf);
			break;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			break;
		}
	}
	*str = '\0';
	return str-buf;
}
/***********************************************************************
Name:       SVT_Print
Created:    Kim
Date:       2010.04.24
***********************************************************************/
int SVT_Print(char *string, ...)
{
  va_list arg;
  int done;

  va_start (arg, string);
  done = kprintf (stdout, string, arg);
  va_end (arg);

  return done;
}

/***********************************************************************
         ############### OSD(On Screen Display) ###############
***********************************************************************/

//RGB555; if the high bit is set, it will be transparented.
#define SCOLOR_INI   0x8000

char Rgb_Buf[2][WIDTH*HEIGHT*2]__attribute__((aligned(256)));
#define Osd_Set_Symbol(a,b) SVT_Memory_Copy((a),(b),4)
#define Osd_Int_To_Char(a,b) SVT_Memory_Copy((a),(b),4)

unsigned int Linewidth;
unsigned int Osd_Flag=0;
#define OSD_FG_BUF		  (0x1<<00)
#define OSD_FG_OPEN       (0x1<<01)
#define OSD_FG_STR        (0x1<<02)
#define OSD_FG_RB         (0x1<<03)   /*right bounary*/
#define OSD_FG_SHOW       (0x1<<04)   /*show or not.*/
#define OSD_FG_MASK       (0x1<<05)   /*show or not.*/
#define OSD_FG_CMDSEND    (0x1<<06)   /*show or not.*/


#define OSD_MAX_FILE_BUFFER_SIZE  (48*1024)
volatile char ROsd_Bmp_Buffer[OSD_MAX_FILE_BUFFER_SIZE]__attribute__((aligned(256)));
volatile char Osd_Bmp_Buffer[OSD_MAX_FILE_BUFFER_SIZE]__attribute__((aligned(256)));
char Scroll_Buf[WIDTH*2]__attribute__((aligned(4)));
unsigned int Osd_Count;
unsigned int ROsd_Count;

typedef struct
{
  unsigned char *pFile_Addr;
  unsigned int File_Size;
}TAG_OSD_PACKED_FILES;

#define OSD_MAX_FILES     256
TAG_OSD_PACKED_FILES Osd_Files[OSD_MAX_FILES];
TAG_OSD_PACKED_FILES ROsd_Files[OSD_MAX_FILES];
unsigned int Osd_Yuv_Mask_Cb;
unsigned int Osd_Yuv_Mask_Cr;


extern void Draw_2_Rgb_Pixel_On_Yuv_0(char *pYuv, char *pRgb);
extern void Draw_2_Rgb_Pixel_On_Yuv_1(char *pYuv, char *pRgb);
extern void Draw_2_Rgb_Pixel_On_Yuv_2(char *pYuv, char *pRgb);
extern void Draw_2_Rgb_Pixel_On_Yuv(char *pYuv, char *pRgb);
extern void SVT_Osd_Draw_Buffer_To_Yuv(unsigned long YUV,unsigned long w,unsigned long h);

void SVT_Osd_Pack_Bmps_Open(void)
{
  Osd_Count=0;
}

int SVT_Osd_Pack_Bmps(char *pFile_Addr, unsigned int File_Size)
{
  if(Osd_Count>=OSD_MAX_FILES)
    return OSD_OVER_FILE_SIZE;
  Osd_Files[Osd_Count].pFile_Addr=pFile_Addr;
  Osd_Files[Osd_Count++].File_Size=File_Size;
  return Osd_Count;
}
int SVT_Osd_Pack_Bmps_Send(void)
{
  if(Osd_Count==0)
    return OSD_NONE;
  char *pFile=(char*)Osd_Bmp_Buffer;
  char *pAddr;
  unsigned int i,j;
  unsigned int File_Size;
  Osd_Set_Symbol(pFile,"BMPS");
  Osd_Int_To_Char(pFile+4,(char *)&Osd_Count);
  pAddr=pFile+8;
  File_Size=4*(Osd_Count+3);
  pFile+=File_Size;
  Osd_Int_To_Char(pAddr,(char *)&File_Size);
  j=File_Size;
  for(i=0;i<Osd_Count;i++)
  {
    pAddr+=4;
    if((File_Size+Osd_Files[i].File_Size)>OSD_MAX_FILE_BUFFER_SIZE)
      return OSD_OVER_FILE_BUFFER_SIZE;
    SVT_Memory_Copy((char*)(Osd_Bmp_Buffer+File_Size),Osd_Files[i].pFile_Addr,Osd_Files[i].File_Size);
    File_Size+=Osd_Files[i].File_Size;
    Osd_Int_To_Char(pAddr,(char *)&File_Size);
  }
  Osd_Set_Symbol((char*)(Osd_Bmp_Buffer+File_Size),"ENDS");
  return OSD_SENT;
}
void SVT_Osd_Pack_Bmps_Close(void)
{
  //Osd_Set_Symbol((char*)Osd_Bmp_Buffer,"ABCD");
  Osd_Count=0;
}

#define exchange(a,b) \
{\
  int exc;\
  exc=(a);\
  (a)=(b);\
  (b)=exc;\
}
void SVT_Osd_Clear_Rectangle(
            int lx,
            int ty,
            int rx,
            int by)
{
  TAG_COLOR cl={0,0,0,1};
  SVT_Osd_Set_Rectangle(lx,ty,rx,by,&cl);
}
void SVT_Osd_Set_Rectangle(
            int lx,
            int ty,
            int rx,
            int by,
            TAG_COLOR *rgba)
{
  volatile unsigned short c;
  volatile unsigned short *p;
  volatile char *crb;
  int i,j;
  c=(((unsigned short)(rgba->R&0xf8))<<7)+\
    (((unsigned short)(rgba->G&0xf8))<<2)+\
    (((unsigned short)(rgba->B&0xf8))>>3);
  if(rgba->Transparent!=0)
    c=0x8000;
  crb=(char*)&Rgb_Buf[OSD_FG_BUF&Osd_Flag][0];
  if(lx>rx) exchange(lx,rx);
  if(ty>by) exchange(ty,by);
  if((rx<0)|(by<0))return;
  lx=lx>0?lx:0;
  rx=rx<WIDTH?rx:WIDTH-1;
  ty=ty>0?ty:0;
  by=by<HEIGHT?by:HEIGHT-1;
  for(i=ty;i<=by;i++)
  {
    p=(unsigned short *)(crb+i*WIDTH*2);
    for(j=lx;j<=rx;j++)
    {
      *(p+j)=c;
    }
  }
}
void SVT_Osd_Draw_Rectangle(
  int lx,
  int ty,
  int rx,
  int by,
  TAG_COLOR *rgba)
{
  volatile unsigned short c;
  volatile unsigned int i,j;
  int e,s;
  volatile char *crb;
  volatile unsigned short *p;
  crb=(char*)&Rgb_Buf[OSD_FG_BUF&Osd_Flag][0];
  c=(((unsigned short)(rgba->R&0xf8))<<7)+\
    (((unsigned short)(rgba->G&0xf8))<<2)+\
    (((unsigned short)(rgba->B&0xf8))>>3);
  if(rgba->Transparent!=0)
    c+=0x1<<15;
  if(lx>rx) exchange(lx,rx);
  if(ty>by) exchange(ty,by);
  if((rx<0)|(by<0))return;
  for(j=0;j<Linewidth;j++)
  {
    //horizontal lines
    s=lx>0?lx:0;
    e=(rx+Linewidth)<WIDTH?rx+Linewidth:WIDTH;
    //top line.
    if((ty+j)>=0)
    {
      p=(unsigned short *)(crb+((ty+j)*WIDTH+s)*2);
      for(i=(unsigned int)s;i<(unsigned int)e;i++)
        *p++=c;
    }
    //bottom line
    if((by+j)<HEIGHT)
    {
      p=(unsigned short *)(crb+((by+j)*WIDTH+s)*2);
      for(i=(unsigned int)s;i<(unsigned int)e;i++)
        *p++=c;
    }
    //vertical lines
    s=ty>0?ty:0;
    e=(by+Linewidth)<HEIGHT?by+Linewidth:HEIGHT;
    //left line
    if((lx+j)>=0)
    {
      p=(unsigned short *)(crb+(s*WIDTH+(lx+j))*2);
      for(i=0;i<(unsigned int)(e-s);i++)
      {
        *p=c;
        p+=WIDTH;
      }
    }
    //right line
    if((rx+j)<WIDTH)
    {
      p=(unsigned short *)(crb+(s*WIDTH+(rx+j))*2);
      for(i=0;i<(unsigned int)(e-s);i++)
      {
        *p=c;
        p+=WIDTH;
      }
    }
  }
}
void SVT_Osd_Open(void)
{
  TAG_COLOR tc;
  tc.R=0xff;
  tc.G=0xff;
  tc.B=0xff;
  Osd_Flag=OSD_FG_OPEN;
  Osd_Flag=0;
  Linewidth=1;
  SVT_Memory_Set_HW((short *)&Rgb_Buf[0][0],SCOLOR_INI,WIDTH*HEIGHT);
  SVT_Memory_Set_HW((short *)&Rgb_Buf[1][0],SCOLOR_INI,WIDTH*HEIGHT);
  SVT_Osd_Set_Text_Boundary(2,2);
  SVT_Osd_Set_Text_Interval(2,2);
  SVT_Osd_Set_Text_Color(&tc);
  JAPI_AttachVideoProc(SVT_Osd_Draw_Buffer_To_Yuv);
  Osd_Yuv_Mask_Cb=64;
  Osd_Yuv_Mask_Cr=64;
}
void SVT_Osd_Close(void)
{
  Osd_Flag=0;
  JAPI_FreeVideoProc();
}

void inline SVT_Osd_Set_Linewidth(unsigned int w)
{
  w=w==0?1:w;
  Linewidth=w<10?w:10;
}
unsigned char inline SVT_Osd_Get_Linewidth(void)
{
  return Linewidth;
}

unsigned int tLeftBoundary=2;
unsigned int tRightBoundary=160-2;
unsigned int tCharInterval=2;
unsigned int tLineInterval=2;
char* Osd_Text_Char_Lib_Addr;
unsigned int Osd_Text_Char_Lib_Len;
int osd_text_x;
int osd_text_y;
TAG_COLOR      tColor=
{
  0xff,0xff,0xff,0x00
};

void SVT_Osd_Set_Text_Boundary(unsigned int lx,unsigned int rx)
{
  tLeftBoundary=lx;
  tRightBoundary=160-rx;
}
void SVT_Osd_Set_Text_Interval(
  unsigned int ci,
  unsigned int li)
{
  tCharInterval=ci;
  tLineInterval=li;
}
void SVT_Osd_Set_Text_Color(TAG_COLOR *c)
{
  tColor.R=c->R;
  tColor.G=c->G;
  tColor.B=c->B;
  tColor.Transparent=0;
}

unsigned char Get_Bmp_Index(unsigned char *ucBuf,int index,unsigned char BitCount)
{
  unsigned char ucBit;
  unsigned char Shift_Bit;
  if(BitCount==1)
  {
    ucBit=0x80;
    ucBit=ucBit>>(index%8);
    if(*(ucBuf+index/8)&ucBit)
      return 1;
    else
      return 0;
  }
  else if(BitCount==2)
  {
    ucBit=0xc0;
    Shift_Bit=(index%4)<<1;
    ucBit=ucBit>>Shift_Bit;
    return ((*(ucBuf+index/4)&ucBit)>>(6-Shift_Bit))&0x3;
  }
  else if(BitCount==4)
  {
    ucBit=0xf0;
    Shift_Bit=(index%2)<<2;
    ucBit=ucBit>>Shift_Bit;
    return ((*(ucBuf+index/2)&ucBit)>>(4-Shift_Bit))&0xf;
  }
  return 0;
}

int SVT_Osd_Draw_Bitmap_To_Buffer(
     char *pBmpBuffer,
     int x,
     int y,
     TAG_COLOR *cTransparent)
{
  BITMAPFILEHEADER fd;
  BITMAPINFOHEADER ih;
  char *pBmp;
  char *pRgb;
  char *pRgbT;
  char iBytes;
  pBmp=pBmpBuffer;
  unsigned short cb;
  unsigned short c[16];
  unsigned char uc[80];
  int i,j;
  unsigned char cCount;
  SVT_Memory_Copy((unsigned char*)&fd,pBmp,sizeofBITMAPFILEHEADER);
  if(fd.bfType!=0x4d42)
    return -1;
  pBmp+=sizeofBITMAPFILEHEADER;
  SVT_Memory_Copy((unsigned char*)&ih,pBmp,sizeofBITMAPINFOHEADER);
  pBmp+=sizeofBITMAPINFOHEADER;
  if(!((ih.biBitCount==1)||
      (ih.biBitCount==2)||
      (ih.biBitCount==4)))
    return -1;
  cCount=0x1<<(ih.biBitCount);
  for(i=0;i<cCount;i++)
  {
    if((*(pBmp+2)==cTransparent->R)&&
       (*(pBmp+1)==cTransparent->G)&&
       (*(pBmp+0)==cTransparent->B))
      c[i]=SCOLOR_INI;
    else
      c[i]=(((unsigned short)(*(pBmp+2)&0xf8))<<7)+\
           (((unsigned short)(*(pBmp+1)&0xf8))<<2)+\
           (((unsigned short)(*(pBmp+0)&0xf8))>>3);
    pBmp+=4;
  }
  if(Osd_Flag&OSD_FG_STR)
  {
    c[0]=(((unsigned short)(tColor.R&0xf8))<<7)+\
         (((unsigned short)(tColor.G&0xf8))<<2)+\
         (((unsigned short)(tColor.B&0xf8))>>3);
    c[1]=SCOLOR_INI;
    i=x+(int)ih.biWidth;
    j=y+(int)ih.biHeight;
    if((x+(int)ih.biWidth)>=(int)tRightBoundary)
    {
      Osd_Flag|=OSD_FG_RB;
      return (int)ih.biHeight;
    }
    if((i<=tLeftBoundary)||
       (x>tRightBoundary)||
       (j<=0)||
       (y>120))
      return 0;
  }
  if(ih.biWidth%(8/ih.biBitCount)==0)
    iBytes=ih.biWidth/(8/ih.biBitCount);
  else
    iBytes=ih.biWidth/(8/ih.biBitCount)+1;
  if(iBytes%4!=0)
    iBytes=(iBytes/4+1)*4;
  pRgb=(char*)&Rgb_Buf[Osd_Flag&OSD_FG_BUF][0];
  for(i=0;i<ih.biHeight;i++)
  {
    if((y+i)>=120)
      break;
    if((y+i)<0)
      continue;
    pRgbT=pRgb+y*320+(ih.biHeight-i-1)*320+x*2;
    SVT_Memory_Copy(uc,pBmp,iBytes);
    pBmp+=iBytes;
    for(j=0;j<ih.biWidth;j++)
    {
      if((x+j)>=160)
        break;
      if((x+j)<0)
      {
        pRgbT+=2;
        continue;
      }
      cb=c[Get_Bmp_Index(uc,j,ih.biBitCount)];
      if(cb!=SCOLOR_INI)
        *(unsigned short*)pRgbT=cb;
      pRgbT+=2;
    }
  }
  return ih.biWidth;
}
void SVT_Osd_Set_Text_Font(char *pcb, unsigned int clen)
{
  Osd_Text_Char_Lib_Addr=pcb;
  Osd_Text_Char_Lib_Len=clen;
}
int SVT_Osd_Draw_Text(int x, int y, char * fmt,...)
{
  osd_text_x=x;
  osd_text_y=y;

  va_list arg;
  char *p;
  char *pc;
  int Ret;
  unsigned char uc;
  unsigned int Addr;
  pc=Osd_Text_Char_Lib_Addr;
  int len;

  va_start (arg, fmt);
  len = kprintf(stdout, fmt, arg);
  va_end (arg);
  p=(char*)stdout;
  x+=(int)tLeftBoundary;
  Osd_Flag&=~OSD_FG_RB;
  while(*p)
  {
    if((*p<0x20)||(*p>0x7f)) *p='?';
    uc=*p-0x20;
    SVT_Memory_Copy((char*)&Addr,pc,4);
    if(uc>=Addr) uc='?';
    SVT_Memory_Copy((char*)&Addr,pc+(uc+1)*4,4);
    Osd_Flag|=OSD_FG_STR;
    if(uc!=0x20)
      Ret=SVT_Osd_Draw_Bitmap_To_Buffer(pc+Addr,x,y,&tColor);
    else
      Ret=0x4;
    if(Osd_Flag&OSD_FG_RB)
    {
      Osd_Flag&=~OSD_FG_RB;
      x=(signed int)tLeftBoundary;
      y+=(signed int)tLineInterval+16;
    }
    else
    {
      x+=Ret+tCharInterval;
      p++;
    }
  }
  Osd_Flag&=~(OSD_FG_RB|OSD_FG_STR);
  return len;
}

int Compare_ID(char *uc1, char *uc2)
{
  while(*uc2)
  {
    if(*uc1++!=*uc2++)
      return false;
  }
  return true;
}
void SVT_Osd_Draw_Bitmap(
     char BmpIndex,
     int x,
     int y,
     TAG_COLOR *cTransparent)
{
  char *p;
  unsigned int BmpCount;
  p=(char *)Osd_Bmp_Buffer;
  if(Compare_ID(p,"BMPS")==false)
    return;
  SVT_Memory_Copy((char*)&BmpCount,p+4,4);
  if(BmpIndex>=BmpCount)
    return;
  p=*((unsigned int *)(p+8+4*BmpIndex))+p;
  SVT_Osd_Draw_Bitmap_To_Buffer(p,x,y,cTransparent);
}
void SVT_Osd_Show(void)
{
  if(Osd_Flag&OSD_FG_BUF)
  {
    Osd_Flag&=~OSD_FG_BUF;
    SVT_Memory_Copy((char*)&Rgb_Buf[0][0],(char*)&Rgb_Buf[1][0],WIDTH*HEIGHT*2);
  }
  else
  {
    Osd_Flag|=OSD_FG_BUF;
    SVT_Memory_Copy((char*)&Rgb_Buf[1][0],(char*)&Rgb_Buf[0][0],WIDTH*HEIGHT*2);
  }
  Osd_Flag|=OSD_FG_SHOW;
}

void SVT_Osd_Yuv_Mask_Open(unsigned int Cb, unsigned int Cr)
{
  Osd_Yuv_Mask_Cb=Cb;
  Osd_Yuv_Mask_Cr=Cr;
  Osd_Flag|=OSD_FG_MASK;
}

void SVT_Osd_Yuv_Mask_Close(void)
{
  Osd_Flag&=~OSD_FG_MASK;
}

void SVT_Osd_Draw_Buffer_To_Yuv(unsigned long YUV,unsigned long w,unsigned long h)
{
  char *py=(char*)YUV;
  if((Osd_Flag&OSD_FG_SHOW)==0)
    return;
  if(Osd_Flag&OSD_FG_MASK)
    SVT_Mask_YUV(py,Osd_Yuv_Mask_Cb,Osd_Yuv_Mask_Cr);
  char *pr;
  if(Osd_Flag&OSD_FG_BUF)
    pr=&Rgb_Buf[0][0];
  else
    pr=&Rgb_Buf[1][0];
  Draw_2_Rgb_Pixel_On_Yuv(py,pr);
}

void SVT_ROsd_Pack_Bmps_Open(void)
{
  ROsd_Count=0;
}

int SVT_ROsd_Pack_Bmps(char *pFile_Addr, unsigned int File_Size)
{
  if(ROsd_Count>=OSD_MAX_FILES)
    return OSD_OVER_FILE_SIZE;
  ROsd_Files[ROsd_Count].pFile_Addr=pFile_Addr;
  ROsd_Files[ROsd_Count++].File_Size=File_Size;
  return ROsd_Count-1;
}
int SVT_ROsd_Pack_Bmps_Send(void)
{
  if(ROsd_Count==0)
    return OSD_NONE;
  char *pFile=(char*)ROsd_Bmp_Buffer;
  char *pAddr;
  unsigned int i,j;
  unsigned int File_Size;
  Osd_Set_Symbol(pFile,"BMPS");
  Osd_Int_To_Char(pFile+4,(char *)&ROsd_Count);
  pAddr=pFile+8;
  File_Size=4*(ROsd_Count+3);
  pFile+=File_Size;
  Osd_Int_To_Char(pAddr,(char *)&File_Size);
  j=File_Size;
  for(i=0;i<ROsd_Count;i++)
  {
    pAddr+=4;
    if((File_Size+ROsd_Files[i].File_Size)>OSD_MAX_FILE_BUFFER_SIZE)
      return OSD_OVER_FILE_BUFFER_SIZE;
    SVT_Memory_Copy((char*)(ROsd_Bmp_Buffer+File_Size),ROsd_Files[i].pFile_Addr,ROsd_Files[i].File_Size);
    File_Size+=ROsd_Files[i].File_Size;
    Osd_Int_To_Char(pAddr,(char *)&File_Size);
  }
  Osd_Set_Symbol((char*)(ROsd_Bmp_Buffer+File_Size),"ENDS");
  File_Size+=4;
  i=0;
  do
  {
    if(i++>3)
      break;
    j=Debug_TransFile((unsigned char*)ROsd_Bmp_Buffer,File_Size);
  }while(j==0);
  if(j==0)
    return OSD_FAIL;
  else
    return OSD_SENT;
}
void SVT_ROsd_Pack_Bmps_Close(void)
{
  //Osd_Set_Symbol((char*)ROsd_Bmp_Buffer,"ABCD");
  ROsd_Count=0;
}
void SVT_Osd_Set_Screen(TAG_COLOR *rgba)
{
  unsigned short c;
  c=(((unsigned short)(rgba->R&0xf8))<<7)+\
    (((unsigned short)(rgba->G&0xf8))<<2)+\
    (((unsigned short)(rgba->B&0xf8))>>3);
  if(rgba->Transparent!=0)
    c=0x1<<15;
  SVT_Memory_Set_HW((short*)&Rgb_Buf[OSD_FG_BUF&Osd_Flag][0],c,WIDTH*HEIGHT);
}
void SVT_Osd_Clear_Screen(void)
{
  TAG_COLOR cl={0,0,0,1};
  SVT_Osd_Set_Screen(&cl);
}

void SVT_Osd_ScrollRectangle(
  int lx,int ty,int rx,int by,
  int mx,int my)
{
  int i;
  int lines;
  int width;
  unsigned char *pDest;
  int nmlx,nmrx,nmy;
  if(mx==0&&my==0)
    return;
  if(lx>160||ty>120||rx<0||by<0)
    return;
  pDest=(unsigned char*)&Rgb_Buf[Osd_Flag&OSD_FG_BUF][0];
  lx=lx>0?lx:0;
  rx=rx<WIDTH?rx:WIDTH-1;
  ty=ty>0?ty:0;
  by=by<HEIGHT?by:HEIGHT-1;
  lines=by-ty;
  if(lx==rx||ty==by)
    return;
  if(my<0)
  {
    for(i=0;i<lines;i++)
    {
      nmy=ty+my+i;
      nmlx=lx+mx>0?lx+mx:0;
      nmrx=rx+mx<WIDTH?rx+mx:WIDTH-1;
      width=nmrx-nmlx+1;
      if(nmy>=0)
        SVT_Memory_Copy(Scroll_Buf,(char*)(pDest+((ty+i)*WIDTH+lx)*2),width*2);
      SVT_Memory_Set_HW((short*)(pDest+((ty+i)*WIDTH+lx)*2),0x8000,rx-lx+1);
      if(nmy>=0)
        SVT_Memory_Copy((char*)(pDest+(nmy*WIDTH+nmlx)*2),Scroll_Buf,width*2);
    }
  }
  else
  {
    for(i=0;i<lines;i++)
    {
      nmy=by+my-i;
      nmlx=lx+mx>0?lx+mx:0;
      nmrx=rx+mx<WIDTH?rx+mx:WIDTH-1;
      width=nmrx-nmlx+1;
      if(nmy<120)
        SVT_Memory_Copy(Scroll_Buf,(char*)(pDest+((by-i)*WIDTH+lx)*2),width*2);
      SVT_Memory_Set_HW((short*)(pDest+((by-i)*WIDTH+lx)*2),0x8000,rx-lx+1);
      if(nmy<120)
        SVT_Memory_Copy((char*)(pDest+(nmy*WIDTH+nmlx)*2),Scroll_Buf,width*2);
    }
  }
}


#define OSD_CMD_MAX_SIZE  (2*1024)
volatile char CMD_Buf[OSD_CMD_MAX_SIZE]__attribute__((aligned(256)));
unsigned char CMD_Index;
unsigned short CMD_Length=0;
unsigned char *pCmd;
#define CMD_ID                     "CMDS"
#define CMD_SID_ENDS               "ENDS"
#define CMD_SID_MASKOPEN           0
#define CMD_SID_MASKCLOSE          1
#define CMD_SID_OPEN               2
#define CMD_SID_SHOW               3
#define CMD_SID_CLOSE              4
#define CMD_SID_DRAWRECTANGLE      5
#define CMD_SID_SETRECTANGLE       6
#define CMD_SID_TEXT               7
#define CMD_SID_BITMAP             8
#define CMD_SID_SETSCREEN          9
#define CMD_SID_SCROLLRECTANGLE    10

void Debug_TransCmd(void)
{
  JAPI_OpenRfVideo();
  JAPI_SendRfVideo((unsigned long *)CMD_Buf,(unsigned long)CMD_Length);
  JAPI_CloseRfVideo();
}

void SVT_ROsd_Yuv_Mask_Open(unsigned int Cb, unsigned int Cr)
{
  char* pb;
  char* pr;
  Cb=Cb<0x4000?Cb:0x4000;
  Cr=Cr<0x4000?Cr:0x4000;
  pb=(char*)&Cb;
  pr=(char*)&Cr;
  *pCmd++=CMD_SID_MASKOPEN;
  *pCmd++=*pb++;
  *pCmd++=*pb++;
  *pCmd++=*pr++;
  *pCmd++=*pr++;
}

void SVT_ROsd_Yuv_Mask_Close(void)
{
  *pCmd++=CMD_SID_MASKCLOSE;
}
void SVT_ROsd_Open(void)
{
  CMD_Length=0;
  pCmd=(char*)CMD_Buf;
  SVT_Memory_Copy(pCmd,CMD_ID,4);
  pCmd+=10;
  *pCmd++=CMD_SID_OPEN;
  Osd_Flag&=~OSD_FG_CMDSEND;
}
void SVT_ROsd_Show(void)
{
  unsigned int i,Check_Sum,is,ie;
  if((unsigned int)pCmd==(unsigned int)CMD_Buf)
    return;
  *pCmd++=CMD_SID_SHOW;
  Osd_Set_Symbol(pCmd,CMD_SID_ENDS);
  pCmd+=4;
  is=(unsigned int)CMD_Buf;
  ie=(unsigned int)pCmd;
  CMD_Length=ie-is;
  pCmd=(unsigned char*)(CMD_Buf+8);
  SVT_Memory_Copy(pCmd,(unsigned char*)&CMD_Length,2);
  Check_Sum=0;
  for(i=8;i<CMD_Length;i++)
    Check_Sum+=*pCmd++;
  CMD_Index++;
  if(CMD_Index==0)
    CMD_Index++;
  Check_Sum=(Check_Sum&0xffffff)+(CMD_Index<<24);
  SVT_Memory_Copy((unsigned char*)(CMD_Buf+4),(unsigned char*)&Check_Sum,4);
  Osd_Flag|=OSD_FG_CMDSEND;
}
void SVT_ROsd_Close(void)
{
  unsigned int i,Check_Sum,is,ie;
  SVT_ROsd_Clear_Screen();
  if((unsigned int)pCmd==(unsigned int)CMD_Buf)
    return;
  *pCmd++=CMD_SID_CLOSE;
  Osd_Set_Symbol(pCmd,CMD_SID_ENDS);
  pCmd+=4;
  is=(unsigned int)CMD_Buf;
  ie=(unsigned int)pCmd;
  CMD_Length=ie-is;
  pCmd=(unsigned char*)(CMD_Buf+8);
  SVT_Memory_Copy(pCmd,(unsigned char*)&CMD_Length,2);
  Check_Sum=0;
  for(i=8;i<CMD_Length;i++)
    Check_Sum+=*pCmd++;
  CMD_Index++;
  if(CMD_Index==0)
    CMD_Index++;
  Check_Sum=(Check_Sum&0xffffff)+(CMD_Index<<24);
  SVT_Memory_Copy((unsigned char*)(CMD_Buf+4),(unsigned char*)&Check_Sum,4);
  Osd_Flag|=OSD_FG_CMDSEND;
}

void SVT_ROsd_Draw_Rectangle(
  int lx,
  int ty,
  int rx,
  int by,
  TAG_COLOR *rgba)
{
  *pCmd++=CMD_SID_DRAWRECTANGLE;
  signed short slx=lx,sty=ty,srx=rx,sby=by;
  SVT_Memory_Copy(pCmd+0,(char*)&slx,2);
  SVT_Memory_Copy(pCmd+2,(char*)&sty,2);
  SVT_Memory_Copy(pCmd+4,(char*)&srx,2);
  SVT_Memory_Copy(pCmd+6,(char*)&sby,2);
  pCmd+=8;
  *pCmd++=rgba->R;
  *pCmd++=rgba->G;
  *pCmd++=rgba->B;
  *pCmd++=*((char*)&Linewidth);
}
void SVT_ROsd_Clear_Rectangle(
            int lx,
            int ty,
            int rx,
            int by)
{
  TAG_COLOR cl={0,0,0,1};
  SVT_ROsd_Set_Rectangle(lx,ty,rx,by,&cl);
}

int  SVT_ROsd_Draw_Text(int x, int y, char * fmt,...)
{
  signed short slb=tLeftBoundary,srb=tRightBoundary,sci=tCharInterval,sli=tLineInterval,sx=x,sy=y;
  va_list arg;
  unsigned short len;
  va_start (arg, fmt);
  len = (unsigned short)kprintf(stdout, fmt, arg)+1;
  va_end (arg);
  *pCmd++=CMD_SID_TEXT;
  SVT_Memory_Copy(pCmd+0,(char*)&len,2);
  SVT_Memory_Copy(pCmd+2,(char*)&slb,2);
  SVT_Memory_Copy(pCmd+4,(char*)&srb,2);
  SVT_Memory_Copy(pCmd+6,(char*)&sci,2);
  SVT_Memory_Copy(pCmd+8,(char*)&sli,2);
  pCmd+=10;
  *pCmd++=tColor.R;
  *pCmd++=tColor.G;
  *pCmd++=tColor.B;
  *pCmd++=tColor.Transparent;
  SVT_Memory_Copy(pCmd,(char*)&sx,2);
  SVT_Memory_Copy(pCmd+2,(char*)&sy,2);
  pCmd+=4;
  SVT_Memory_Copy(pCmd,stdout,len);
  pCmd+=len;
  return len;
}
void SVT_ROsd_Draw_Bitmap(
            char BmpIndex,
            int x,
            int y,
            TAG_COLOR *cTransparent)
{
  signed short sx=x,sy=y;
  *pCmd++=CMD_SID_BITMAP;
  SVT_Memory_Copy(pCmd+0,(char*)&sx,2);
  SVT_Memory_Copy(pCmd+2,(char*)&sy,2);
  pCmd+=4;
  *pCmd++=cTransparent->R;
  *pCmd++=cTransparent->G;
  *pCmd++=cTransparent->B;
  *pCmd++=cTransparent->Transparent;
  *pCmd++=BmpIndex;
}

void SVT_ROsd_Set_Rectangle(
            int lx,
            int ty,
            int rx,
            int by,
            TAG_COLOR *rgba)
{
  *pCmd++=CMD_SID_SETRECTANGLE;
  signed short slx=lx,sty=ty,srx=rx,sby=by;
  SVT_Memory_Copy(pCmd+0,(char*)&slx,2);
  SVT_Memory_Copy(pCmd+2,(char*)&sty,2);
  SVT_Memory_Copy(pCmd+4,(char*)&srx,2);
  SVT_Memory_Copy(pCmd+6,(char*)&sby,2);
  pCmd+=8;
  *pCmd++=rgba->R;
  *pCmd++=rgba->G;
  *pCmd++=rgba->B;
  *pCmd++=rgba->Transparent;
}

void SVT_ROsd_Set_Screen(TAG_COLOR *rgba)
{
  *pCmd++=CMD_SID_SETSCREEN;
  *pCmd++=rgba->R;
  *pCmd++=rgba->G;
  *pCmd++=rgba->B;
  *pCmd++=rgba->Transparent;
}

void SVT_ROsd_Clear_Screen(void)
{
  TAG_COLOR cl={0,0,0,1};
  SVT_ROsd_Set_Screen(&cl);
}

void SVT_ROsd_ScrollRectangle(
  int lx,int ty,int rx,int by,
  int mx,int my)
{
  *pCmd++=CMD_SID_SCROLLRECTANGLE;
  signed short slx=lx,sty=ty,srx=rx,sby=by,smx=mx,smy=my;
  SVT_Memory_Copy(pCmd+0,(char*)&slx,2);
  SVT_Memory_Copy(pCmd+2,(char*)&sty,2);
  SVT_Memory_Copy(pCmd+4,(char*)&srx,2);
  SVT_Memory_Copy(pCmd+6,(char*)&sby,2);
  SVT_Memory_Copy(pCmd+8,(char*)&smx,2);
  SVT_Memory_Copy(pCmd+10,(char*)&smy,2);
  pCmd+=12;
}

bool SVT_ROsd_Is_Cmd_Sent(void)
{
  if(Osd_Flag&OSD_FG_CMDSEND) return false;
  return true;
}
void SVT_ROsd_Send_Cmd(void)
{
  if(Osd_Flag&OSD_FG_CMDSEND)
  {
    Debug_TransCmd();
    if((JAPI_GetCmdAck()&0xff)==CMD_Index)
    {
      CMD_Length=0;
      pCmd=(char*)CMD_Buf;
      SVT_Memory_Copy(pCmd,CMD_ID,4);
      pCmd+=10;
      Osd_Flag&=~OSD_FG_CMDSEND;
    }
  }
}
bool SVT_ROsd_Send_Cmd_Times(unsigned int time)
{
  bool ret=false;
  int timeRequest = time;
  JAPI_OpenRfVideo();
  //int test;
  for(;time>0;time--)
  {
	int test = 0;
	int index;

    if (LOGGING) SVT_Log( "    Write Attempt %d", timeRequest - time );
	JAPI_SendRfVideo((unsigned long *)CMD_Buf,(unsigned long)CMD_Length);

    if (LOGGING) SVT_Log( "      Waiting for Index %d", CMD_Index );
    do {
    	index = JAPI_GetCmdAck()&0xff;
  	    if ( index != CMD_Index )
  	      if (LOGGING) SVT_Log( "      Call %d: Wrong Index %d", test + 1, index );
    	SVT_Sleep( 1 );
    } while ( ++test < 100 && index != CMD_Index );

	if(index==CMD_Index)
    {
	  if (LOGGING) SVT_Log( "      Call %d: Right Index %d", test, index );

      CMD_Length=0;
      pCmd=(char*)CMD_Buf;
      SVT_Memory_Copy(pCmd,CMD_ID,4);
      pCmd+=10;
      Osd_Flag&=~OSD_FG_CMDSEND;
      ret=true;
      break;
    }
  }
  if ( time > 0 )
	  if (LOGGING) SVT_Log( "    Send took %d ", timeRequest - time + 1 );
  JAPI_CloseRfVideo();
  return ret;
}

bool SVT_ROsd_Send_Cmd_Times_Wait(unsigned int time)
{
  bool ret=false;
  JAPI_OpenRfVideo();
  for(;time>0;time--)
  {
    JAPI_SendRfVideo((unsigned long *)CMD_Buf,(unsigned long)CMD_Length);
    if((JAPI_GetCmdAck()&0xff)==CMD_Index)
    {
      CMD_Length=0;
      pCmd=(char*)CMD_Buf;
      SVT_Memory_Copy(pCmd,CMD_ID,4);
      pCmd+=10;
      Osd_Flag&=~OSD_FG_CMDSEND;
      ret=true;
      break;
    }
  }
  JAPI_CloseRfVideo();
  return ret;
}

