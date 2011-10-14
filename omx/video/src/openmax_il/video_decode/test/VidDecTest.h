
/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* ============================================================================ */
/**
* @file VidDecTest.h
*
* This is an header file for an video Mpeg4 decoder that is fully
* compliant with the OMX Video specification.
* This the file that the application that uses OMX would include
* in its code.
*
* @path $(CSLPATH)\
*
* @rev 0.1
*/
/* --------------------------------------------------------------------------- */

#ifndef VIDDECTEST_H
#define VIDDECTEST_H

    #define _XOPEN_SOURCE 600
    #include <sys/select.h>
    #include <signal.h>
    #include <unistd.h>     /* for sleep */

#define __GET_BC_VOP__
/*#define __GET_BC_VOP_ADVANCE__*/
#define ADDED_TESTCASES

#ifndef VIDDEC_SPARK_CODE
    #define VIDDEC_SPARK_CODE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <sched.h>
#include <sys/select.h>
#include <OMX_Component.h>
#include <signal.h>
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_Types.h>
#include <OMX_Component.h>
#include <semaphore.h>
#include <OMX_VideoDecoder.h>
#include "overlay_common.h"

#define KHRONOS_1_1
/*#define KHRONOS_1_2*/
/*#define DSP_MMU_FAULT_HANDLING 1*/
#ifndef KHRONOS_1_2
 #define OMX_BUFFERFLAG_CODECCONFIG 0x00000080
#endif
#define CMD_FILL                            1
#define CMD_EMPTY                           2
#define ADD_CHARS_TO_OUTPUT                 6
#define MAX_VIDDEC_NUM_OF_IN_BUFFERS        NUM_OVERLAY_BUFFERS_REQUESTED
#define MAX_VIDDEC_NUM_OF_OUT_BUFFERS       NUM_OVERLAY_BUFFERS_REQUESTED
#define VIDDECTEST_BUFFERSIZE_FIXED         3000
#define CHUNK_FILE                          3000
/*#define VIDDECTEST_USEFORMATSIZE_RANDOMIZED*/
/*#define VIDDECTEST_USEFORMATSIZE*//* do not define stop-restart issues*/
#ifdef VIDDECTEST_USEFORMATSIZE
 #define VIDDECTEST_BUFFERSIZE_SPLITTER     10
#endif
#ifdef VIDDECTEST_USEFORMATSIZE_RANDOMIZED
 #define VIDDECTEST_BUFFERSIZE_SPLITTER_MIN (VIDDECTEST_BUFFERSIZE_SPLITTER / 2) - 1
 #define VIDDECTEST_BUFFERSIZE_SPLITTER_MAX VIDDECTEST_BUFFERSIZE_SPLITTER + \
                                            ((VIDDECTEST_BUFFERSIZE_SPLITTER / 2))
#endif
#define VIDDECTEST_MODULEVALUE              5
#define VAL_ZERO                            0
#define VAL_ONE                             1
#define VAL_MINUS                           -1
#define VAL_WMVHEADER                       20
#define APP_TIMEOUT                         20
#define MAX_TEXT_STRING                     210

#define VERSION_MAJOR                       1
#ifdef KHRONOS_1_1
#define VERSION_MINOR                       0
#else
#define VERSION_MINOR                       0
#endif
#define VERSION_REVISION                    0
#define VERSION_STEP                        0

#define BUFFERMINCOUNT                      VAL_ONE
#define PORTENABLED                         OMX_TRUE
#define PORTPOPULATED                       OMX_FALSE
#define PORTDOMAIN                          OMX_PortDomainVideo

#define MIMETYPEH263                        "H263"
#define MIMETYPEH264                        "H264"
#define MIMETYPEMPEG4                       "MPEG4"
#define MIMETYPEMPEG2                       "MPEG2"
#define MIMETYPEWMV                         "WMV"
#define MIMETYPEYUV                         "YUV"
#ifdef VIDDEC_SPARK_CODE
    #define MIMETYPESPARK                     "SPARK"
#endif

