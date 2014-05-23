#include "Audio.h"
#include "Trakr.h"

#pragma pack(1)

#define LOGGING 0

typedef struct{
  unsigned int id;
  unsigned int size;
}sCHUNK;

typedef struct
{
  char cRiffID[4]; // 'R','I','F','F'
  unsigned int dwRiffSize;
  char cRiffFormat[4]; // 'W','A','V','E'
}sRIFF_HEADER;


typedef struct
{
  char szFmtID[4]; // 'f','m','t',' '
  unsigned int dwFmtSize;
  sWAVE_FORMAT sWavFormat;
}sFMT_BLOCK;

typedef struct
{
  char szFactID[4]; // 'f','a','c','t'
  unsigned int dwFactSize;
  unsigned int nSamples;
}sFACT_BLOCK;

typedef struct
{
  unsigned char ucDataID[4]; // 'd','a','t','a'
  unsigned int DataSize;
}sDATA_BLOCK;

#pragma pack(4)

typedef	void (*PFNSEEK)(TAG_HANDLE* pWave, unsigned int Offset);
typedef void (*PFNREAD)(TAG_HANDLE* pWave,
              unsigned char* pBuffer,
              unsigned int Length,
              unsigned long *ReadLength);
typedef void (*PFNWRITE)(TAG_HANDLE* pSrc,
               unsigned char* pBuffer,
               unsigned int Length,
               unsigned long *WriteLength);
typedef void (*PFNCLOSE)(TAG_HANDLE* pSrc);
PFNSEEK pfnSeek;
PFNREAD pfnRead;
PFNWRITE pfnWrite;
PFNCLOSE pfnClose;

TAG_HANDLE *pAudio;

unsigned int WaveHeadBuffer[64];

#define AUDIO_BUFFER_SIZE (1024*4)
unsigned char Audio_Buffer[AUDIO_BUFFER_SIZE*2]__attribute__((aligned(16)));

extern void WAV_Seek(TAG_HANDLE* pWave, unsigned int Offset);
extern void WAV_Read(TAG_HANDLE* pWave,
              unsigned char* pBuffer,
              unsigned int Length,
              unsigned long *ReadLength);
extern void WAV_Write(TAG_HANDLE* pSrc,
               unsigned char* pBuffer,
               unsigned int Length,
               unsigned long *WriteLength);
extern void WAV_Close(TAG_HANDLE* pWave);

extern void Buffer_Seek(TAG_HANDLE* pWave, unsigned int Offset);
extern void Buffer_Read(TAG_HANDLE* pWave,
                 unsigned char* pBuffer,
                 unsigned int Length,
                 unsigned long *ReadLength);
extern void Buffer_Write(TAG_HANDLE* pWave,
                  unsigned char* pBuffer,
                  unsigned int Length,
                  unsigned long *WriteLength);
extern void Buffer_Close(TAG_HANDLE* pWave);
extern void Copy_Data_To_Buffer(unsigned char Index);
extern void Wav_Read_ReadData(TAG_HANDLE *pWave,char *puc, unsigned int Len,unsigned long *pLen);
extern bool Wav_Write_Finish(TAG_HANDLE *pWave);


