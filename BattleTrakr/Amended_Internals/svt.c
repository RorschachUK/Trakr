#include "svt.h"
#include "JAPI.h"
#include "Audio.h"
#include "Image0.o.h"
#include "Image1.o.h"
#include "Image2.o.h"
#include "Image3.o.h"

#define LOGGING 0

extern int kprintf(char *buf, const char *fmt, va_list args);
extern int Log_Handle;
extern char Log_Buffer[ 256 ];

void Log(String text, ... )
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

void Sleep( uint32 us )
{
  JAPI_Sleep( us / 6 );
}

void ResetTimer( void )
{
  JAPI_ResetTimer2();
}

uint16 ReadTimer( void )
{
  return JAPI_GetTimer2();
}

// IR
void OpenIR()
{
  JAPI_GetIRCtrl();
}

void SetIR( bool state )
{
  if ( state )
	JAPI_IRon( );
  else
	JAPI_IRoff( );
}

void SetLED( bool state )
{
  if (state)
    (*((volatile unsigned long *)0xFFF84008))&=~0x400;                 //       LED on
  else
	(*((volatile unsigned long *)0xFFF84008))|=0x400;            //       LED off
}

void CloseIR()
{
  JAPI_QuitIRCtrl( );
}

// Get Voltage / Current
int GetBatteryVoltage()
{
	// To Do - get the correct constant
	return ( JAPI_ReadBatteryVoltage()  ) / 8 ;
}

bool GetRemoteKeyStatus( int key )
{
  return ( ( JAPI_GetCtrlStatus() >> 8 ) & key ) != 0;
}

int GetRemoteKeys()
{
  return JAPI_GetCtrlStatus() >> 8;
}

bool GetCarPushButton()
{
  // Log( "Car Button %ld", JAPI_GetKey() );
  return ( JAPI_GetKey() != 0 );
}

extern int Motor_State;

// Motor control
void OpenMotors()
{
  JAPI_MotorOpen();
  JAPI_SetMotorLeft( 0 );
  JAPI_SetMotorRight( 0 );
}

void SetLeftMotor( int speed )
{
  JAPI_SetMotorLeft( speed );
}

void SetRightMotor( int speed )
{
  JAPI_SetMotorRight( speed );
}

void SetMotors( int leftSpeed, int rightSpeed )
{
  JAPI_SetMotorLeft( leftSpeed );
  JAPI_SetMotorRight( rightSpeed );
}

void CloseMotors()
{
  SetMotors( 0, 0 );
  JAPI_MotorClose();
}

uint16 GetLeftMotorCurrent()
{
  return JAPI_ReadADC( 2 );
}

uint16 GetRightMotorCurrent()
{
  return JAPI_ReadADC( 3 );
}

// File IO
#define JAPI_GetWchar(a) SVT_Char_To_WChar(a)

long OpenFileSystem()
{
  return JAPI_FS_Open();
}

void CloseFileSystem()
{
  JAPI_FS_Close();
}

long CreateFile( String pathName )
{
  unsigned short* wName = JAPI_GetWchar( pathName );
  JAPI_FS_FileCreate( 0, wName );
  return JAPI_FS_FileOpen( wName );
}

long DeleteFile( String pathName )
{
  return JAPI_FS_FileDelete( JAPI_GetWchar( pathName ) );
}

File OpenFile( String filename )
{
  return JAPI_FS_FileOpen( JAPI_GetWchar( filename ) );
}

void CloseFile( File f )
{
  JAPI_FS_FileClose( f );
}

long SeekFile( File f, long pos)
{
  return JAPI_FS_FileSeek( f, pos );
}

long ReadFile( File f, void* buffer, uint32 bufferLength )
{
  long bytesRead = 0;
  JAPI_FS_FileRead( f, buffer, bufferLength, &bytesRead );
  return bytesRead;
}

long WriteFile( File f, void* buffer, uint32 bufferLength )
{
  long bytesWritten = 0;
  JAPI_FS_FileWrite( f, buffer, bufferLength, &bytesWritten );
  return bytesWritten;
}

long FlushFile( File f )
{
  return JAPI_FS_FileFlush( f );
}


// Graphics

void OpenGraphics()
{
	SVT_ROsd_Open();
}

void CloseGraphics()
{
	SVT_ROsd_Close();
}

void OpenImageRegister( void )
{
  SVT_ROsd_Pack_Bmps_Open();
}