#ifdef KHRONOS_1_1
/*#define TEST_ROLES*/
#define COMPONENTROLES_H263                 "video_decoder.h263"
#define COMPONENTROLES_H264                 "video_decoder.h264"
#define COMPONENTROLES_MPEG2                "video_decoder.mpeg2"
#define COMPONENTROLES_MPEG4                "video_decoder.mpeg4"
#define COMPONENTROLES_WMV9                 "video_decoder.wmv"
#define COMPONENTROLES_NUMBER               6
#ifdef VIDDEC_SPARK_CODE
#define COMPONENTROLES_SPARK                 "video_decoder.spark"
#endif
#endif

#define PORTCOMPRESSIONFORMATH263           OMX_VIDEO_CodingH263
#define PORTCOMPRESSIONFORMATH264           OMX_VIDEO_CodingAVC
#define PORTCOMPRESSIONFORMATMPEG4          OMX_VIDEO_CodingMPEG4
#define PORTCOMPRESSIONFORMATMPEG2          OMX_VIDEO_CodingMPEG2
#define PORTCOMPRESSIONFORMATWMV            OMX_VIDEO_CodingWMV
#define PORTCOMPRESSIONFORMATOUTPUT         OMX_VIDEO_CodingUnused
#define PORTCOMPRESSIONFORMATUNUSED         OMX_VIDEO_CodingUnused
#ifdef VIDDEC_SPARK_CODE
#define PORTCOMPRESSIONFORMATSPARK          OMX_VIDEO_CodingUnused
#endif

#define INPORTNATIVERENDER                  NULL
#define INPORTSTRIDE                        VAL_MINUS
#define INPORTSLICEHEIGHT                   VAL_MINUS
#define INPORTBITRATE                       VAL_MINUS
#define INPORTFRAMERATE                     VAL_MINUS
#define INPORTFLAGERRORCONCEALMENT          OMX_FALSE

#define OUTPORTNATIVERENDER                 NULL
#define OUTPORTSTRIDE                       VAL_ZERO
#define OUTPORTSLICEHEIGHT                  VAL_ZERO
#define OUTPORTBITRATE                      VAL_ZERO
#define OUTPORTFRAMERATE                    VAL_ZERO
#define OUTPORTFLAGERRORCONCEALMENT         OMX_FALSE

#define COLORFORMAT422                      OMX_COLOR_FormatCbYCrY
#define COLORFORMAT420                      OMX_COLOR_FormatYUV420Planar /*Not handles in Andorid: OMX_COLOR_FormatYUV420PackedPlanar*/

#define FACTORFORMAT422                     2
#define FACTORFORMAT420                     1.5

#define PAUSERESUMEFRAME                    25
#define STOPRESUMEFRAME                     25
#define FLUSHINGFRAME                       15
#define FRAMECOUNTMULT                      4
#define FRAMECOUNTFLUSH                     20
#define SLEEPTIME                           1
#define FRAMECOUNTEOS                       20

#define WMV_ELEMSTREAM                      0
#define WMV_RCVSTREAM                       1
#define WMV_STARCODE_LENGHT                 4

#define OMX_VIDDEC_1_1
#ifdef OMX_VIDDEC_1_1
    #define OMX_BUFFERFLAG_SYNCFRAME                0x00000020
#endif

typedef enum {
    I_FRAME =0,
    P_FRAME,
    B_FRAME,
    IDR_FRAME
} FrameType;

#define FRAMETYPE_MASK                              0xF0000000
#define FRAMETYPE_I_FRAME                           0x10000000
#define FRAMETYPE_P_FRAME                           0x20000000
#define FRAMETYPE_B_FRAME                           0x40000000
#define FRAMETYPE_IDR_FRAME                         0x80000000

#define BUFFERFLAG_EXTENDERROR_MASK                 0x0FFFF000
#define BUFFERFLAG_EXTENDERROR_DIRTY                0x000FF000
#define BUFFERFLAG_EXTENDERROR_APPLIEDCONCEALMENT   0x00200000
#define BUFFERFLAG_EXTENDERROR_INSUFFICIENTDATA     0x00400000
#define BUFFERFLAG_EXTENDERROR_CORRUPTEDDATA        0x00800000
#define BUFFERFLAG_EXTENDERROR_CORRUPTEDHEADER      0x01000000
#define BUFFERFLAG_EXTENDERROR_UNSUPPORTEDINPUT     0x02000000
#define BUFFERFLAG_EXTENDERROR_UNSUPPORTEDPARAM     0x04000000
#define BUFFERFLAG_EXTENDERROR_FATALERROR           0x08000000