bool SVT_WAV_Play(const char* pFileName, TAG_HANDLE* pWave)
{
  unsigned int Len,LenRead,ReadPos;
  unsigned int *puc;
  sWAVE_FORMAT *sFmt;
  pAudio=pWave;
  pWave->flag&=~0x2;
  pWave->flag|=0x1;
  ReadPos=0;


  if (LOGGING) SVT_Log( "PLAYBACK TIME");

  if((SVT_Str_Cmp((char*)pFileName,pWave->FileName)==true)&&(pWave->pRamBuffer!=0))
  {
	if (LOGGING) SVT_Log( "  Using Buffer" );
    pfnSeek=Buffer_Seek;
    pfnRead=Buffer_Read;
    pfnClose=Buffer_Close;
  }
  else
  {
    Len=SVT_Str_Length(pFileName)+1;
    if(Len>256)
      return false;
    pfnSeek=WAV_Seek;
    pfnRead=WAV_Read;
    pfnClose=WAV_Close;
//    if(JAPI_FS_Open()<0)
//      return false;
    JAPI_FS_Open();
    pWave->FileHandle=JAPI_FS_FileOpen(SVT_Char_To_WChar(pFileName));
    if(pWave->FileHandle<0)
    {
      JAPI_FS_Close();
      return false;
    }
    if(LOGGING)SVT_Log( "  File Opened %s", pFileName );
    SVT_Memory_Copy(pWave->FileName,pFileName,Len);
  }
  puc=WaveHeadBuffer;
  Len=sizeof(sRIFF_HEADER);
  pfnSeek(pWave,ReadPos);
  pfnRead(pWave,(char*)puc,Len,(unsigned long*)&LenRead);
  if(Len!=LenRead)
    return false;
  if(*puc!=0x46464952)
  {
    return false;
  }
  pWave->FileLen=((sRIFF_HEADER*)puc)->dwRiffSize;
  pWave->BufferCount=pWave->FileLen+8;
  pWave->FileSize=pWave->FileLen+8;
  ReadPos+=Len;

  puc=WaveHeadBuffer;
  Len=sizeof(sFMT_BLOCK);
  pfnSeek(pWave,ReadPos);
  pfnRead(pWave,(char*)puc,Len,(unsigned long*)&LenRead);
  if(Len!=LenRead)
    return false;
  if(*puc!=0x20746d66)
  {
    return false;
  }
  if(*(puc+1)!=0x10)
  {
    return false;
  }
  sFmt=(sWAVE_FORMAT*)&(((sFMT_BLOCK*)puc)->sWavFormat);
  if(sFmt->wFormatTag!=1)
  {
    return false;
  }
  pWave->sFmt.wFormatTag=sFmt->wFormatTag;
  pWave->sFmt.wChannels=sFmt->wChannels;
  pWave->sFmt.dwSamplesPerSec=sFmt->dwSamplesPerSec;
  pWave->sFmt.dwAvgBytesPerSec=sFmt->dwAvgBytesPerSec;
  pWave->sFmt.wBlockAlign=sFmt->wBlockAlign;
  pWave->sFmt.wBitsPerSample=sFmt->wBitsPerSample;
  ReadPos+=Len;

  puc=WaveHeadBuffer;
  Len=sizeof(sFACT_BLOCK);
  pfnSeek(pWave,ReadPos);
  pfnRead(pWave,(char*)puc,Len,(unsigned long*)&LenRead);
  if(Len!=LenRead)
    return false;
  pWave->FileHeaderLen=44;
  if(*puc==0x74636166)
  {
    ReadPos+=Len;
    pfnSeek(pWave,ReadPos);
    pfnRead(pWave,(char*)puc,Len,(unsigned long*)&LenRead);
    if(LenRead!=Len)
      return false;
    pWave->FileHeaderLen=56;
  }
  if(*puc!=0x61746164)
  {
    return false;
  }
  ReadPos+=sizeof(sDATA_BLOCK);
  pWave->FilePos=ReadPos;
  pWave->DataPos=0;
  pWave->DataLen=((sDATA_BLOCK*)puc)->DataSize;
  pfnSeek(pWave,pWave->FileHeaderLen);
  Apu_Stop();
  Apu_Open(pWave->sFmt.dwSamplesPerSec,pWave->sFmt.wChannels);
  Apu_Register_Fetch_Data(((pfnFillBuffer)Copy_Data_To_Buffer));
  Apu_Set_Buffer((unsigned int)&Audio_Buffer[0]);
  ApuStopIndex=0;
  Copy_Data_To_Buffer(0);
  Copy_Data_To_Buffer(1);
  Apu_Start();
  return true;
}
void Copy_Data_To_Buffer(unsigned char Index)
{
  signed int Len;
  signed int Len2;
  signed int Len3;
  Len=(signed int)(pAudio->DataLen);
  Len2=(signed int)(pAudio->DataPos);
  Len2=Len-Len2;
  Len3=(signed int)AUDIO_BUFFER_SIZE;
  if(Len3<Len2)
  {
    Len=Len3;
    Wav_Read_ReadData(pAudio,
            (unsigned char *)&Audio_Buffer[Index*AUDIO_BUFFER_SIZE],
            Len,
            (unsigned long *)&Len);
    Apu_Set_Buffer_Size(Index,Len);
  }
  else
  {
    Len=Len2;
    SVT_Memory_Set((unsigned char *)&Audio_Buffer[Index*AUDIO_BUFFER_SIZE],0,Len3);
    Wav_Read_ReadData(pAudio,
            (unsigned char *)&Audio_Buffer[Index*AUDIO_BUFFER_SIZE],
            Len,
            (unsigned long *)&Len);
    Len=Len>Len3/2?Len:Len3/2;
    Apu_Set_Buffer_Size(Index,Len);
    ApuStopIndex=Index+1;
  }
}
bool SVT_WAV_Is_Busy(TAG_HANDLE *Handle)
{
  if(Apu_Is_Busy())
  {
    return true;
  }
  else
  {
    pfnClose(Handle);
    return false;
  }
}
void SVT_WAV_Stop(TAG_HANDLE *Handle)
{
  if(Handle->flag&0x1)
  {
    Apu_Stop();
    Apu_Close();
    pfnClose(Handle);
  }
  if(Handle->flag&0x2)
  {
    Wav_Write_Finish(Handle);
  }
  Handle->flag=0;
  return;
}
bool SVT_WAV_Record(const char * pFileName, TAG_HANDLE *pWave)
{
  unsigned int *puc;
  unsigned int Len;
  unsigned int ReadPos;
  bool IsBuffer;
  bool IsSD;
  sWAVE_FORMAT *fmt;
  pAudio=pWave;
  pWave->flag&=~0x1;
  pWave->flag|=0x2;
  fmt=&(pWave->sFmt);

  fmt->wFormatTag=1;
  fmt->wChannels=1;
  fmt->dwSamplesPerSec=8000;
  fmt->wBitsPerSample=16;

  fmt->dwAvgBytesPerSec=fmt->wChannels*fmt->dwSamplesPerSec*fmt->wBitsPerSample/8;
  fmt->wBlockAlign=fmt->wChannels*fmt->wBitsPerSample/8;
  if(SVT_Str_Cmp(pFileName,pWave->FileName)!=true)
  {
    Len=SVT_Str_Length(pFileName)+1;
    if(Len>256)return false;
    SVT_Memory_Copy(pWave->FileName,pFileName,Len);
  }
  if(pWave->pRamBuffer!=0)
  {
    IsBuffer=true;
  }
  else
  {
    IsBuffer=false;
    pWave->RamMax=0xfffffff;
  }
/*
  if(JAPI_FS_Open()<0)
  {
	SVT_Log(   "  WAVE RECORD: FS_OPEN FAILED" );
    IsSD=false;
    if(IsBuffer==false)return false;
    else
    {
      pfnSeek=Buffer_Seek;
      pfnWrite=Buffer_Write;
      pfnClose=Buffer_Close;
    }
  }
  else
  */
  JAPI_FS_Open();
  {
    if (LOGGING) SVT_Log(   "  WAVE RECORD: FS_OPEN SUCCEEDED" );
	if (LOGGING) SVT_Log(   "  WAVE RECORD: Deleting %s", pFileName );
    JAPI_FS_FileDelete(SVT_Char_To_WChar(pFileName));
	if (LOGGING) SVT_Log(   "  WAVE RECORD: Opening %s", pFileName );
    if(JAPI_FS_FileCreate(0,SVT_Char_To_WChar(pFileName))<0)
    {
      if (LOGGING) SVT_Log(   "  WAVE RECORD: File Created" );
      IsSD=false;
      if(IsBuffer==false)return false;
      else
      {
        pfnSeek=Buffer_Seek;
        pfnWrite=Buffer_Write;
        pfnClose=Buffer_Close;
      }
    }
    else
    {
      IsSD=true;
      pWave->FileHandle=JAPI_FS_FileOpen(SVT_Char_To_WChar(pFileName));
      if(pWave->FileHandle<0)
      {
        if (LOGGING) SVT_Log(   "  WAVE RECORD: File Not Opened %d", pWave->FileHandle );

        IsSD=false;
        if(IsBuffer==false)return false;
        else
        {
          pfnSeek=Buffer_Seek;
          pfnWrite=Buffer_Write;
          pfnClose=Buffer_Close;
        }
      }
      else
      {
        if (LOGGING) SVT_Log(   "  WAVE RECORD: File Opened" );
        pfnSeek=WAV_Seek;
        pfnWrite=WAV_Write;
        pfnClose=WAV_Close;
      }
    }
  }
  ReadPos=0;
  //RIFF
  puc=WaveHeadBuffer;
  Len=sizeof(sRIFF_HEADER);
  //puc=(unsigned char *)malloc(Len);
  //if(puc==0)
  //  return false;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)puc,Len,(unsigned long*)&Len);
  //free(puc);
  puc=WaveHeadBuffer;
  ReadPos+=Len;
  //FMT
  Len=sizeof(sFMT_BLOCK);
  //puc=(unsigned char *)malloc(Len);
  //if(puc==0)
  //  return false;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)puc,Len,(unsigned long*)&Len);
  //free(puc);
  puc=WaveHeadBuffer;
  ReadPos+=Len;
  //Data
  Len=sizeof(sDATA_BLOCK);
  //puc=(unsigned char *)malloc(Len);
  //if(puc==0)
  //  return false;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)puc,Len,(unsigned long*)&Len);
  //free(puc);
  ReadPos+=Len;
  pWave->FilePos=ReadPos;
  pWave->DataPos=0;
  pWave->DataLen=0;
  pWave->FileHeaderLen=44;
  Adc_Start();
  return true;
}
bool SVT_WAV_Recording(TAG_HANDLE *Handle)
{
  unsigned int WriteSize=0;
  if(Adc_Is_Fill_Data())
  {
    if (LOGGING) SVT_Log(   "  WAVE RECORD: ADC full... Writing (@%d)", SVT_GetTimer() );
    pfnWrite(Handle,(unsigned char *)Adc_Get_Recorded_Data_Address(),AdcBufferSize,(unsigned long*)&WriteSize);
    Handle->DataPos+=WriteSize;
    Handle->DataLen+=WriteSize;
    if(WriteSize>=Handle->RamMax)
    {
      Wav_Write_Finish(Handle);
      return false;
    }
  }
  return true;
}