int RegisterImage( void* image, int size)
{
  return SVT_ROsd_Pack_Bmps( (char *)image, (unsigned int)size );
}

void CloseImageRegister( void )
{
  SVT_ROsd_Pack_Bmps_Send();
  SVT_ROsd_Pack_Bmps_Close();
}

void ScrollRectangle(int lx, int ty, int rx, int by, int mx, int my)
{
   SVT_ROsd_ScrollRectangle( lx, ty, rx, by, mx, my);
}

void DrawRectangle( int lx, int ty, int rx, int by, Color rgba )
{
  SVT_ROsd_Draw_Rectangle( lx, ty, rx, by, (TAG_COLOR*)&rgba );
}

void SetLineWidth( int w )
{
  SVT_ROsd_Set_Linewidth( w );
}

void ClearRectangle( int lx, int ty, int rx, int by )
{
  SVT_ROsd_Clear_Rectangle( lx, ty, rx, by );
}

char DTBuffer[ 256 ];

int DrawText(int x, int y, char * fmt,...)
{
  va_list argp;
  va_start(argp, fmt );
  kprintf( DTBuffer, fmt, argp );

  return SVT_ROsd_Draw_Text( x, y, DTBuffer );
}

void SetTextColor( Color rgba )
{
  SVT_ROsd_Set_Text_Color( (TAG_COLOR*)&rgba );
}

int Image0 = -1;
int Image1 = -1;
int Image2 = -1;
int Image3 = -1;

void RegisterDefaultImages( void )
{
	OpenImageRegister( );
	if (LOGGING) Log( "RDI: s-e %d size %d", _binary_Images_Image0_bmp_end - _binary_Images_Image0_bmp_start, _binary_Images_Image0_bmp_size );
	Image0 = RegisterImage( _binary_Images_Image0_bmp_start, _binary_Images_Image0_bmp_end - _binary_Images_Image0_bmp_start );
	Image1 = RegisterImage( _binary_Images_Image1_bmp_start, _binary_Images_Image1_bmp_end - _binary_Images_Image1_bmp_start );
	Image2 = RegisterImage( _binary_Images_Image2_bmp_start, _binary_Images_Image2_bmp_end - _binary_Images_Image2_bmp_start );
	Image3 = RegisterImage( _binary_Images_Image3_bmp_start, _binary_Images_Image3_bmp_end - _binary_Images_Image3_bmp_start );
	CloseImageRegister( );
}

void DrawImage( int imageIndex, int x, int y, Color transparent )
{
  SVT_ROsd_Draw_Bitmap( imageIndex, x, y, (TAG_COLOR*)&transparent );
}

void SetScreen( Color rgba )
{
  SVT_ROsd_Set_Screen( (TAG_COLOR*)&rgba );
}

void ClearScreen( void )
{
  SVT_ROsd_Clear_Screen( );
}

void SetRectangle( int lx, int ty, int rx, int by, Color rgba )
{
  SVT_ROsd_Draw_Rectangle( lx, ty, rx, by, (TAG_COLOR*)&rgba );
}

void Show(void)
{
  SVT_ROsd_Show();
  if(SVT_ROsd_Is_Cmd_Sent()==false)
  {
	if ( SVT_ROsd_Send_Cmd_Times( 20 ) != true ) {
      if (LOGGING) SVT_Log( "  Frame send error - not successful in 10 attempts" );
      SVT_ROsd_Open();
	}
    return;
  }
}


int ReadADC( int channel )
{
	return JAPI_ReadADC( channel );
}

#define WAV_BUFFER_SIZE       (300*1024)
unsigned char Wave_Buffer[WAV_BUFFER_SIZE];
TAG_HANDLE  Wave;

bool StartAudioPlayback( char* filename )
{
  Wave.pRamBuffer=Wave_Buffer;
  Wave.RamMax=WAV_BUFFER_SIZE;
  return SVT_WAV_Play( filename, &Wave );
}

bool IsAudioPlaying()
{
  return SVT_WAV_Is_Busy( &Wave );
}

bool StartAudioRecording( char* filename )
{
  Wave.pRamBuffer=Wave_Buffer;
  Wave.RamMax=WAV_BUFFER_SIZE;
  return SVT_WAV_Record( filename, &Wave ) ;
}

bool WriteAudioData()
{
  return SVT_WAV_Recording( &Wave );
}

void StopAudioRecording()
{
  SVT_WAV_Stop( &Wave );
}