/*#define MEASURE_TIME*/

#define StrVideoDecoder "OMX.TI.Video.Decoder"
#define ENV_TICKCOUNTPRINT_NAME "PRINTTICKCOUNT"
typedef enum VIDDECTEST_WMV_PROFILES
{
    VIDDECTEST_WMV_PROFILE0,
    VIDDECTEST_WMV_PROFILE1,
    VIDDECTEST_WMV_PROFILE2,
    VIDDECTEST_WMV_PROFILE3,
    VIDDECTEST_WMV_PROFILE4,
    VIDDECTEST_WMV_PROFILE5,
    VIDDECTEST_WMV_PROFILE6,
    VIDDECTEST_WMV_PROFILE7,
    VIDDECTEST_WMV_PROFILE8,
    VIDDECTEST_WMV_PROFILEMAX
}VIDDECTEST_WMV_PROFILES;

typedef enum PROCESS_MODE_TYPE
{
    PROCESS_MODE_TYPE_FRAME = 0 ,
    PROCESS_MODE_TYPE_STREAM = 1 ,
    PROCESS_MODE_TYPE_MAX = PROCESS_MODE_TYPE_STREAM
}PROCESS_MODE_TYPE;

typedef enum VIDDECTEST_H264FILETYPE
{
    VIDDECTEST_H264FILETYPE_BC = 0,
    VIDDECTEST_H264FILETYPE_VOP = 1,
    VIDDECTEST_H264FILETYPE_NAL = 2,
    VIDDECTEST_H264FILETYPE_NAL_TWO_BE = 3,
    VIDDECTEST_H264FILETYPE_NAL_TWO_LE = 4,
    VIDDECTEST_H264FILETYPE_MAX = VIDDECTEST_H264FILETYPE_NAL_TWO_LE
}VIDDECTEST_H264FILETYPE;

typedef enum TESTCASE_TYPE
{
    TESTCASE_TYPE_PLAYBACK = 0,
    TESTCASE_TYPE_PAUSERESUME = 1,
    TESTCASE_TYPE_STOPRESUME = 2,
    TESTCASE_TYPE_REPETITION = 3,
    TESTCASE_TYPE_FASTFORWARD = 4,
    TESTCASE_TYPE_REWIND = 5,
    TESTCASE_TYPE_SEEK = 6,
    TESTCASE_TYPE_PROP_TIMESTAMPS = 7,
    TESTCASE_TYPE_PROP_TICKCOUNT = 8,
    TESTCASE_TYPE_FLUSHING = 9,
    TESTCASE_TYPE_VOPMILISECOND = 10,
    TESTCASE_TYPE_MBERRORCHECK = 11,
    TESTCASE_TYPE_PLAYAFTEREOS = 12,
    TESTCASE_TYPE_DERINGINGENABLE = 13,
    TESTCASE_TYPE_MAX = TESTCASE_TYPE_DERINGINGENABLE
}TESTCASE_TYPE;

#define TESTCASE_TYPE_UNUSED TESTCASE_TYPE_FLUSHING
typedef enum VIDDECTEST_FILLDATA_TYPE
{
    VIDDECTEST_FILLDATA_TYPE_STREAM = 0,
    VIDDECTEST_FILLDATA_TYPE_FRAME_BC,
    VIDDECTEST_FILLDATA_TYPE_FRAME_VOP,
    VIDDECTEST_FILLDATA_TYPE_FRAME_NAL,
    VIDDECTEST_FILLDATA_TYPE_STREAM_NAL,
    VIDDECTEST_FILLDATA_TYPE_FRAME_RCV,
    VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_BE,
    VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_LE,
    VIDDECTEST_FILLDATA_TYPE_MAX = VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_LE
}VIDDECTEST_FILLDATA_TYPE;

typedef struct APP_TIME
{
    time_t rawTime;
    struct tm* pTimeInfo;
    int nHrs;
    int nMin;
    int nSec;
    OMX_BOOL bInitTime;
    int nTotalTime;
} APP_TIME;