bool Wav_Write_Finish(TAG_HANDLE *pWave)
{
  unsigned int* puc;
  unsigned int Len;
  unsigned int LenBuf;
  unsigned int ReadPos;
  ReadPos=0;
  Len=sizeof(sRIFF_HEADER);
  puc=WaveHeadBuffer;
  *puc++=0x46464952;
  *puc++=pWave->DataLen+pWave->FileHeaderLen-8;
  *puc++=0x45564157;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)WaveHeadBuffer,Len,(unsigned long*)&LenBuf);
  if(LenBuf!=Len)
  {
    pfnClose(pWave);
    return false;
  }
  puc=WaveHeadBuffer;
  ReadPos+=Len;
  Len=sizeof(sFMT_BLOCK);
  *puc++=0x20746d66;
  *puc++=0x10;
  *puc++=pWave->sFmt.wChannels*0x10000+pWave->sFmt.wFormatTag;
  *puc++=pWave->sFmt.dwSamplesPerSec;
  *puc++=pWave->sFmt.dwAvgBytesPerSec;
  *puc++=pWave->sFmt.wBitsPerSample*0x10000+pWave->sFmt.wBlockAlign;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)WaveHeadBuffer,Len,(unsigned long*)&LenBuf);
  if(LenBuf!=Len)
  {
    pfnClose(pWave);
    return false;
  }
  ReadPos+=Len;
  puc=WaveHeadBuffer;
  Len=sizeof(sDATA_BLOCK);
  *puc++=0x61746164;
  *puc++=pWave->DataLen;
  pWave->BufferCount=pWave->DataLen+pWave->FileHeaderLen;
  pfnSeek(pWave,ReadPos);
  pfnWrite(pWave,(char*)WaveHeadBuffer,Len,(unsigned long*)&LenBuf);
  if(LenBuf!=Len)
  {
    pfnClose(pWave);
    return false;
  }
  ReadPos+=Len;
  pfnClose(pWave);
  return true;
}

void WAV_Seek(TAG_HANDLE* pWave, unsigned int Offset)
{
  JAPI_FS_FileSeek(pWave->FileHandle,Offset);
  if(pWave->pRamBuffer)
  {
    pWave->BufferOffset=Offset<pWave->RamMax?Offset:pWave->RamMax;
  }
}
void WAV_Read(TAG_HANDLE* pWave,
              unsigned char* pBuffer,
              unsigned int Length,
              unsigned long *ReadLength)
{
  JAPI_FS_FileRead(pWave->FileHandle,pBuffer,Length,ReadLength);
  if(pWave->pRamBuffer)
  {
    Buffer_Write(pWave,pBuffer,Length,ReadLength);
  }
}
void WAV_Write(TAG_HANDLE* pWave,
               unsigned char* pBuffer,
               unsigned int Length,
               unsigned long *WriteLength)
{
  JAPI_FS_FileWrite(pWave->FileHandle,pBuffer,Length,WriteLength);
  if(pWave->pRamBuffer)
  {
    Buffer_Write(pWave,pBuffer,Length,WriteLength);
  }
}
void WAV_Close(TAG_HANDLE* pWave)
{
  if(pWave->FileHandle>=0)
  {
    JAPI_FS_FileClose(pWave->FileHandle);
    // DW - maybe not close?  Maybe other files open?
    //JAPI_FS_Close();
  }
}