typedef struct MYBUFFER_DATA{
    time_t rawTime;
    struct tm* pTimeInfo;
} MYBUFFER_DATA;

typedef struct OMX_TI_CONFIG_MACROBLOCKERRORMAPTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nErrMapSize;
    OMX_U8  ErrMap[(864 * 480) / 256];
} OMX_TI_CONFIG_MACROBLOCKERRORMAPTYPE;

typedef struct MYDATATYPE{
    OMX_HANDLETYPE pHandle;
    OMX_U8* szInFile;
    OMX_U8* szOutFile;
    OMX_U8* szFrmFile;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_U32 nOutputFormat;
    OMX_U8 eColorFormat;
    OMX_U32 nBitrate;
    OMX_U8 nFramerate;
    OMX_U8 eCompressionFormat;
    OMX_U8 eLevel;
    OMX_U32 nOutBuffSize;
    OMX_STATETYPE eState;
    OMX_PARAM_PORTDEFINITIONTYPE* pInPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE* pOutPortDef;
    OMX_VIDEO_PARAM_AVCTYPE* pH264;
    OMX_VIDEO_PARAM_MPEG4TYPE* pMpeg4;
    OMX_VIDEO_PARAM_WMVTYPE* pWMV;
    OMX_VIDEO_PARAM_MPEG2TYPE* pMpeg2;
#ifdef KHRONOS_1_1
    OMX_PARAM_COMPONENTROLETYPE* pComponentRole;
#endif
    int IpBuf_Pipe[2];
    int OpBuf_Pipe[2];
    int Error_Pipe[2];
    OMX_S32 fdmax;
    FILE* fIn;
    FILE* fBC;
    FILE* fOut;
#ifdef __GET_BC_VOP__
    FILE* fbcData;
    FILE* fvopData;
#endif
    OMX_U8 nDone;
    OMX_BOOL bDeinit;
    OMX_BOOL bWaitExit;
    OMX_BOOL bExit;
    OMX_BOOL bFlushReady;
    OMX_S32 H264BCType;
    OMX_U32 nWMVFileType;
    OMX_BOOL nWMVUseFix;
    OMX_U32 ProcessMode;
    OMX_U32 nCurrentFrameIn;
    OMX_U32 nCurrentFrameOut;
    OMX_U32 nCurrentFrameOutCorrupt;
    OMX_U32 nDecodeFrms;
    /*filldata*/
    OMX_U32 nRetVal;
    OMX_S32 nRewind;
    OMX_U32 nFPos;
    OMX_U32 nFPosSeek;
    OMX_BOOL bFirstBufferRead;
    OMX_U32 nFileSize;
    OMX_U32 nFileTotalRead;
    VIDDECTEST_FILLDATA_TYPE eFillDataType;
    OMX_TICKS nTimeStamp;
    fpos_t pos1;
    fpos_t pos2;
    OMX_S32 nLevelProfile;
    OMX_CALLBACKTYPE* pCb;
    OMX_COMPONENTTYPE* pComponent;
    OMX_BUFFERHEADERTYPE* pInBuff[MAX_VIDDEC_NUM_OF_IN_BUFFERS];
    OMX_BUFFERHEADERTYPE* pOutBuff[MAX_VIDDEC_NUM_OF_OUT_BUFFERS];

    OMX_BOOL bInputEOS;
    OMX_BOOL bOutputEOS;
    OMX_BOOL bCallGetHnd;
    OMX_BOOL bWaitFirstBReturned;
    sem_t sWaitFirstBReturned;
    sem_t sWaitCommandReceived;
    OMX_BOOL bFirstPortSettingsChanged;
    OMX_BOOL bSecondPortSettingsChanged;
    OMX_S32 cFirstPortChanged;
    OMX_S32 cSecondPortChanged;
#ifdef VIDDEC_SPARK_CODE
    OMX_BOOL bIsSparkInput;
#endif

    OMX_U8 cInputBuffers;
    OMX_U8 cOutputBuffers;
    OMX_U32 H264BitStreamFormat;

    APP_TIME* pStartTime;
    APP_TIME* pEndTime;

    OMX_S32 nTimesCount;
    OMX_S32 nRenamingOut;
    OMX_S32 nTimes;
#ifdef ADDED_TESTCASES
    TESTCASE_TYPE nTestCase;
#endif
    OMX_ERRORTYPE eAppError;
    OMX_U32 OutputBufferInComp;

    OMX_U32 nInputPortIndex;
    OMX_U32 nOutputPortIndex;
    OMX_PARAM_DEBLOCKINGTYPE deblockSwType;
    OMX_U8 deblockSW;
    OMX_U8 MB_Error_Logic_Switch;
} MYDATATYPE;