void Buffer_Seek(TAG_HANDLE* pWave, unsigned int Offset)
{
  pWave->BufferOffset=Offset<pWave->BufferCount?Offset:pWave->BufferCount;
}
void Buffer_Read(TAG_HANDLE* pWave,
                 unsigned char* pBuffer,
                 unsigned int Length,
                 unsigned long *ReadLength)
{
  unsigned char *puc;
  puc=(unsigned char*)(pWave->pRamBuffer+pWave->BufferOffset);
  if((pWave->BufferOffset+Length)>pWave->BufferCount)
  {
    Length=pWave->BufferCount-pWave->BufferOffset;
  }
  pWave->BufferOffset+=Length;
  *ReadLength=Length;
  SVT_Memory_Copy(pBuffer,puc,Length);
}
void Buffer_Write(TAG_HANDLE* pWave,
                  unsigned char* pBuffer,
                  unsigned int Length,
                  unsigned long *WriteLength)
{
  unsigned char *puc;
  puc=(unsigned char*)(pWave->pRamBuffer+pWave->BufferOffset);
  Length=(pWave->BufferOffset+Length)<pWave->RamMax?
         Length:
         pWave->RamMax-pWave->BufferOffset;
  pWave->BufferOffset+=Length;
  pWave->BufferCount=pWave->BufferCount>pWave->BufferOffset?
                     pWave->BufferCount:
                     pWave->BufferOffset;
  *WriteLength=Length;
  SVT_Memory_Copy(puc,pBuffer,Length);
}
void Buffer_Close(TAG_HANDLE* pWave)
{
  return;
}

void Wav_Read_ReadData(TAG_HANDLE *pWave,char *puc, unsigned int Len,unsigned long *pLen)
{
  pfnRead(pWave,puc,Len,pLen);
  pWave->DataPos+=*pLen;
  pWave->DataPos=pWave->DataPos<pWave->DataLen?
                 pWave->DataPos:
                 pWave->DataLen;
}