typedef struct VIDEODEC_PORT_INDEX
{
    int nInputPortIndex;
    int nOutputPortIndex;
}VIDEODEC_PORT_INDEX;

/*function prototypes*/
int NormalRunningTest(int argc, char** argv, MYDATATYPE *pTempAppData);
int NormalRunningTest2(int argc, char* argv[], MYDATATYPE *pTempAppData);
int ResourceExhaustationTest(int argc, char** argv);

OMX_ERRORTYPE VIDDECTEST_SplitNAL(OMX_BUFFERHEADERTYPE *pBufHeader, OMX_STRING cFilename, OMX_U32 nBytesConsumed, OMX_BOOL bEndianness, OMX_BOOL bClosed);
OMX_ERRORTYPE VIDDECTEST_LookBack0x0A(FILE* pFile, OMX_U32* nPosition);
OMX_ERRORTYPE VIDDECTEST_GetVOPValues(OMX_BUFFERHEADERTYPE *pBufHeader,
                                      TESTCASE_TYPE* nTestCase,
                                      FILE* pInFile,
                                      FILE* pVopFile,
                                      OMX_U32* nFPosSeek,
                                      OMX_U32* nFPos,
                                      OMX_U32* totalRead,
                                      OMX_U32* index,
                                      OMX_S32* type,
                                      OMX_TICKS* timeStamp);

OMX_ERRORTYPE VIDDECTEST_FillData(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBufferHeader, VIDDECTEST_FILLDATA_TYPE eFillDataType);
OMX_ERRORTYPE TickCounPropatation(MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE *pBufferHeader, OMX_U32 nTickCount);

OMX_ERRORTYPE VidDec_WaitForState(OMX_HANDLETYPE* pHandle, OMX_PTR pAppData, OMX_STATETYPE DesiredState);
OMX_ERRORTYPE GetVidDecPortDef(OMX_HANDLETYPE pHandleVidDecPort, VIDEODEC_PORT_INDEX *pVidDecPortDef);
void VidDec_EventHandler (OMX_HANDLETYPE hComponent,OMX_PTR pAppData,OMX_EVENTTYPE eEvent,
                           OMX_U32 data, OMX_U32 data2, OMX_STRING eInfo);
void VidDec_FillBufferDone (OMX_HANDLETYPE hComponent, MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
void VidDec_EmptyBufferDone (OMX_HANDLETYPE hComponent, MYDATATYPE*pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
OMX_S32 maxint(int a, int b);
/*MPEG4 function prototypes*/
extern OMX_ERRORTYPE MPEG4VIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf);
extern OMX_ERRORTYPE MPEG4VIDDEC_AllocateResources(MYDATATYPE* pAppData);
extern OMX_ERRORTYPE MPEG4VIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
extern void MPEG4VIDDEC_FreeResources(MYDATATYPE* pAppData);
/*H264 function prototypes*/
extern OMX_ERRORTYPE H264VIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf);
extern OMX_ERRORTYPE H264VIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
extern OMX_ERRORTYPE H264VIDDEC_AllocateResources(MYDATATYPE* pAppData);
extern void H264VIDDEC_FreeResources(MYDATATYPE* pAppData);
/*WMV function prototypes*/
extern OMX_ERRORTYPE WMVVIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf);
extern OMX_ERRORTYPE WMVVIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
extern OMX_ERRORTYPE WMVVIDDEC_AllocateResources(MYDATATYPE* pAppData);
extern void WMVVIDDEC_FreeResources(MYDATATYPE* pAppData);
/*MPEG2 function prototypes*/
extern OMX_ERRORTYPE MPEG2VIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf);
extern OMX_ERRORTYPE MPEG2VIDDEC_AllocateResources(MYDATATYPE* pAppData);
extern OMX_ERRORTYPE MPEG2VIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
extern void MPEG2VIDDEC_FreeResources(MYDATATYPE* pAppData);

OMX_ERRORTYPE PortSettingChanged(MYDATATYPE* pAppData, OMX_S32 nParam);
#ifdef __GET_BC_VOP__
typedef enum VIDDEC_BUFFER_OWNER
{
    VIDDEC_BUFFER_WITH_CLIENT = 0x0,
    VIDDEC_BUFFER_WITH_COMPONENT,
    VIDDEC_BUFFER_WITH_DSP,
    VIDDEC_BUFFER_WITH_TUNNELEDCOMP
} VIDDEC_BUFFER_OWNER;

typedef enum VIDDEC_TYPE_ALLOCATE
{
    VIDDEC_TALLOC_USEBUFFER,
    VIDDEC_TALLOC_ALLOCBUFFER
}VIDDEC_TYPE_ALLOCATE;

typedef struct VIDDEC_BUFFER_PRIVATE
{
    OMX_BUFFERHEADERTYPE* pBufferHdr;
    OMX_PTR pUalgParam;
    VIDDEC_BUFFER_OWNER eBufferOwner;
    VIDDEC_TYPE_ALLOCATE bAllocByComponent;
    OMX_U32 nNumber;
} VIDDEC_BUFFER_PRIVATE;

#define H264VDEC_SN_MAX_MB_NUMBER 1620

typedef struct {
    OMX_U32 ulDisplayID;
    OMX_U32 ulBytesConsumed;
    OMX_S32 iErrorCode;
    OMX_U32 ulDecodedFrameType;
    OMX_U32 ulNumOfNALUDecoded;
    OMX_S32 lMBErrStatFlag;
    OMX_U8  pMBErrStatOutBuf[H264VDEC_SN_MAX_MB_NUMBER];
} H264VDEC_UALGOutputParam;

#endif

/*function not defined yet*/
#define VIDDECTEST_TIMESTAMP(_nextTS, _curTS, _mode)   \
{                                                   \
    if (_mode == PROCESS_MODE_TYPE_FRAME) {         \
        _nextTS = _curTS;                               \
    }                                               \
    else {                                          \
        _nextTS = 0;                                    \
    }                                               \
}

#define CHECK_ERROR(_eError, _eCode, _eMsg, _egoto)\
{                                               \
    if (_eError != OMX_ErrorNone) {             \
        ERR_PRINT(_eMsg,_eCode);                \
        _eError = _eCode;                       \
        goto _egoto;                            \
    }                                           \
}

#define CONFIG_SIZE_AND_VERSION(param) \
        param.nSize=sizeof(param); \
        param.nVersion.s.nVersionMajor = VERSION_MAJOR; \
        param.nVersion.s.nVersionMinor = VERSION_MINOR; \
        param.nVersion.s.nRevision = VERSION_REVISION; \
        param.nVersion.s.nStep = VERSION_STEP;

#ifdef VIDDEC_SPARK_CODE
 #define VIDDEC_SPARKCHECK \
    ((pAppData->bIsSparkInput) && \
    (pAppData->eCompressionFormat == OMX_VIDEO_CodingUnused))
#else
 #define VIDDEC_SPARKCHECK OMX_FALSE
#endif

    #define __ERR_PRINT
    #define __APP_DPRINT
    #define __ERR_DPRINT
    #define __PRINT

    #define __ERR_PRINT
    #define __APP_PRINT

#ifdef __APP_PRINT
    #define APP_PRINT printf
#else
    #define APP_PRINT(...)
#endif

#ifdef __APP_DPRINT
    #define APP_DPRINT printf
#else
    #define APP_DPRINT(...)
#endif

#ifdef __ERR_PRINT
    #define ERR_PRINT printf
#else
    #define ERR_PRINT(...)
#endif

#ifdef __ERR_DPRINT
    #define ERR_DPRINT printf
#else
    #define ERR_DPRINT(...)
#endif

#ifdef __PRINT
    #define PRINT printf
#else
    #define PRINT(...)
#endif

#endif /*VIDDECTEST_H*/

