
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
/* ===================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
* ================================================================= */
#include "VidDecTest.h"
#include <unistd.h>
#include "MPEG4DecFunctions.h"
#include "MPEG2DecFunctions.h"
#include "H264DecFunctions.h"
#include "WMV9DecFunctions.h"
#ifdef KHRONOS_1_1
#include "OMX_ComponentRegistry.h"
/*#define TEST_ROLES*/
#endif
/*#define __MEM_TEST__*/
#ifdef __MEM_TEST__
    #include <mcheck.h>
#endif
#define NALPATTERN 0x1000000
#define NALUNITS 15
/* The position of the Deblocking Enable/Disable Switch
 * */
#define DEBLOCKING_ARG_POS (15)
/* The position of the MB Error Logic Switch
 * */
#define MB_ERROR_LOGIC_SWITCH_POS (16)
/* Make Private the Routine that Sets Deblocking
 * */
static OMX_ERRORTYPE MPEG4VIDDEC_SetParamDeblocking(MYDATATYPE* pAppData);
/* Make private the MB Error Printing Routine
 * */
static OMX_S32 MBErrorPrintRoutine(OMX_HANDLETYPE pHandle, OMX_U32* bMBErrorCount, MYDATATYPE* pAppData);
/* safe routine to get the maximum of 2 integers */
OMX_S32 maxint(int a, int b)
{
    return (a>b) ? a : b;
}

OMX_U32 CONV_ENDIAN (OMX_U32 endianess, OMX_U32 value);
OMX_U32 CONV_ENDIAN (OMX_U32 endianess, OMX_U32 value)
{
  OMX_U32 nTemp = 0;
  OMX_U8* pTemp = 0;
  pTemp = (OMX_U8*)&value;
        if ((endianess) == 0) {
        nTemp = (OMX_U32)pTemp[0]<<0 |
                         pTemp[1]<< 8 |
                         pTemp[2]<< 16 |
                         pTemp[3]<<24;
        }
        else {
        nTemp = (OMX_U32)pTemp[0]<<24|
                         pTemp[1]<< 16|
                         pTemp[2]<< 8|
                         pTemp[3]<<0 ;
        }
        return nTemp;
}

OMX_ERRORTYPE VIDDECTEST_SplitNAL(OMX_BUFFERHEADERTYPE *pBufHeader, OMX_STRING cFilename, OMX_U32 nBytesConsumed, OMX_BOOL bLittleEndianness, OMX_BOOL bClosed)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    static FILE* pFileNAL = NULL;
    static FILE* pFileStream = NULL;
    static FILE* pFileFrame = NULL;
    static FILE* pFileInput = NULL;
    static FILE* pFileNALData = NULL;
    static FILE* pFileNALVop = NULL;
    OMX_U32 nPositionInit = 0;
    OMX_U32 nPositionActual = 0;
    OMX_U32 nPositionFind = 0;
    OMX_U32 nTotalBytes = 0;
    OMX_U32 nCount = 0;
    OMX_U32 nTemp = 0;
    OMX_U32 nTemp2 = 0;
    OMX_U32  nNALList[15];
    static OMX_STRING cNALName = NULL;
    static OMX_STRING cStreamName = NULL;
    static OMX_STRING cFrameName = NULL;
    static OMX_STRING cNALDataName = NULL;
    static OMX_STRING cNALVopName = NULL;
    static OMX_U8* pBuffer = NULL;
    OMX_STRING pTemp = NULL;
    OMX_U32 nIntBuffer = 0;

    if (pBufHeader->nFilledLen == 0 && (pBufHeader->nFlags & OMX_BUFFERFLAG_EOS)) {
        return eError;
    }
    if (bClosed) {
        if (pFileInput != NULL ) { fclose (pFileInput);}
        if (pFileNAL != NULL ) { fclose (pFileNAL);}
        if (pFileStream != NULL ) { fclose (pFileStream);}
        if (pFileFrame != NULL ) { fclose (pFileFrame);}
        if (pFileNALData != NULL ) { fclose (pFileNALData);}
        if (pFileNALVop != NULL ) { fclose (pFileNALVop);}
        free (cNALName);
        free (cStreamName);
        free (cFrameName);
        free (cNALDataName);
        free (cNALVopName);
        return eError;
    }

    nTotalBytes = nBytesConsumed;
    pBuffer = malloc (pBufHeader->nAllocLen);
    memset(pBuffer,0,nTotalBytes);

    if (pFileNAL == NULL) {
        pFileInput = fopen((char*)cFilename, "rb");
        cNALName = malloc(strlen(cFilename) + 10);
        strcpy(cNALName, cFilename);
        pTemp = strrchr(cNALName, '.');
        *pTemp = '\0';
        strcat(cNALName, ".264nb");
        pFileNAL = fopen((char*)cNALName, "wb+");
        if (pFileStream == NULL) {
            cStreamName = malloc(strlen(cFilename) + 15);
            strcpy(cStreamName, cFilename);
            pTemp = strrchr(cStreamName, '.');
            *pTemp = '\0';
            strcat(cStreamName, "_stream.264nd");
            pFileStream = fopen((char*)cStreamName, "wb+");
        }
        if (pFileFrame == NULL) {
            cFrameName = malloc(strlen(cFilename) + 15);
            strcpy(cFrameName, cFilename);
            pTemp = strrchr(cFrameName, '.');
            *pTemp = '\0';
            strcat(cFrameName, "_frame.264nd");
            pFileFrame = fopen((char*)cFrameName, "wb+");
        }
        if (pFileNALData == NULL) {
            cNALDataName = malloc(strlen(cFilename) + 15);
            strcpy(cNALDataName, cFilename);
            pTemp = strrchr(cNALDataName, '.');
            *pTemp = '\0';
            if (bLittleEndianness) {
                strcat(cNALDataName, "_4Bytes_LE.264nb");
            }
            else {
                strcat(cNALDataName, "_4Bytes_BE.264nb");
            }
            pFileNALData = fopen((char*)cNALDataName, "wb+");
        }
        if (pFileNALVop == NULL) {
            cNALVopName = malloc(strlen(cFilename) + 15);
            strcpy(cNALVopName, cFilename);
            pTemp = strrchr(cNALVopName, '.');
            *pTemp = '\0';
            if (bLittleEndianness) {
                strcat(cNALVopName, "_4Bytes_LE.264nd");
            }
            else {
                strcat(cNALVopName, "_4Bytes_BE.264nd");
            }
            pFileNALVop = fopen((char*)cNALVopName, "w+");
        }
    }
    fread(pBuffer, 1, nBytesConsumed, pFileInput);
    nCount = 0;
    nPositionInit = 0;
    for ( nPositionActual = 0; nPositionActual < nTotalBytes; nPositionActual++){
        nIntBuffer = (OMX_U32)pBuffer[nPositionActual]<<0 |
                              pBuffer[nPositionActual+1] << 8 |
                              pBuffer[nPositionActual+2] << 16 |
                              pBuffer[nPositionActual+3]<<24;
        if (nIntBuffer == NALPATTERN) {
            if (nPositionActual != 0) {
                nNALList[nCount++] = nPositionActual - nPositionInit - 4;
                fwrite( &pBuffer[nPositionInit + 4], 1, nPositionActual - nPositionInit - 4, pFileNAL);
                if (bLittleEndianness) {
                    nTemp2 = CONV_ENDIAN (0, nNALList[nCount - 1]);
                }
                else {
                    nTemp2 = CONV_ENDIAN (1, nNALList[nCount - 1]);
                }
                fwrite( &nTemp2, 1, 4, pFileNALData);
                fwrite( &pBuffer[nPositionInit + 4], 1, nNALList[nCount - 1], pFileNALData);
                nPositionInit = nPositionActual;
            }
        }
        if (nPositionActual >= nTotalBytes - 4) {
            nNALList[nCount++] = nTotalBytes - nPositionInit - 4;
            fwrite( &pBuffer[nPositionInit + 4], 1, nTotalBytes - nPositionInit - 4, pFileNAL);
            if (bLittleEndianness) {
                nTemp2 = CONV_ENDIAN (0, nNALList[nCount - 1]);
            }
            else {
                nTemp2 = CONV_ENDIAN (1, nNALList[nCount - 1]);
            }
            fwrite( &nTemp2, 1, 4, pFileNALData);
            fwrite( &pBuffer[nPositionInit + 4], 1, nNALList[nCount - 1], pFileNALData);
            break;
        }
    }

    nTemp = nCount;
    nPositionActual = 0;
    nPositionFind = 0;
    while (nTemp != 0) {
        OMX_U32  nTmp = 1;
        fwrite( &nTmp, 4, 1, pFileStream);
        fwrite( &nNALList[nPositionFind], 4, 1, pFileStream);
        nPositionActual = nNALList[nPositionFind];
        nPositionFind++;
        nTemp--;
    }
    nTemp = nCount;
    nPositionActual = 0;
    nPositionFind = 0;
    nTemp2 = 0;
    fwrite( &nTemp, 4, 1, pFileFrame);
    while (nTemp != 0) {
        nTemp2 += nNALList[nPositionFind] + 4;
        fwrite( &nNALList[nPositionFind], 4, 1, pFileFrame);
        nPositionFind++;
        nTemp--;
    }
    fprintf(pFileNALVop, "%d\n", (int)nTemp2);
    free (pBuffer);
    return eError;
}/*VIDDECTEST_SplitNAL*/

OMX_ERRORTYPE VIDDECTEST_LookBack0x0A(FILE* pFile, OMX_U32* nPosition)
{
    OMX_S32 nRetVal = 0;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    if (nPosition != NULL) {
        if ((*nPosition) != 0) {
            nRetVal = fseek (pFile, (*nPosition), SEEK_SET);
            if (ferror (pFile)) {
                clearerr (pFile);
                eError = OMX_ErrorHardware;
            }
        }
    }
    do {
        if (!feof (pFile)) {
            nRetVal = fseek (pFile, -2 , SEEK_CUR);
            if (ferror (pFile)) {
                clearerr (pFile);
                eError = OMX_ErrorHardware;
                break;
            }
        }
        else {
            fseek (pFile, 0, SEEK_SET);
            clearerr (pFile);
            eError = OMX_ErrorNoMore;
            break;
        }
        if (ftell (pFile) == 0) {
            fseek (pFile,0 , SEEK_SET);
            clearerr (pFile);
            eError = OMX_ErrorNoMore;
            break;
        }
        if (ftell (pFile) <= 5) {
            fseek (pFile,0 , SEEK_SET);
            clearerr (pFile);
            eError = OMX_ErrorNone;
            break;
        }
        nRetVal = fgetc (pFile);
        if (eError == OMX_ErrorNone) {
            break;
        }
    } while (nRetVal != 0x0A);
    if (nRetVal == 0x0A) {
        if (nPosition != NULL) {
            (*nPosition) = ftell (pFile);
        }
        eError = OMX_ErrorNone;
    }
    return eError;
}

OMX_ERRORTYPE VIDDECTEST_GetVOPValues(OMX_BUFFERHEADERTYPE *pBufHeader,
                                    TESTCASE_TYPE* nTestCase,
                                    FILE* pInFile,
                                    FILE* pVopFile,
                                    OMX_U32* nFPosSeek, /*used as current position*/
                                    OMX_U32* nFPos, /*mobile position value*/
                                    OMX_U32* totalRead,
                                    OMX_U32* index,
                                    OMX_S32* type,
                                    OMX_TICKS* timeStamp)
{
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;
    static OMX_BOOL bFirstBufferRead = OMX_FALSE;
    static fpos_t pos;
    static fpos_t pos1;
    OMX_U32 nTempIndex = 0;
    OMX_U32 temptimeStamp = 0;
    OMX_U32 index2 = 0;
    OMX_U32 type2 = 0;
    OMX_U32 temptimeStamp2 = 0;

    OMX_S32 nRetVal = 0;
    OMX_S32 nReadScan = 0;

    if (pBufHeader == NULL || nTestCase == NULL || pInFile == NULL || pVopFile == NULL ||
        index == NULL || type == NULL || timeStamp == NULL) {
        eError = OMX_ErrorUndefined;
    }
    else {
        /*continue reading the VOP file*/
        if(!feof(pVopFile)) {
            if((*nTestCase) == TESTCASE_TYPE_FASTFORWARD) {
                (*type) = -1;
                (*index) = ftell (pInFile);
                do {
                    if(feof(pVopFile)) {
                        (*index) = (*totalRead);
                        (*type) = 0;
                        clearerr (pVopFile);
                        eError = OMX_ErrorNoMore;
                        break;
                    }
                    nTempIndex = (*index);
                    nReadScan = fscanf ( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);

                } while((*type) != 0);
                if ((*type) == 0) {
                    (*totalRead) = nTempIndex;
                    (*timeStamp) = (OMX_TICKS)temptimeStamp;
                    fseek(pInFile, (*totalRead), SEEK_SET);
                    eError = OMX_ErrorNone;
                }
                else {
                    eError = OMX_ErrorNoMore;
                }
            }/*end of TESTCASE_TYPE_FASTFORWARD*/
            else if ( (*nTestCase) == TESTCASE_TYPE_REWIND) {
                (*type) = -1;
                (*index) = ftell (pInFile);
                if(bFirstBufferRead) {
                    /*if first buffer then go end of file*/
                    if (ftell ( pVopFile) <= 1) {
                        fseek( pVopFile, -5 , SEEK_END);
                        fgetpos ( pVopFile, &pos);
                    }
                    do {
                        nTempIndex = (*index);
                        fsetpos (pVopFile, &pos);
                        clearerr (pVopFile);
                        eError = VIDDECTEST_LookBack0x0A ( pVopFile, NULL);
                        if( eError == OMX_ErrorNone || eError == OMX_ErrorNoMore) {
                            fgetpos ( pVopFile,&pos);
                            fgetpos ( pVopFile,&pos1);
                            nReadScan = fscanf( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                            fsetpos ( pVopFile,&pos1);
                        }
                        else {
                            (*type) = 0;
                            break;
                        }
                    } while ( (*type) != 0);
                    if ( feof ( pVopFile)) { /*ckeck EOF*/
                        eError = OMX_ErrorNoMore;
                    }
                    if ( eError == OMX_ErrorNone) {
                        eError = VIDDECTEST_LookBack0x0A ( pVopFile, NULL);
                        if ( eError == OMX_ErrorNone) {
                            nReadScan = fscanf ( pVopFile, "%lu %lu %lu", &index2, &type2, &temptimeStamp2);
                            nTempIndex = index2;
                        }
                    }
                    else if ( eError == OMX_ErrorNoMore) {
                        index2 = 0;
                        type2 = (*type);
                        temptimeStamp2 = temptimeStamp;
                        nTempIndex = index2;
                    }
                    if ( eError == OMX_ErrorNoMore) {
                        index2 = 0;
                        type2 = (*type);
                        temptimeStamp2 = temptimeStamp;
                        nTempIndex = index2;
                    }
                    if( eError == OMX_ErrorNone || eError == OMX_ErrorNoMore) {
                        (*totalRead) = nTempIndex;
                        (*timeStamp) = (OMX_TICKS)temptimeStamp;
                        fsetpos ( pVopFile, &pos);
                        fseek ( pInFile, (*totalRead), SEEK_SET);

                    }
                }
                else {
                    /*send the first frame to be decode, decodeonly flag set*/
                    pBufHeader->nFlags |= OMX_BUFFERFLAG_DECODEONLY;
                    nReadScan = fscanf(pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                    bFirstBufferRead = OMX_TRUE;
                    (*timeStamp) = (OMX_TICKS)temptimeStamp;
                    nRetVal = fseek (pVopFile, 0, SEEK_SET);
                    eError = OMX_ErrorNone;
                }
            }/*end of TESTCASE_TYPE_REWIND*/
            else if ( (*nTestCase) == TESTCASE_TYPE_SEEK) {
                if ( bFirstBufferRead) {
                    fgetpos ( pVopFile, &pos);
                    (*nFPos) = 0;
                    fseek ( pVopFile, 0, SEEK_SET);
                    if ( (*nFPosSeek) == 0) {
                        do {
                            /*counting total frames*/
                            if ( feof ( pVopFile)) {
                                clearerr ( pVopFile);
                                eError = OMX_ErrorNone;
                                break;
                            }
                            else {
                                nReadScan = fscanf ( pVopFile, "%lu %lu %lu", &index2, &type2, &temptimeStamp2);
                            }
                            (*nFPos)++;
                        } while (1);
                            /*end counting total frames*/
                        (*nFPosSeek) = ( (*nFPos) / 10) * 7;
                    }
                    (*nFPos) = 0;
                    fseek ( pVopFile, 0, SEEK_SET);
                    do {
                        /*looking for X frames*/
                        if ( feof( pVopFile)) {
                            clearerr ( pVopFile);
                            eError = OMX_ErrorNoMore;
                            break;
                        }
                        else {
                            nReadScan = fscanf ( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                            eError = OMX_ErrorNone;
                            (*nFPos)++;
                        }
                    } while ( (*nFPos) < (*nFPosSeek));
                    /*end of looking for X frames*/
                    if ( eError == OMX_ErrorNone) {
                        fgetpos ( pVopFile, &pos);
                        do {
                            nTempIndex = (*index);
                            eError = VIDDECTEST_LookBack0x0A ( pVopFile, NULL);
                            if( eError == OMX_ErrorNone || eError == OMX_ErrorNoMore) {
                                fgetpos ( pVopFile, &pos1);
                                nReadScan = fscanf ( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                                fgetpos ( pVopFile, &pos);
                                fsetpos ( pVopFile, &pos1);
                                (*nFPos) -= 1;
                            }
                            else {
                                break;
                            }
                        } while ( (*type) != 0);
                        if ( feof ( pVopFile)) { /*ckeck EOF*/
                            eError = OMX_ErrorNoMore;
                        }
                        if ( eError == OMX_ErrorNone) {
                            eError = VIDDECTEST_LookBack0x0A ( pVopFile, NULL);
                            if ( eError == OMX_ErrorNone) {
                                nReadScan = fscanf ( pVopFile, "%lu %lu %lu", &index2, &type2, &temptimeStamp2);
                                nTempIndex = index2;
                            }
                        }
                        if( eError == OMX_ErrorNone || eError == OMX_ErrorNoMore) {
                            (*totalRead) = nTempIndex;
                            (*timeStamp) = (OMX_TICKS)temptimeStamp;
                            (*nTestCase) = TESTCASE_TYPE_PLAYBACK;
                            fseek ( pInFile, (*totalRead), SEEK_SET);
                            fsetpos ( pVopFile, &pos);
                        }
                    }
                    else {
                        ERR_PRINT ( "EOF reached no frame number located %d total %d\n",(int)(*nFPosSeek), (int)(*nFPos));
                        ERR_PRINT ( "decoding from actual frame\n");
                        fsetpos ( pVopFile, &pos);
                    }
                }
                else {
                    pBufHeader->nFlags |= OMX_BUFFERFLAG_DECODEONLY;
                    nReadScan = fscanf ( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                    bFirstBufferRead = OMX_TRUE;
                    (*timeStamp) = (OMX_TICKS)temptimeStamp;
                    nRetVal = fseek ( pVopFile, 0, SEEK_SET);
                    fseek ( pInFile, (*totalRead), SEEK_SET);
                    eError = OMX_ErrorNone;
                }
            }/*end of TESTCASE_TYPE_SEEK*/
            else {/*default*/
                nReadScan = fscanf ( pVopFile, "%lu %lu %lu", index, type, &temptimeStamp);
                if ( feof ( pVopFile)) {
                    eError = OMX_ErrorNoMore;
                }
                if (ferror (pVopFile)) {
                    clearerr (pVopFile);
                    eError = OMX_ErrorHardware;
                }
                else {
                    eError = OMX_ErrorNone;
                }
                if( eError == OMX_ErrorNone || eError == OMX_ErrorNoMore) {
                    if (((*nFPos) < (*nFPosSeek))) {
                        pBufHeader->nFlags |= OMX_BUFFERFLAG_DECODEONLY;
                        (*nFPos)++;
                    }
                    (*timeStamp) = (OMX_TICKS)temptimeStamp;
                    eError = OMX_ErrorNone;
                }
            }
        }
    }
    if( eError != OMX_ErrorNone) {
        if ( eError != OMX_ErrorNoMore) {
            (*index) = (*totalRead);
            (*type) = 0;
            clearerr (pVopFile);
            eError = OMX_ErrorNoMore;
            pBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
            return eError;
        }
        else {
            return eError;
        }
    }
    return eError;
}

OMX_ERRORTYPE VIDDECTEST_FillData(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBufferHeader, VIDDECTEST_FILLDATA_TYPE eFillDataType)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nVOPIndex = 0;
    OMX_TICKS nVOPTimeStamp = 0;
    OMX_S32 nVOPType = 0;
    OMX_S32 nRead = 0;
    OMX_S32 nSizeToRead = 0;
    struct stat filestat;
    static OMX_U32 nbccount = 0;
#ifdef VIDDECTEST_USEFORMATSIZE_RANDOMIZED
    static OMX_U8 unSplitterValue = VIDDECTEST_BUFFERSIZE_SPLITTER_MIN;
#endif
    OMX_U32 npartialtotal = VAL_ZERO;
    if (pAppData == NULL || pBufferHeader == NULL) {
        eError = OMX_ErrorResourcesLost;
        return eError;
    }

    if(pAppData->nRewind != 0) {
        pAppData->nFileSize = 0;
        pAppData->nFileTotalRead = 0;
        pAppData->bFirstBufferRead = OMX_FALSE;
        pAppData->nRewind = 0;
        if (pAppData->fIn != NULL){
            rewind(pAppData->fIn);
        }
        if(pAppData->ProcessMode == PROCESS_MODE_TYPE_FRAME) {
            if (pAppData->fBC != NULL) {
                rewind(pAppData->fBC);
            }
        }
        if(pAppData->ProcessMode == PROCESS_MODE_TYPE_STREAM && pAppData->H264BCType == 2){
            if (pAppData->fBC != NULL) {
                rewind(pAppData->fBC);
            }
        }
        pAppData->nFPosSeek = 0;
        pAppData->nFPos = 0;
    }

    pBufferHeader->nFlags = 0;

    /*verifying the input files sizes*/
    if(pAppData->nFileSize == 0) {
        switch (eFillDataType) {
            case VIDDECTEST_FILLDATA_TYPE_FRAME_BC:
            case VIDDECTEST_FILLDATA_TYPE_FRAME_VOP:
            case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL:
            case VIDDECTEST_FILLDATA_TYPE_STREAM_NAL:
            {
                if (pAppData->fBC != NULL) {
                    OMX_U32 nFileSize = 0;
                    if (pAppData->szFrmFile != NULL) {
                        stat ((char*)pAppData->szFrmFile, &filestat);
                        nFileSize = filestat.st_size;
                    }
                    if( nFileSize <= 0){
                        ERR_DPRINT("VOP-BC file empty, verify it\n");
                        return OMX_ErrorStreamCorrupt;
                    }
                }
                else {
                    ERR_DPRINT("VOP-BC filename not defined\n");
                    return OMX_ErrorStreamCorrupt;
                }
            }
            case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_BE:
            case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_LE:
            {
                if(pAppData->ProcessMode == PROCESS_MODE_TYPE_FRAME) {
                    if (pAppData->fBC != NULL) {
                        OMX_U32 nFileSize = 0;
                        if (pAppData->szFrmFile != NULL) {
                            stat ((char*)pAppData->szFrmFile, &filestat);
                            nFileSize = filestat.st_size;
                        }
                        if( nFileSize <= 0){
                            ERR_DPRINT("VOP-BC file empty, verify it\n");
                            return OMX_ErrorStreamCorrupt;
                        }
                    }
                    else {
                        ERR_DPRINT("VOP-BC filename not defined\n");
                        return OMX_ErrorStreamCorrupt;
                    }
                }
            }
            default:
            {
                break;
            }
        }
        if (pAppData->fIn != NULL) {
            pAppData->nFileSize = 0;
            if (pAppData->szFrmFile != NULL) {
                stat ((char*)pAppData->szInFile, &filestat);
                pAppData->nFileSize = filestat.st_size;
            }
            if(pAppData->nFileSize <= 0){
                ERR_DPRINT("Input file empty, verify it\n");
                return OMX_ErrorStreamCorrupt;
            }
            else {
                APP_DPRINT("Input File size %d\n", (int)pAppData->nFileSize);
            }
        }
        else {
            ERR_DPRINT("Input filename not defined\n");
            return OMX_ErrorStreamCorrupt;
        }
    }

    switch (eFillDataType) {
        case VIDDECTEST_FILLDATA_TYPE_STREAM:
        case VIDDECTEST_FILLDATA_TYPE_FRAME_BC:
        case VIDDECTEST_FILLDATA_TYPE_FRAME_VOP:
        case VIDDECTEST_FILLDATA_TYPE_FRAME_RCV:
        {
            if(eFillDataType == VIDDECTEST_FILLDATA_TYPE_STREAM) {
                if(!feof(pAppData->fIn)) {
#ifdef VIDDECTEST_USEFORMATSIZE
                    if (pAppData->pOutPortDef->nBufferSize > 0) {
#ifndef VIDDECTEST_USEFORMATSIZE_RANDOMIZED
                        if (pAppData->pInPortDef->nBufferSize >= (pAppData->pOutPortDef->nBufferSize / VIDDECTEST_BUFFERSIZE_SPLITTER)) {
                            nSizeToRead = pAppData->pOutPortDef->nBufferSize / VIDDECTEST_BUFFERSIZE_SPLITTER;
                        }
                        else {
                            nSizeToRead = pAppData->pInPortDef->nBufferSize - 1;
                        }
#else
                        nSizeToRead = pAppData->pOutPortDef->nBufferSize / unSplitterValue;
                        if (unSplitterValue++ > VIDDECTEST_BUFFERSIZE_SPLITTER_MAX) {
                            unSplitterValue = VIDDECTEST_BUFFERSIZE_SPLITTER_MIN;
                        }
#endif
                    }
                    else {
                        APP_PRINT("nBufferSize not defined\n");
                        return OMX_ErrorResourcesLost;
                    }
#else
                    nSizeToRead = VIDDECTEST_BUFFERSIZE_FIXED;
#endif
                }
                else{
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }
            }
            else if (eFillDataType == VIDDECTEST_FILLDATA_TYPE_FRAME_RCV) {
               if(!feof(pAppData->fIn)) {
                    if (!pAppData->bFirstBufferRead) {
                        nRead = fread(pBufferHeader->pBuffer, 1, VAL_WMVHEADER, pAppData->fIn);
                        pAppData->bFirstBufferRead = OMX_TRUE;
                        pBufferHeader->pBuffer[0]                = pBufferHeader->pBuffer[8];
                        pBufferHeader->pBuffer[1]                = pBufferHeader->pBuffer[8 + 1];
                        pBufferHeader->pBuffer[2]                = pBufferHeader->pBuffer[8 + 2];
                        pBufferHeader->pBuffer[3]                = pBufferHeader->pBuffer[8 + 3];
                        pBufferHeader->nFlags                    |= OMX_BUFFERFLAG_CODECCONFIG;
                        pBufferHeader->nFilledLen                = 4;
                        break;
                    }
                    else {
                        nSizeToRead = 0;
                        if (pAppData->fIn != NULL) {
                            nRead = fread(&nSizeToRead, sizeof(nSizeToRead), 1, pAppData->fIn);
                        }
                        else {
                            eError = OMX_ErrorResourcesLost;
                            break;
                        }
                    }
                }
                else{
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }

            }/*End VIDDECTEST_FILLDATA_TYPE_FRAME_VOP*/
            else {
                if(!feof(pAppData->fIn) && !feof(pAppData->fBC)) {
                    /*End VIDDECTEST_FILLDATA_TYPE_STREAM*/
                    if (eFillDataType == VIDDECTEST_FILLDATA_TYPE_FRAME_VOP) {
                        eError = VIDDECTEST_GetVOPValues( pBufferHeader,
                                                        /*TESTCASE_TYPE**/  &pAppData->nTestCase,
                                                        /*FILE**/           pAppData->fIn,
                                                        /*FILE**/           pAppData->fBC,
                                                        /*OMX_U32**/        &pAppData->nFPosSeek,  /*used as current position*/
                                                        /*OMX_U32**/        &pAppData->nFPos,      /*mobile position value*/
                                                        /*OMX_U32**/        &pAppData->nFileTotalRead,
                                                        /*OMX_U32**/        &nVOPIndex,
                                                        /*OMX_S32**/        &nVOPType,
                                                        /*OMX_S64**/        &nVOPTimeStamp);

                        if ( eError != OMX_ErrorNone) {
                            if ( eError == OMX_ErrorNoMore) {
                                if (nVOPIndex != 0) {
                                    nSizeToRead = nVOPIndex - pAppData->nFileTotalRead;
                                }
                                else {
                                    nSizeToRead = 0;
                                }
                                pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                                eError = OMX_ErrorNone;
                            }
                            else {
                                pBufferHeader->nFilledLen = 0;
                                break;
                            }
                        }
                        else {
                            if (nVOPIndex != 0) {
                                nSizeToRead = nVOPIndex - pAppData->nFileTotalRead;
                            }
                            else {
                                nSizeToRead = 0;
                            }
                        }

                    }/*End VIDDECTEST_FILLDATA_TYPE_FRAME_VOP*/
                    else {
                        nSizeToRead = 0;
                        if (pAppData->fBC != NULL) {
                            nRead = fread(&nSizeToRead, sizeof(nSizeToRead), 1, pAppData->fBC);
                        }
                        else {
                            eError = OMX_ErrorResourcesLost;
                            break;
                        }
                    }/*End VIDDECTEST_FILLDATA_TYPE_FRAME_BC*/
                }
                else{
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }
            }
            /************************************************/
            /************************************************/
            /************************************************/
            if (eError  == OMX_ErrorNone) {
                /*if the filesize is greater than nSizeToRead*/
                if (nSizeToRead != 0 && ((pAppData->nFileSize - pAppData->nFileTotalRead) < nSizeToRead)) {
                   nSizeToRead = pAppData->nFileSize - pAppData->nFileTotalRead;
                }
                if(nSizeToRead != 0 && ((nSizeToRead) > pBufferHeader->nAllocLen)){
                    nSizeToRead = pBufferHeader->nAllocLen - 1;
                }
                if(!feof(pAppData->fIn)) {
                    if (pAppData->fIn != NULL) {
                        if (nSizeToRead < 0 || nSizeToRead > pAppData->nFileSize) {
                            APP_PRINT("Possible input file corruption\n");
                            return OMX_ErrorHardware;
                        }
                        if ((pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) &&
                            pAppData->nWMVUseFix &&
                            (pAppData->nWMVFileType == WMV_ELEMSTREAM) &&
                            eFillDataType == VIDDECTEST_FILLDATA_TYPE_FRAME_VOP){
                            if (!pAppData->bFirstBufferRead) {
                                pBufferHeader->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
                                pAppData->bFirstBufferRead = OMX_TRUE;
                                nRead = fread(pBufferHeader->pBuffer + pBufferHeader->nOffset, 1, nSizeToRead, pAppData->fIn);
                                APP_DPRINT("-+- %x %x %x %x\n",
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 0],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 1],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 2],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 3]);
                                pAppData->nFileTotalRead += nRead;
                                pBufferHeader->nFilledLen = nRead;
                            }
                            else {
                                nRead = fread(pBufferHeader->pBuffer + pBufferHeader->nOffset, 1, WMV_STARCODE_LENGHT, pAppData->fIn);
                                APP_DPRINT("-- %x %x %x %x\n",
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 0],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 1],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 2],
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 3]);
                                if (pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 0] == 0 &&
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 1] == 0 &&
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 2] == 1 &&
                                    pBufferHeader->pBuffer[(int)pBufferHeader->nOffset + 3] == 0xf) {
                                    OMX_U32 temptimeStamp = 0;
                                    fseek ( pAppData->fIn, -(WMV_STARCODE_LENGHT) , SEEK_CUR);
                                    nRead = fread(pBufferHeader->pBuffer + pBufferHeader->nOffset, 1, nSizeToRead, pAppData->fIn);
                                    pAppData->nFileTotalRead += nRead;
                                    pBufferHeader->nFilledLen = nRead;
                                }
                                else {
                                    nRead = fread(pBufferHeader->pBuffer + pBufferHeader->nOffset, 1, (nSizeToRead - WMV_STARCODE_LENGHT), pAppData->fIn);
                                    pAppData->nFileTotalRead += nRead + WMV_STARCODE_LENGHT;
                                    pBufferHeader->nFilledLen = nRead;
                                }
                            }
                        }
                        else {
                            nRead = fread(pBufferHeader->pBuffer, 1, nSizeToRead, pAppData->fIn);
                            pAppData->nFileTotalRead += nRead;
                            pBufferHeader->nFilledLen = nRead;
                            pBufferHeader->nTimeStamp = nVOPTimeStamp;
                        }
                    }
                    else {
                        eError = OMX_ErrorResourcesLost;
                        break;
                    }
                }
                else{
                    pBufferHeader->nTimeStamp = nVOPTimeStamp;
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }
                /*rcv files do not got timestamp in file it uses tream mode propagation*/
                if (eFillDataType == VIDDECTEST_FILLDATA_TYPE_FRAME_RCV) {
                    VIDDECTEST_TIMESTAMP (pAppData->nTimeStamp, nVOPTimeStamp, PROCESS_MODE_TYPE_STREAM);
                }
                else {
                    VIDDECTEST_TIMESTAMP (pAppData->nTimeStamp, nVOPTimeStamp, pAppData->ProcessMode);
                }
                /*if no read bytes then last buffer*/
                if (nRead <= 0) {
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                }
                eError = OMX_ErrorNone;
            }
            break;
        }
        case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL:
        case VIDDECTEST_FILLDATA_TYPE_STREAM_NAL:
        {
            if(!feof(pAppData->fIn) && !feof(pAppData->fBC)) {
                npartialtotal = 0;
                if(!feof(pAppData->fBC)){
                    nRead = fread(&nbccount, sizeof(nbccount), VAL_ONE, pAppData->fBC);
                }
                while(nbccount != 0){
                    nSizeToRead = 0;
                    --nbccount;
                    if(!feof(pAppData->fBC)){
                        nRead = fread(&nSizeToRead, sizeof(nSizeToRead), 1, pAppData->fBC);
                    }
                    if(!feof(pAppData->fIn)) {
                        if (nSizeToRead < 0 || nSizeToRead > pAppData->nFileSize) {
                            APP_PRINT("Possible input file corruption\n");
                            return OMX_ErrorHardware;
                        }
                        nRead = fread(&pBufferHeader->pBuffer[npartialtotal + 4], 1, nSizeToRead, pAppData->fIn);
#ifdef DEBUG
                        pBufferHeader->pBuffer[npartialtotal + 0] = (((OMX_U32)nSizeToRead) & 0x000000FF);
                        pBufferHeader->pBuffer[npartialtotal + 1] = (((OMX_U32)nSizeToRead) & 0x0000FF00) >> 8;
                        pBufferHeader->pBuffer[npartialtotal + 2] = (((OMX_U32)nSizeToRead) & 0x00FF0000) >> 16;
                        pBufferHeader->pBuffer[npartialtotal + 3] = (((OMX_U32)nSizeToRead) & 0xFF000000) >> 24;
#else
                        pBufferHeader->pBuffer[npartialtotal + 3] = (((OMX_U32)nSizeToRead) & 0x000000FF);
                        pBufferHeader->pBuffer[npartialtotal + 2] = (((OMX_U32)nSizeToRead) & 0x0000FF00) >> 8;
                        pBufferHeader->pBuffer[npartialtotal + 1] = (((OMX_U32)nSizeToRead) & 0x00FF0000) >> 16;
                        pBufferHeader->pBuffer[npartialtotal + 0] = (((OMX_U32)nSizeToRead) & 0xFF000000) >> 24;
#endif
                        if(feof(pAppData->fBC))
                            nRead = 0;
                        else{
                            nRead = ((OMX_U32)nSizeToRead + 4);
                        }
                        npartialtotal += nRead;
                        pAppData->nFileTotalRead += nRead;
                    }
                    else {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                        nRead = 0;
                    }
                }
                if(nRead != 0) {
                    nRead = npartialtotal;
                }
                pAppData->nFileTotalRead += nRead;
                /*NAL files do not got timestamp in file it uses tream mode propagation*/
                if (eFillDataType == VIDDECTEST_FILLDATA_TYPE_FRAME_RCV) {
                    VIDDECTEST_TIMESTAMP (pAppData->nTimeStamp, nVOPTimeStamp, PROCESS_MODE_TYPE_STREAM);
                }
                else {
                    VIDDECTEST_TIMESTAMP (pAppData->nTimeStamp, nVOPTimeStamp, pAppData->ProcessMode);
                }
                /*if no read bytes then last buffer*/
                if (nRead <= 0) {
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                }
                pBufferHeader->nTimeStamp = nVOPTimeStamp;
                pBufferHeader->nFilledLen = nRead;
                eError = OMX_ErrorNone;
            }
            else{
                pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                pBufferHeader->nFilledLen = 0;
                nRead = 0;
                eError = OMX_ErrorNone;
            }
            break;/*End VIDDECTEST_FILLDATA_TYPE_NAL*/
        }
        case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_BE:
        case VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_LE:
        {
            if(pAppData->ProcessMode == PROCESS_MODE_TYPE_FRAME) {
                nSizeToRead = 0;
                if(!feof(pAppData->fIn) && !feof(pAppData->fBC)) {
                    fscanf ( pAppData->fBC, "%lu", &nSizeToRead);
                    if(feof(pAppData->fBC)) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                        pBufferHeader->nFilledLen = 0;
                        nRead = 0;
                        eError = OMX_ErrorNone;
                        goto EXIT;
                    }
                    nRead = fread(pBufferHeader->pBuffer, 1, nSizeToRead, pAppData->fIn);
                    if(feof(pAppData->fIn)) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                        pBufferHeader->nFilledLen = 0;
                        nRead = 0;
                        eError = OMX_ErrorNone;
                        goto EXIT;
                    }
                    if (nRead <= 0) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    }
                    pAppData->nFileTotalRead += nRead;
                    pBufferHeader->nFilledLen = nRead;
                    eError = OMX_ErrorNone;
                }
                else {
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }
            }
            else {
                nSizeToRead = 0;
                if(!feof(pAppData->fIn)) {
                    nRead = fread(&nSizeToRead, 1, pAppData->H264BitStreamFormat, pAppData->fIn);
                    if(feof(pAppData->fIn)) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                        pBufferHeader->nFilledLen = 0;
                        nRead = 0;
                        eError = OMX_ErrorNone;
                        goto EXIT;
                    }
                    nSizeToRead = CONV_ENDIAN (1, nSizeToRead);
                    fseek ( pAppData->fIn, -(pAppData->H264BitStreamFormat) , SEEK_CUR);
                    nSizeToRead += pAppData->H264BitStreamFormat;
                    nRead = fread(pBufferHeader->pBuffer, 1, nSizeToRead, pAppData->fIn);
                    if(feof(pAppData->fIn)) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                        pBufferHeader->nFilledLen = 0;
                        nRead = 0;
                        eError = OMX_ErrorNone;
                        goto EXIT;
                    }
                    if (nRead <= 0) {
                        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    }
                    pAppData->nFileTotalRead += nRead;
                    pBufferHeader->nFilledLen = nRead;
                    eError = OMX_ErrorNone;
                }
                else {
                    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pBufferHeader->nFilledLen = 0;
                    nRead = 0;
                    eError = OMX_ErrorNone;
                }
            }
            break;
        }
        default:
            APP_PRINT("Incorrect operation mode\n");
            return OMX_ErrorNotImplemented;
    }
EXIT:
    return eError;
}

OMX_ERRORTYPE TickCounPropatation(MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE *pBufferHeader, OMX_U32 nTickCount)
{
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    if(pAppData->ProcessMode == 0 && pAppData->eCompressionFormat != OMX_VIDEO_CodingWMV){
        pBufferHeader->nTickCount = nTickCount;
        eError = OMX_ErrorNone;
    }
    if(pAppData->ProcessMode == 1 && pAppData->H264BCType == 2){
        pBufferHeader->nTickCount = nTickCount;
        eError = OMX_ErrorNone;
    }
    if((pAppData->ProcessMode == 0) &&
        (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) &&
        (pAppData->nWMVFileType == WMV_ELEMSTREAM)) {
        pBufferHeader->nTickCount = nTickCount;
        eError = OMX_ErrorNone;
    }
    if (eError != OMX_ErrorNone) {
        pBufferHeader->nTickCount = 0;
    }
    return eError;
}

OMX_ERRORTYPE GetVidDecPortDef(OMX_HANDLETYPE pHandleVidDecPort, VIDEODEC_PORT_INDEX *pVidDecPortDef)
{
    OMX_PORT_PARAM_TYPE *pTempPortType = NULL;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pTempVidPortDef = NULL;
    OMX_S32 nCnt = 0;
    OMX_BOOL bFoundInput = OMX_FALSE;
    pTempPortType = calloc(1, sizeof(OMX_PORT_PARAM_TYPE));

    eError = OMX_GetParameter(pHandleVidDecPort, OMX_IndexParamVideoInit, pTempPortType);
    if (eError != OMX_ErrorNone){
        ERR_PRINT("PostProctest Error at %d\n",__LINE__);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pTempVidPortDef = calloc (1,sizeof (OMX_PARAM_PORTDEFINITIONTYPE));
    if (!pTempVidPortDef){
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    bFoundInput = OMX_FALSE;
    for (nCnt = pTempPortType->nStartPortNumber; nCnt < pTempPortType->nPorts; nCnt ++){
        pTempVidPortDef->nPortIndex = nCnt;
        eError = OMX_GetParameter (pHandleVidDecPort, OMX_IndexParamPortDefinition, pTempVidPortDef);
        if (eError != OMX_ErrorNone ){
            eError = OMX_ErrorBadParameter;
            goto EXIT;
        }
        if ((pTempVidPortDef->eDir == OMX_DirInput)){
            pVidDecPortDef->nInputPortIndex = nCnt;
            bFoundInput = OMX_TRUE;
            continue;
        }
        else{
            pVidDecPortDef->nOutputPortIndex = nCnt;
            continue;
        }
    }

EXIT:
    if (pTempPortType){
        free(pTempPortType);
        pTempPortType = NULL;
    }
    if (pTempVidPortDef)
    {
        free(pTempVidPortDef);
         pTempVidPortDef = NULL;
    }

    return eError;
}

/**
 *
 *
 *
**/
OMX_ERRORTYPE PortSettingChanged(MYDATATYPE* pAppData, OMX_S32 nParam) {
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    fd_set rfds;

    sigset_t set;
    struct timespec tv;

    OMX_U32 retval = 0;
    OMX_BUFFERHEADERTYPE* pBuf;
    OMX_U32 nCount = 0;
    OMX_HANDLETYPE pHandle = pAppData->pHandle;

    /*in the case port enable, they need do be disable*/
    /*move the component from execute to idle*/
    eError = OMX_GetState(pHandle, &pAppData->eState);
    CHECK_ERROR(eError, eError, "Warning:  VideoDec->GetState has returned status %X\n", EXIT);

    eError = OMX_SendCommand( pHandle, OMX_CommandPortDisable, nParam, NULL);
    CHECK_ERROR(eError, eError, "Error from OMX_CommandPortDisable %X\n", EXIT);

    sleep(1); /*wait until buffers are returned*/

    /*clean the pipes of buffers*/
    if ( nParam == 0 || nParam == 1 || nParam == -1){
        int fdmax = maxint(pAppData->IpBuf_Pipe[0], pAppData->OpBuf_Pipe[0]);
        fdmax = maxint(fdmax, pAppData->Error_Pipe[0]);
        do
        {
            FD_ZERO(&rfds);
            FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->Error_Pipe[0], &rfds);

            tv.tv_sec = 0;
            tv.tv_nsec = 100000;

            sigemptyset (&set);
            sigaddset (&set, SIGALRM);
            retval = pselect (fdmax+1, &rfds, NULL, NULL,&tv, &set);
            sigdelset (&set, SIGALRM);

            if (retval == -1) {
                ERR_PRINT("select()");
                ERR_PRINT(" : Error \n");
                break;
            }

            if (retval == 0) {
                break;
            }

            if( FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) )
            {
                read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
            }
            if( FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) )
            {
                read(pAppData->IpBuf_Pipe[0], &pBuf, sizeof(pBuf));
            }
            if( FD_ISSET(pAppData->Error_Pipe[0], &rfds) )
            {
                read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
            }

        } while(FD_ISSET(pAppData->Error_Pipe[0], &rfds) || FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) || FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds));
    }
    /*clean some variables and return pointer to the beggining*/
    /*the file pointers are returned to 0 because of this test case*/
    /*pending to resolve the rewind x frames*/
    pAppData->nCurrentFrameIn   = 0;
    pAppData->nCurrentFrameOut  = 0;
    pAppData->nCurrentFrameOutCorrupt = 0;
    pAppData->nDecodeFrms      = 0;
    pAppData->nRewind          = 1;

    if ( nParam == 0 || nParam == -1){
        for (nCount = 0; nCount < pAppData->pInPortDef->nBufferCountActual; nCount++) {
            eError = OMX_FreeBuffer(pHandle, pAppData->pInPortDef->nPortIndex, pAppData->pInBuff[nCount]);
            CHECK_ERROR(eError, eError, "Error from OMX_FreeBuffer Input %X\n", EXIT);
        }
        sem_wait( &pAppData->sWaitCommandReceived);
    }
    if ( nParam == 1 || nParam == -1){
        for (nCount = 0; nCount < pAppData->pOutPortDef->nBufferCountActual; nCount++) {
            eError = OMX_FreeBuffer(pHandle, pAppData->pOutPortDef->nPortIndex, pAppData->pOutBuff[nCount]);
            CHECK_ERROR(eError, eError, "Error from OMX_FreeBuffer Output %X\n", EXIT);
        }
        sem_wait( &pAppData->sWaitCommandReceived);
    }

    if ( nParam == 0 || nParam == -1){
        eError = OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, pAppData->pInPortDef);
        CHECK_ERROR(eError, eError, "Error from OMX_GetParameter function %X\n", EXIT);
        pAppData->nWidth    = pAppData->pInPortDef->format.video.nFrameWidth;
        pAppData->nHeight   = pAppData->pInPortDef->format.video.nFrameHeight;
        /*buffersize read only, component is setting this value*/
        /*JESA: Andorid doesn't need to send the new values the component already have it*/
    }
    if ( nParam == 1 || nParam == -1){
        eError = OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, pAppData->pOutPortDef);
        CHECK_ERROR(eError, eError, "Error from OMX_GetParameter function %X\n", EXIT);
        pAppData->nWidth    = pAppData->pOutPortDef->format.video.nFrameWidth;
        pAppData->nHeight   = pAppData->pOutPortDef->format.video.nFrameHeight;
        /*buffersize read only, component is setting this value*/
        /*JESA: Android doesn't need to send the new values the component already have it*/
    }

    eError = OMX_SendCommand( pHandle, OMX_CommandPortEnable, nParam, NULL);
    CHECK_ERROR(eError, eError, "Error from OMX_CommandPortEnable %X\n", EXIT);

    if ( nParam == 0 || nParam == -1){
        for (nCount = 0; nCount < pAppData->pInPortDef->nBufferCountActual; nCount++) {
            eError = OMX_AllocateBuffer(pHandle, &pAppData->pInBuff[nCount],
                pAppData->pInPortDef->nPortIndex, pAppData, pAppData->pInPortDef->nBufferSize);
            CHECK_ERROR(eError, eError, "Error from allocating input buffers %x\n", EXIT);
        }
        sem_wait( &pAppData->sWaitCommandReceived);
        pAppData->bSecondPortSettingsChanged = OMX_FALSE;
    }
    if ( nParam == 1 || nParam == -1){
        for (nCount = 0; nCount < pAppData->pOutPortDef->nBufferCountActual; nCount++) {
            eError = OMX_AllocateBuffer(pHandle, &pAppData->pOutBuff[nCount],
                pAppData->pOutPortDef->nPortIndex, pAppData, pAppData->pOutPortDef->nBufferSize);
            CHECK_ERROR(eError, eError, "Error from allocating output buffers %x\n", EXIT);
        }
        sem_wait( &pAppData->sWaitCommandReceived);
        pAppData->bFirstPortSettingsChanged = OMX_FALSE;
    }

    /*reroute the input and output buffer to the pipes*/
        for (nCount = 0; nCount < pAppData->pOutPortDef->nBufferCountActual; nCount++) {
            eError = OMX_FillThisBuffer(pHandle, pAppData->pOutBuff[nCount]);
            CHECK_ERROR(eError, eError, "Error OMX_FillThisBuffer function %x\n", EXIT);
        }
        /*pAppData->nRewind = 1;*//*must not need to move file pointers back*/
        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
            eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
            eError= MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
            eError= H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
            eError= WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
#ifdef VIDDEC_SPARK_CODE
        else if (VIDDEC_SPARKCHECK) {
            eError= MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
#endif
        if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
            ERR_PRINT ("Error from Fill_Data function %x\n",eError);
            goto EXIT;
        }
        ++pAppData->nCurrentFrameIn;
        TickCounPropatation( pAppData, pAppData->pInBuff[0], pAppData->nCurrentFrameIn);
        if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
            APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[0]->nTickCount,
                (unsigned int)pAppData->pInBuff[0]);
        }
        else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
            APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n", pBuf->nTimeStamp, (unsigned int)pBuf);
        }
        eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[0]);
        CHECK_ERROR(eError, eError, "Error from OMX_EmptyThisBuffer function %x\n", EXIT);
        if((pAppData->pInBuff[0]->nFlags & OMX_BUFFERFLAG_EOS)){
            pAppData->bInputEOS = OMX_TRUE;
        }
        for (nCount = 1; nCount < pAppData->pInPortDef->nBufferCountActual; nCount++) {
            if(pAppData->bInputEOS != OMX_TRUE){
                if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                    pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                     eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[nCount]);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                    eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[nCount]);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                    eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[nCount]);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                    eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[nCount]);
                }
#ifdef VIDDEC_SPARK_CODE
                else if (VIDDEC_SPARKCHECK) {
                    eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[nCount]);
                }
#endif
                if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                    ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                    goto EXIT;
                }
                ++pAppData->nCurrentFrameIn;
                TickCounPropatation( pAppData, pAppData->pInBuff[nCount], pAppData->nCurrentFrameIn);
                if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                    APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[nCount]->nTickCount,
                        (unsigned int)pAppData->pInBuff[nCount]);
                }
                else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                    APP_PRINT("In TimeStamp %lld for output Buffer 0x%x\n", pBuf->nTimeStamp, (unsigned int)pBuf);
                }
                eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[nCount]);
                CHECK_ERROR(eError, eError, "Error from OMX_EmptyThisBuffer function %x\n", EXIT);
                if((pAppData->pInBuff[nCount]->nFlags & OMX_BUFFERFLAG_EOS)){
                    pAppData->bInputEOS = 1;
                    break;
                }
            }
        }
    eError = OMX_ErrorNone;

EXIT:
    return eError;
}

/**
 * This method will wait for the component or mixer to get to the correct
 * state.  It is not a real implementation, but just simulates what a real
 * wait routine may do.
**/
OMX_ERRORTYPE VidDec_WaitForState(OMX_HANDLETYPE* pHandle, OMX_PTR pAppData, OMX_STATETYPE DesiredState)
{
    OMX_STATETYPE eState = OMX_StateInvalid;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pComponent = (OMX_COMPONENTTYPE *)pHandle;

    eError = pComponent->GetState(pHandle, &eState);

     while ((eState != DesiredState)) {
        eError = pComponent->GetState(pHandle, &eState);
        if (eState == OMX_StateInvalid) {
            eError = OMX_ErrorInvalidState;
            break;
        }
        if (((MYDATATYPE *)pAppData)->bWaitExit == OMX_TRUE) {
            eError = ((MYDATATYPE *)pAppData)->nRetVal;
            ((MYDATATYPE *)pAppData)->bWaitExit = OMX_FALSE;
            break;
        }
        sched_yield();
    }/*end while*/
    if (eError != OMX_ErrorNone) {
        ((MYDATATYPE *)pAppData)->eAppError = eError;
        return eError;
    }
    return OMX_ErrorNone;

}

void VidDec_EventHandler (OMX_HANDLETYPE hComponent,OMX_PTR pAppData,OMX_EVENTTYPE eEvent,
                           OMX_U32 nData1, OMX_U32 nData2, OMX_STRING eInfo)
{
    VIDEODEC_PORT_INDEX sVidDecPortIndex;
    OMX_STATETYPE state = OMX_StateInvalid;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    /*removed to avoid blocking calls*/
/*
    eError = OMX_GetState(hComponent, &state);
    if (eError != OMX_ErrorNone) {
        ERR_PRINT("%d :: App: Error returned from GetState\n", __LINE__);
    }
*/
    switch (eEvent) {
        case OMX_EventCmdComplete:
            if (nData1 == OMX_CommandFlush) {
                ((MYDATATYPE *)pAppData)->bFlushReady = OMX_TRUE;
                sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                APP_DPRINT("VideoDecoder - Flush command complete %x\n",(int)nData1);
            }
            else if(nData1 == OMX_CommandStateSet){
                /* State change notification. Do Nothing */
                APP_DPRINT("VideoDecoder - State set command complete %x\n",(int)nData2);
            }
            else if(nData1 == OMX_CommandPortDisable){
                if(nData2 == 0) {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT("VideoDecoder - Input port disabled\n");
                }
                else if(nData2 == 1) {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT("VideoDecoder - Output port disabled\n");
                }
                else {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT("VideoDecoder - ALL_OMX port number disabled\n");
                }
            }
            else if(nData1 == OMX_CommandPortEnable){
                if(nData2 == 0) {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT ("VideoDecoder - Input port enabled\n");
                }
                else if(nData2 == 1) {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT ("VideoDecoder - Output port enabled\n");
                }
                else {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                    APP_DPRINT ("VideoDecoder - ALL_OMX port number enabled\n");
                }
            }
            else if(nData1 == OMX_CommandMarkBuffer){
                APP_DPRINT ("VideoDecoder - Mark buffer command completed\n");
            }
            else {
                APP_DPRINT ("VideoDecoder - Notification received\n");
            }
            break;
        case OMX_EventError:
            /* Error notification */
            if (nData1 == OMX_ErrorPortUnpopulated) {
                APP_DPRINT ("VideoDecoder - Notification received\n");
            }
            else if(nData1 == OMX_ErrorInvalidState) {
                eError = OMX_GetState(hComponent, &state);
                if (eError != OMX_ErrorNone) {
                    ERR_PRINT("%d :: App: Error returned from GetState\n", __LINE__);
                }
                ((MYDATATYPE *)pAppData)->eState = state;
                APP_DPRINT ("VideoDecoder - Notification received: OMX_ErrorInvalidState\n");
            }
            else if(nData1 == OMX_ErrorInsufficientResources) {
                APP_DPRINT ("VideoDecoder - Notification received: InsufficientResources\n");
            }

            if (nData1 != OMX_ErrorNone) {
                APP_DPRINT("%d: App: Error Notification received: Error Num 0x%x: String :%s\n",
                        __LINE__, (int)nData1, (OMX_STRING)eInfo);
            }
            if (nData1 != OMX_ErrorNone) {
                ((MYDATATYPE *)pAppData)->bDeinit = OMX_TRUE;
                ((MYDATATYPE *)pAppData)->nRetVal = nData1;
                ((MYDATATYPE *)pAppData)->bWaitExit = OMX_TRUE;
                ((MYDATATYPE *)pAppData)->nTimesCount = ((MYDATATYPE *)pAppData)->nTimes;
                ((MYDATATYPE *)pAppData)->eAppError = nData1;
                sem_post( &((MYDATATYPE *)pAppData)->sWaitCommandReceived);
                if(!((MYDATATYPE *)pAppData)->bWaitFirstBReturned) {
                    sem_post( &((MYDATATYPE *)pAppData)->sWaitFirstBReturned);
                }
                write(((MYDATATYPE *)pAppData)->Error_Pipe[1], &nData1, sizeof(nData1));
            }
            break;
        case OMX_EventMark:
            break;
        case OMX_EventPortSettingsChanged:
            if(((MYDATATYPE *)pAppData)->bFirstPortSettingsChanged == OMX_FALSE) {
                ((MYDATATYPE *)pAppData)->bFirstPortSettingsChanged = OMX_TRUE;
                ((MYDATATYPE *)pAppData)->cFirstPortChanged = (OMX_S32)nData1;
            }
            else {
                ((MYDATATYPE *)pAppData)->bSecondPortSettingsChanged = OMX_TRUE;
                ((MYDATATYPE *)pAppData)->cSecondPortChanged = (OMX_S32)nData1;
            }

            APP_DPRINT("%d :: App: OMX_Event Port Settings Changed %d\n", __LINE__, state);
            break;
        case OMX_EventBufferFlag:
            eError = GetVidDecPortDef (hComponent, &sVidDecPortIndex);
            if (eError != OMX_ErrorNone) {
                eError = OMX_ErrorBadParameter;
            }
            if((nData2 == OMX_BUFFERFLAG_STARTTIME)) {
                APP_DPRINT("%d :: App: Start Time Generated %d\n", __LINE__, state);
            }
            if(nData1 == sVidDecPortIndex.nOutputPortIndex && (nData2 & OMX_BUFFERFLAG_EOS) == 1) {
                APP_DPRINT("%d :: App: End of Output Stream %d\n", __LINE__, state);
            }
            if(nData1 == sVidDecPortIndex.nInputPortIndex && (nData2 & OMX_BUFFERFLAG_EOS) == 1) {
                APP_DPRINT("%d :: App: End of Input Stream %d\n", __LINE__, state);
                ((MYDATATYPE *)pAppData)->bInputEOS = 1;
            }
            break;
        case OMX_EventResourcesAcquired:
            break;
#ifdef KHRONOS_1_1
        case OMX_EventComponentResumed:
            break;
        case OMX_EventDynamicResourcesAvailable:
            break;
        case OMX_EventPortFormatDetected:
            break;
#endif
#ifdef KHRONOS_1_2
        case OMX_EventKhronosExtensions:
            break;
        case OMX_EventVendorStartUnused:
            break;
#endif
        case OMX_EventMax:
            break;
    } /*end of switch*/
}

void VidDec_EmptyBufferDone (OMX_HANDLETYPE hComponent, MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    pBuffer->nFlags = 0;
    write(pAppData->IpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));
    if(!pAppData->bWaitFirstBReturned) {
        sem_post( &pAppData->sWaitFirstBReturned);
    }
}

void VidDec_FillBufferDone (OMX_HANDLETYPE hComponent, MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    write(pAppData->OpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));
}
/* ========================================================================== */
/**
 *  MPEG4VIDDEC_SetParamDeblocking() Enable/Disables Deblocking filter at the
 *       OMX IL
 * @param
 *     pAppData            Application's' private data
 *
 * @retval OMX_NoError              Success, ready to roll
 **/
/* ========================================================================== */
OMX_ERRORTYPE MPEG4VIDDEC_SetParamDeblocking(MYDATATYPE* pAppData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    VIDEODEC_PORT_INDEX sVidDecPortIndex;
    /* Get port definitions
     * */
    eError = GetVidDecPortDef (pAppData->pHandle, &sVidDecPortIndex);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    /* Clear the memory related to deblocking
     * */
    memset(&(pAppData->deblockSwType), VAL_ZERO, sizeof(OMX_PARAM_DEBLOCKINGTYPE));
    /* Set its enable/disable flag
     * */
    pAppData->deblockSwType.nSize
                                 = sizeof(OMX_PARAM_DEBLOCKINGTYPE);
    pAppData->deblockSwType.nVersion.s.nVersionMajor
                                 = VERSION_MAJOR;
    pAppData->deblockSwType.nVersion.s.nVersionMinor
                                 = VERSION_MINOR;
    pAppData->deblockSwType.nVersion.s.nRevision
                                 = VERSION_REVISION;
    pAppData->deblockSwType.nVersion.s.nStep
                                 = VERSION_STEP;
    pAppData->deblockSwType.nPortIndex
                                 = sVidDecPortIndex.nOutputPortIndex;
    pAppData->deblockSwType.bDeblocking
                                 = (pAppData->deblockSW != 0);
    eError = OMX_SetParameter(pAppData->pHandle, OMX_IndexParamCommonDeblocking,
                              &(pAppData->deblockSwType));
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
EXIT:
    return eError;
}

void calc_time(MYDATATYPE* pAppData)
{
    OMX_U8 nMinToSec;
    OMX_U8 nFinalSec;
    OMX_U8 nMoreSec;
    float nFPS;

    nMinToSec = (pAppData->pEndTime->nMin) - (pAppData->pStartTime->nMin);
    nMoreSec = nMinToSec * 60;
    nFinalSec = (nMoreSec + pAppData->pEndTime->nSec) - pAppData->pStartTime->nSec;
    nFPS = (float)pAppData->nDecodeFrms / (float)nFinalSec;
    APP_DPRINT("\nEncoding Time    : %d seconds ", nFinalSec);
    APP_DPRINT("Number of Frames : %d", (int)pAppData->nDecodeFrms);
    APP_DPRINT(" Frames per second: %f\n", nFPS);
}


int main(int argc, char** argv)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

#ifdef __MEM_TEST__
    mtrace();
#endif

    if(argc >= 2) {
        /*validating Test Mode*/
        if((atoi(argv[1]) < 0) || (atoi(argv[1]) > TESTCASE_TYPE_MAX)) {
            APP_PRINT("Incorrect Test Mode, invalid value %s\n", argv[1]);
            return -1;
        }

#ifndef __GET_BC_VOP__
        if((atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2) || (atoi(argv[1]) >= 4) && (atoi(argv[1]) <= TESTCASE_TYPE_MAX)) {
            eError = NormalRunningTest(argc, argv, NULL);
        }
        else {
            eError = ResourceExhaustationTest(argc, argv);
        }

#else
        if(((atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2)) || ((atoi(argv[1]) >= 4) && (atoi(argv[1]) <= TESTCASE_TYPE_MAX))) {
            eError = NormalRunningTest(argc, argv, NULL);
        }
        else {
            eError = ResourceExhaustationTest(argc, argv);
        }
#endif
    }
    else {

        APP_PRINT("OMX Test App Built On " __DATE__ ":" __TIME__ "\n");
        APP_PRINT("This exercises the core and OMX Video Decode Component\n");
        APP_PRINT("************************************************************************************\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("MPEG2/H263/MPEG4/H.264/WMV/SPARK Decode Usage: \n");
#else
        APP_PRINT("MPEG2/H263/MPEG4/H.264/WMV Decode Usage: \n");
#endif
        APP_PRINT("usage : %s <testcase> <input video> <input.vop> <output.yuv> <width> <height>\n", argv[0]);
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("  <format 1:MPEG2/2:H263/3:MPEG4/4:H264(bc)/5:H264(vop)/6:H264(NAL-Frame mode)/7:WMV(RCV/VC1)/8:WMV(VC1)/9:SPARK> \n");
#else
        APP_PRINT("  <format 1:MPEG2/2:H263/3:MPEG4/4:H264(bc)/5:H264(vop)/6:H264(NAL-Frame mode)/7:WMV(RCV/VC1)/8:WMV(VC1)> \n");
#endif
        APP_PRINT("  <format 10:H264(NAL Big Endianness mode)/11:H264(NAL Little Endianness mode)> \n");
        APP_PRINT("  <output 0:420/1:422> <0-frame 1-stream> <InBuffers 1-4> <OutBuffers 1-4> <Iterations> \n");
        APP_PRINT("  <Save output 0:save first out/1: save all out> <profile used>\n");



        APP_PRINT("------------------------------------------------------------------------------------\n");
#ifdef __GET_BC_VOP__
        APP_PRINT("arg1: Test Mode: 0:Play; 1:Pause-Resume; 2:Stop-Resume; 3:Exhaustation Test 9:Flushing 10:Get BC and VOP 12:PlayAfterEOS\n");
#else
        APP_PRINT("arg1: Test Mode: 0:Play; 1:Pause-Resume; 2:Stop-Resume; 3:Exhaustation Test \n");
#endif
        APP_PRINT("        testcase 1-pause-resume 4 times for 5 seconds in frame 25\n");
        APP_PRINT("        testcase 2-stop-resume 4 times for 5 seconds in frame 25\n");
        APP_PRINT("        testcase 3 Repetition within TIOMX_GetHandle\\TIOMX_FreeHandle\n");
        APP_PRINT("                   <input command file> <exit on error = 1, no exit = 0>\n");

#ifdef __GET_BC_VOP__
        APP_PRINT("        testcase 9-flushing OMX component after 20 frames and restart it 10 times\n");
        APP_PRINT("        testcase 10-gets (msecs) bc and vop for the input file, no output is written\n");
#endif
        APP_PRINT("        testcase 11-MacrcoBlock Error Reporting, code for H264 and Mpeg4-H263 is enabled\n");
        APP_PRINT("        testcase 12-Play after EOS test is reapeated after EOS is received\n");
        APP_PRINT("------------------------------------------------------------------------------------\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("arg2:  Input video: input.m4p/.263/.264/.rcv/.vc1/.spark\n");
#else
        APP_PRINT("arg2:  Input video: input.m4p/.263/.264/.rcv/.vc1\n");
#endif
        APP_PRINT("arg3:  Header: input.vop/.bc blank.bc/.vop\n");
        APP_PRINT("arg4:  Output: yuv: xyz.yuv\n");
        APP_PRINT("arg5:  Width-numeric:  QCIF-176; CIF-352; QVGA-320(mpeg4/h263); VGA-640;\n");
        APP_PRINT("arg6:  Height-numeric: QCIF-144; CIF-288; QVGA-240(mpeg4/h263); VGA-480;\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("arg7:  Format:  1:MPEG2; 2:H263; 3:MPEG4; 4:H264(bc); 5:H264(vop); [6:H264(NAL-264nd)]; [7:WMV(RCV)]; [8:WMV(VC1); [9:SPARK(SPARK)]\n");
#else
        APP_PRINT("arg7:  Format:  1:MPEG2; 2:H263; 3:MPEG4; 4:H264(bc); 5:H264(vop); [6:H264(NAL-264nd)]; [7:WMV(RCV)]; [8:WMV(VC1)]\n");
#endif
        APP_PRINT("arg8:  Output type YUV: 0:420; 1:422\n");
        APP_PRINT("arg9:  Decoder Mode: 0-Frame; 1:Stream\n");
        APP_PRINT("arg10: Input buffer number  1-4\n");
        APP_PRINT("arg11: Output buffer number 1-4\n");
        APP_PRINT("arg12: Repetition time - numeric\n");
        APP_PRINT("arg13: Save output 0:save first out/1: save all out\n");
        APP_PRINT("arg14: profile used, h264 Level and WMV9 Profile selection in stream mode\n");
        APP_PRINT("arg15: "
                  "Effective only on MPEG4 format \(-arg7=3\)"
                  "Will disable deblocking when "
                  "clear \(0\); if this value is set \(1\) the automatic deblocking "
                  "settings will be used\n");
        APP_PRINT("arg16: "
                  "Enable//Disable MB Error Checking\n");

#ifdef __GET_BC_VOP__
        APP_PRINT("arg17: fps value or NAL lenght used bytes\n");
#endif
        APP_PRINT("************************************************************************************\n");
        APP_PRINT("WMV9 test vector file extension is .rcv for frame mode and .vc1 in stream mode. Header input file can be any blank file.\n");
        APP_PRINT(".bc file Header file for H264 in binary format\n");
        APP_PRINT(".vop file Header file for MPEG4 in ascii format, with timestamps\n");
        APP_PRINT("For NAL format you need to use a 264nb and 264nd files.\n");
        APP_PRINT("H264 Level (0-MAX|1-L1.0|2-L1.b|3-L1.1|4-L1.2|5-L1.3|6-L2|7-L2.1|8-L2.2|9-L3.0 and greater)\n");
        APP_PRINT("WMV Profile (0-MAX|1-QCIF(Frame)|2-CIF(Frame)|3-VGA(Frame)|4-MainLowDefaultOverhead(Stream)\n");
        APP_PRINT("      |5-MainLowLowOverhead(Stream)|6-MainMediumTradeoffOverhead(Stream)\n");
        APP_PRINT("      |7-AdvancedL1DefaultOverhead|8-AdvancedL1TradeoffOverhead(Stream)\n");
        APP_PRINT("      |9-AdvancedL1LowOverhead)(Stream)\n");
        APP_PRINT("WMV Video Decoder cannot run RCV files in stream mode\n");
        APP_PRINT("************************************************************************************\n");
        return -1;
    }
#ifdef __MEM_TEST__
    muntrace();
#endif
    if (eError != OMX_ErrorNone)
        return -1;
    else
        return 0;
}
/* ========================================================================== */
/**
 *  MBErrorPrintRoutine() Print the MBErrors reported by the
 *       OMX IL
 * @param
 *     pHandle             Component's handle
 *     bMBErrorCount       MB Error Count
 *     pAppData            Application's' private data
 *
 * @retval OMX_NoError              Success, ready to roll
 **/
/* ========================================================================== */
OMX_S32 MBErrorPrintRoutine(OMX_HANDLETYPE pHandle, OMX_U32* bMBErrorCount, MYDATATYPE* pAppData)
/***************** MBError Code ********************/
{
    OMX_S32 eError=OMX_ErrorNone;
    OMX_U32 nlooping = 0;
    OMX_TI_CONFIG_MACROBLOCKERRORMAPTYPE pMBErrorMap;
    memset(&pMBErrorMap, 0, sizeof(OMX_TI_CONFIG_MACROBLOCKERRORMAPTYPE));
    pMBErrorMap.nSize                     = sizeof(OMX_TI_CONFIG_MACROBLOCKERRORMAPTYPE);
    pMBErrorMap.nVersion.s.nVersionMajor  = VERSION_MAJOR;
    pMBErrorMap.nVersion.s.nVersionMinor  = VERSION_MINOR;
    pMBErrorMap.nVersion.s.nRevision      = VERSION_REVISION;
    pMBErrorMap.nVersion.s.nStep          = VERSION_STEP;
    eError = OMX_GetConfig(pHandle, OMX_IndexConfigVideoMacroBlockErrorMap, &pMBErrorMap);
    if (eError != OMX_ErrorNone) {
        APP_PRINT("Not Supported setting %x\n",eError);
        goto ERROR;
    }
    for (nlooping = 0; nlooping < pMBErrorMap.nErrMapSize; nlooping++) {
        if (pMBErrorMap.ErrMap[nlooping] != 0) {
            OMX_U8 i;
            /* Search the byte for the MBs causing the
             * problem(s)
             * */
            for (i = 0; i < 8 ; i++) {
                /* If a bit is found set
                 * increment the error count and print the
                 * error
                 * */
                if (pMBErrorMap.ErrMap[nlooping]
                        &  1 << i) {
                    (*bMBErrorCount)++;
                    APP_PRINT("Frm# %d,\tMB# %d\n",
                              pAppData->nCurrentFrameOut,
                              nlooping*8+i);
                }
            }
        }
    }
ERROR:
    return eError;
}


int NormalRunningTest(int argc, char** argv, MYDATATYPE *pTempAppData)
{
    OMX_BOOL bDoPauseResume = OMX_FALSE;
    OMX_BOOL bDoStopResume = OMX_FALSE;
    OMX_HANDLETYPE pHandle;
    MYDATATYPE* pAppData = NULL;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 nPassCount = 0;
    OMX_BOOL nStreamFlag = OMX_FALSE;
    OMX_U32 ntimeoutcount = 0;
    int retval = 0;
    int i = 0;
    fd_set rfds;
    OMX_U32 bMBErrorCount = 0;

    sigset_t set;
    struct timespec tv;

    OMX_BUFFERHEADERTYPE* pBuf;
    char* fileOutName = NULL, cmd_format[16];
#ifdef __GET_BC_VOP__
    char *vopname=NULL;
    int currentUseFrame, totalUseFrame=0, rate = 66;
#endif
    struct stat filestat;

    OMX_CALLBACKTYPE VidDecCaBa = {(void *)VidDec_EventHandler,
                                   (void *)VidDec_EmptyBufferDone,
                                   (void *)VidDec_FillBufferDone};

    OMX_INDEXTYPE nVidDecodeCustomParamIndex;

    /* validate command line args */
#ifndef __GET_BC_VOP__
    if(argc > 17) {
#else
    if((argc > 18)) {
#endif

        APP_PRINT("OMX Test App Built On " __DATE__ ":" __TIME__ "\n");
        APP_PRINT("This exercises the core and OMX Video Decode Component\n");
        APP_PRINT("************************************************************************************\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("MPEG2/H263/MPEG4/H.264/WMV/SPARK Decode Usage: \n");
#else
        APP_PRINT("MPEG2/H263/MPEG4/H.264/WMV Decode Usage: \n");
#endif
        APP_PRINT("usage : %s <testcase> <input video> <input.vop> <output.yuv> <width> <height>\n", argv[0]);
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("  <format 1:MPEG2/2:H263/3:MPEG4/4:H264(bc)/5:H264(vop)/6:H264(NAL-Frame mode)/7:WMV(RCV/VC1)/8:WMV(VC1)/9:SPARK> \n");
#else
        APP_PRINT("  <format 1:MPEG2/2:H263/3:MPEG4/4:H264(bc)/5:H264(vop)/6:H264(NAL-Frame mode)/7:WMV(RCV/VC1)/8:WMV(VC1)> \n");
#endif
        APP_PRINT("  <format 10:H264(NAL Big Endianness mode)/11:H264(NAL Little Endianness mode)> \n");
        APP_PRINT("  <output 0:420/1:422> <0-frame 1-stream> <InBuffers 1-4> <OutBuffers 1-4> <Iterations> \n");
        APP_PRINT("  <Save output 0:save first out/1: save all out> <profile used>\n");



        APP_PRINT("------------------------------------------------------------------------------------\n");
#ifdef __GET_BC_VOP__
        APP_PRINT("arg1: Test Mode: 0:Play; 1:Pause-Resume; 2:Stop-Resume; 3:Exhaustation Test 9:Flushing 10:Get BC and VOP 12:PlayAfterEOs\n");
#else
        APP_PRINT("arg1: Test Mode: 0:Play; 1:Pause-Resume; 2:Stop-Resume; 3:Exhaustation Test \n");
#endif
        APP_PRINT("        testcase 1-pause-resume 4 times for 5 seconds in frame 25\n");
        APP_PRINT("        testcase 2-stop-resume 4 times for 5 seconds in frame 25\n");
        APP_PRINT("        testcase 3 Repetition within TIOMX_GetHandle\\TIOMX_FreeHandle\n");
        APP_PRINT("                   <input command file> <exit on error = 1, no exit = 0>\n");

#ifdef __GET_BC_VOP__
        APP_PRINT("        testcase 9-flushing OMX component after 20 frames and restart it 10 times\n");
        APP_PRINT("        testcase 10-gets (msecs) bc and vop for the input file, no output is written\n");
#endif
        APP_PRINT("        testcase 11-MacrcoBlock Error Reporting, code for H264 and Mpeg4-H263 is enabled\n");
        APP_PRINT("        testcase 12-Play after EOS test is reapeated after EOS is received\n");
        APP_PRINT("------------------------------------------------------------------------------------\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("arg2:  Input video: input.m4p/.263/.264/.rcv/.vc1/.spark\n");
#else
        APP_PRINT("arg2:  Input video: input.m4p/.263/.264/.rcv/.vc1\n");
#endif
        APP_PRINT("arg3:  Header: input.vop/.bc blank.bc/.vop\n");
        APP_PRINT("arg4:  Output: yuv: xyz.yuv\n");
        APP_PRINT("arg5:  Width-numeric:  QCIF-176; CIF-352; QVGA-320(mpeg4/h263); VGA-640;\n");
        APP_PRINT("arg6:  Height-numeric: QCIF-144; CIF-288; QVGA-240(mpeg4/h263); VGA-480;\n");
#ifdef VIDDEC_SPARK_CODE
        APP_PRINT("arg7:  Format:  1:MPEG2; 2:H263; 3:MPEG4; 4:H264(bc); 5:H264(vop); [6:H264(NAL-264nd)]; [7:WMV(RCV)]; [8:WMV(VC1); [9:SPARK(SPARK)]\n");
#else
        APP_PRINT("arg7:  Format:  1:MPEG2; 2:H263; 3:MPEG4; 4:H264(bc); 5:H264(vop); [6:H264(NAL-264nd)]; [7:WMV(RCV)]; [8:WMV(VC1)]\n");
#endif
        APP_PRINT("arg8:  Output type YUV: 0:420; 1:422\n");
        APP_PRINT("arg9:  Decoder Mode: 0-Frame; 1:Stream\n");
        APP_PRINT("arg10: Input buffer number  1-%d\n", MAX_VIDDEC_NUM_OF_IN_BUFFERS);
        APP_PRINT("arg11: Output buffer number 1-%d\n", MAX_VIDDEC_NUM_OF_OUT_BUFFERS);
        APP_PRINT("arg12: Repetition time - numeric\n");
        APP_PRINT("arg13: Save output 0:save first out/1: save all out\n");
        APP_PRINT("arg14: profile used, h264 Level and WMV9 Profile selection in stream mode\n");
        APP_PRINT("arg15: "
                  "Effective only on MPEG4 format \(-arg7=3\)"
                  "Will disable deblocking when "
                  "clear \(0\); if this value is set \(1\) the automatic deblocking "
                  "settings will be used\n");
        APP_PRINT("arg16: "
                  "Enable//Disable MB Error Checking\n"
                 );
#ifdef __GET_BC_VOP__
        APP_PRINT("arg17: fps value or NAL lenght used bytes\n");
#endif
        APP_PRINT("************************************************************************************\n");
        APP_PRINT("WMV9 test vector file extension is .rcv for frame mode and .vc1 in stream mode. Header input file can be any blank file.\n");
        APP_PRINT(".bc file  Header file for H264 in binary format\n");
        APP_PRINT(".vop file  Header file for MPEG4 in ascii format, with timestamps\n");
        APP_PRINT("For NAL format you need to use a 264nb and 264nd files.\n");
        APP_PRINT("H264 Level (0-MAX|1-L1.0|2-L1.b|3-L1.1|4-L1.2|5-L1.3|6-L2|7-L2.1|8-L2.2|9-L3.0 and greater)\n");
        APP_PRINT("WMV Profile (0-MAX|1-QCIF(Frame)|2-CIF(Frame)|3-VGA(Frame)|4-MainLowDefaultOverhead(Stream)\n");
        APP_PRINT("      |5-MainLowLowOverhead(Stream)|6-MainMediumTradeoffOverhead(Stream)\n");
        APP_PRINT("      |7-AdvancedL1DefaultOverhead|8-AdvancedL1TradeoffOverhead(Stream)\n");
        APP_PRINT("      |9-AdvancedL1LowOverhead)(Stream)\n");
        APP_PRINT("WMV Video Decoder cannot run RCV files in stream mode\n");
        APP_PRINT("************************************************************************************\n");
        return -1;
    }

    /*validating Input Format*/
#ifndef __GET_BC_VOP__
    if((atoi(argv[7]) != 1) && (atoi(argv[7]) != 2) && (atoi(argv[7]) != 3) && (atoi(argv[7]) != 4) &&
            (atoi(argv[7]) != 5)  && (atoi(argv[7]) != 6) && (atoi(argv[7]) != 7) && (atoi(argv[7]) != 8) &&
            (atoi(argv[7]) != 10) && (atoi(argv[7]) != 11 && (atoi(argv[7]) != 12))
#ifdef VIDDEC_SPARK_CODE
            && (atoi(argv[7]) != 9)) {
#else
            ) {
#endif
        APP_PRINT("Incorrect Format, invalid value %s, must be 2/3/4/5/7\n", argv[7]);
        return -1;
    }
#else
    if( atoi(argv[1]) == 10 )
    {
        if((atoi(argv[7]) != 1) && (atoi(argv[7]) != 2) && (atoi(argv[7]) != 3) && (atoi(argv[7]) != 4) &&
            (atoi(argv[7]) != 5) && (atoi(argv[7]) != 8)
#ifdef VIDDEC_SPARK_CODE
            && (atoi(argv[7]) != 9)) {
#else
            ) {
#endif
            APP_PRINT("Incorrect Format, invalid value %s, must be 2/3/4/5\n", argv[7]);
            return -1;
        }
    }
    else
    {
        if((atoi(argv[7]) != 1) && (atoi(argv[7]) != 2) && (atoi(argv[7]) != 3) && (atoi(argv[7]) != 4) &&
            (atoi(argv[7]) != 5)  && (atoi(argv[7]) != 6) && (atoi(argv[7]) != 7) && (atoi(argv[7]) != 8) &&
            (atoi(argv[7]) != 10) && (atoi(argv[7]) != 11 && (atoi(argv[7]) != 12))
#ifdef VIDDEC_SPARK_CODE
            && (atoi(argv[7]) != 9)) {
#else
            ) {
#endif
            APP_PRINT("Incorrect Format, invalid value %s, must be 2/3/4/5/6/7\n", argv[7]);
            return -1;
        }
    }
#endif

    /*validating Output Type*/
    if((atoi(argv[8]) < 0) || (atoi(argv[8]) > 1)) {
        APP_PRINT("Incorrect Output Type, invalid value %s, must be 0-1\n", argv[8]);
        return -1;
    }

    /*validating Decode Mode*/
    if((atoi(argv[9]) < 0) || (atoi(argv[9]) > 1)) {
        APP_PRINT("Incorrect Decode Mode, invalid value %s, must be 0-1\n", argv[9]);
        return -1;
    }

    /*validating Input Buffers*/
    if((atoi(argv[10]) < 1) || (atoi(argv[10]) > MAX_VIDDEC_NUM_OF_IN_BUFFERS)) {
        APP_PRINT("Incorrect Input Buffers, invalid value %s, must be 1-%d\n", argv[10], MAX_VIDDEC_NUM_OF_IN_BUFFERS);
        return -1;
    }

    /*validating Output Buffers*/
    if((atoi(argv[11]) < 1) || (atoi(argv[11]) > MAX_VIDDEC_NUM_OF_OUT_BUFFERS)) {
        APP_PRINT("Incorrect Output Buffers, invalid value %s, must be 1-%d\n", argv[11], MAX_VIDDEC_NUM_OF_OUT_BUFFERS);
        return -1;
    }

    /*validating Levels and Profiles*/
    if((atoi(argv[14]) < 0) || (atoi(argv[14]) > 10)) {
        APP_PRINT("Incorrect Level/Profile, %s, must be 0-9\n", argv[14]);
        return -1;
    }

    if(strlen(argv[4]) >= (MAX_TEXT_STRING - 10)) {
        APP_PRINT("Error in Handle Assignment\n");
        return -1;
    }

    if(pTempAppData == NULL){
        pAppData = (MYDATATYPE *)malloc(sizeof(MYDATATYPE));
        if (!pAppData) {
            ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
            eError = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        pAppData->bCallGetHnd = OMX_TRUE;
    }
    else{
        pAppData = pTempAppData;
        pAppData->bCallGetHnd = OMX_FALSE;
    }
    pAppData->nDone             = 0;
    pAppData->bDeinit           = OMX_FALSE;
    pAppData->bExit             = OMX_FALSE;
    pAppData->bFlushReady       = OMX_FALSE;
    pAppData->H264BCType        = VIDDECTEST_H264FILETYPE_BC;
    pAppData->ProcessMode       = 0;
    pAppData->szInFile          = NULL;
    pAppData->szOutFile         = NULL;
    pAppData->szFrmFile         = NULL;
    pAppData->fIn               = NULL;
    pAppData->fBC               = NULL;
    pAppData->fOut              = NULL;
#ifdef __GET_BC_VOP__
    pAppData->fbcData           = NULL;
    pAppData->fvopData          = NULL;
#endif
    pAppData->nCurrentFrameIn   = 0;
    pAppData->nCurrentFrameOut  = 0;
    pAppData->nCurrentFrameOutCorrupt = 0;
    pAppData->nRetVal           = 0;
    pAppData->cInputBuffers     = 0;
    pAppData->cOutputBuffers    = 0;
    pAppData->bWaitExit         = 0;
    pAppData->H264BitStreamFormat = 0;
    pAppData->eAppError         = 0;
    pAppData->nLevelProfile     = 0;
    pAppData->nRenamingOut      = 0;
    pAppData->nTimes            = 0;
    pAppData->nWMVFileType      = 0;
    pAppData->nWMVUseFix        = OMX_FALSE;
    pAppData->bWaitFirstBReturned = OMX_FALSE;
    pAppData->nTestCase         = TESTCASE_TYPE_PLAYBACK;
#ifdef VIDDEC_SPARK_CODE
    pAppData->bIsSparkInput     = OMX_FALSE;
#endif

    /*TODO: Get the index using OMX_IndexParamPortDefinition*/
    pAppData->nInputPortIndex = 0;
    pAppData->nOutputPortIndex = 1;

    bDoPauseResume = OMX_FALSE;
    bDoStopResume = OMX_FALSE;
    eError = OMX_ErrorNone;
    nPassCount = 0;
    ntimeoutcount = 0;
    retval = 0;
    i = 0;

    if (atoi(argv[7]) == 3) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (atoi(argv[7]) == 1) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingMPEG2;
    }
    else if (atoi(argv[7]) == 2) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (atoi(argv[7]) == 4) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;/*bc file*/
        pAppData->H264BCType = VIDDECTEST_H264FILETYPE_BC;
    }
    else if (atoi(argv[7]) == 5) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;/*vop file*/
        pAppData->H264BCType = VIDDECTEST_H264FILETYPE_VOP;
    }
    else if (atoi(argv[7]) == 6) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;/*NAL BC file*/
        pAppData->H264BCType = VIDDECTEST_H264FILETYPE_NAL;
        pAppData->H264BitStreamFormat = 4;
    }
    else if (atoi(argv[7]) == 7) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingWMV;
        if(atoi(argv[9]) == 0){
            pAppData->nWMVFileType = WMV_RCVSTREAM;
        }
        else {
            APP_PRINT("Invalid compression format value rcv stream mode is not supported.\n");
            eError = OMX_ErrorUnsupportedSetting;
            goto EXIT;
        }
    }
    else if (atoi(argv[7]) == 8) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingWMV;
        pAppData->nWMVFileType = WMV_ELEMSTREAM;
    }
    else if (atoi(argv[7]) == 12) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingWMV;
        pAppData->nWMVFileType = WMV_ELEMSTREAM;
        pAppData->nWMVUseFix = OMX_TRUE;
    }
#ifdef VIDDEC_SPARK_CODE
    else if (atoi(argv[7]) == 9) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingUnused;
        pAppData->bIsSparkInput = OMX_TRUE;
    }
#endif
    else if (atoi(argv[7]) == 10) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;/*NAL BC file*/
        pAppData->H264BCType = VIDDECTEST_H264FILETYPE_NAL_TWO_BE;
        if (argc == 16) {
            if (atoi(argv[15]) == 1 || atoi(argv[15]) == 2 || atoi(argv[15]) == 4) {
                pAppData->H264BitStreamFormat = atoi(argv[15]);
            }
            else {
                ERR_PRINT("Invalid bytestream value.\n");
                eError = OMX_ErrorUnsupportedSetting;
                goto EXIT;
            }
        }
        else {
            pAppData->H264BitStreamFormat = 4;
        }
    }
    else if (atoi(argv[7]) == 11) {
        pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;/*NAL BC file*/
        pAppData->H264BCType = VIDDECTEST_H264FILETYPE_NAL_TWO_LE;
        if (argc == 16) {
            if (atoi(argv[15]) == 1 || atoi(argv[15]) == 2 || atoi(argv[15]) == 4) {
                pAppData->H264BitStreamFormat = atoi(argv[15]);
            }
            else {
                ERR_PRINT("Invalid bytestream value.\n");
                eError = OMX_ErrorUnsupportedSetting;
                goto EXIT;
            }
        }
        else {
            pAppData->H264BitStreamFormat = 4;
        }
    }
    else {
        ERR_PRINT("Invalid compression format value.\n");
        eError = OMX_ErrorUnsupportedSetting;
        goto EXIT;
    }

    /*allocate resources for application*/
    if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
        eError = MPEG4VIDDEC_AllocateResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
        eError = MPEG2VIDDEC_AllocateResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
        eError = H264VIDDEC_AllocateResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
        eError = WMVVIDDEC_AllocateResources(pAppData);
    }
#ifdef VIDDEC_SPARK_CODE
    else if (VIDDEC_SPARKCHECK) {
        eError = MPEG4VIDDEC_AllocateResources(pAppData);
    }
#endif
    if (eError != OMX_ErrorNone) {
        ERR_PRINT("Error allocating resources!\n");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
        switch(atoi(argv[14])) {
            case 1:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel1;
                break;
            case 2:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel1b;
                break;
            case 3:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel11;
                break;
            case 4:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel12;
                break;
            case 5:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel13;
                break;
            case 6:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel2;
                break;
            case 7:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel21;
                break;
            case 8:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel22;
                break;
            case 9:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevel3;
                break;
            default:
                pAppData->nLevelProfile = OMX_VIDEO_AVCLevelMax;
                break;
        }
    } else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
        switch(atoi(argv[14])) {
            case 1:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE0;
                break;
            case 2:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE1;
                break;
            case 3:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE2;
                break;
            case 4:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE3;
                break;
            case 5:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE4;
                break;
            case 6:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE5;
                break;
            case 7:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE6;
                break;
            case 8:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE7;
                break;
            case 9:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILE8;
                break;
            default:
                pAppData->nLevelProfile = VIDDECTEST_WMV_PROFILEMAX;
        }
    }
    /*validating the testcase number*/
    if(atoi(argv[1]) == 0) {
        bDoPauseResume        = OMX_FALSE;
        bDoStopResume       = OMX_FALSE;
    }
    else if(atoi(argv[1]) == 1){
        bDoPauseResume        = OMX_TRUE;
        bDoStopResume       = OMX_FALSE;
    }
    else if(atoi(argv[1]) == 2){
        bDoPauseResume        = OMX_FALSE;
        bDoStopResume       = OMX_TRUE;
    }
#ifdef __GET_BC_VOP__
    else if(atoi(argv[1]) == 10){
        bDoPauseResume        = OMX_FALSE;
        bDoStopResume       = OMX_FALSE;
    }
#endif
    else if(atoi(argv[1]) == 3 || atoi(argv[1]) == 4 || atoi(argv[1]) == 5 || atoi(argv[1]) == 9 ||
            atoi(argv[1]) == 6 || atoi(argv[1]) == 7 || atoi(argv[1]) == 8 || atoi(argv[1]) == 11 ||
            atoi(argv[1]) == 12 || atoi(argv[1]) == 13){
        bDoPauseResume        = OMX_FALSE;
        bDoStopResume       = OMX_FALSE;
    }
    else {
        ERR_PRINT("Invalid testcase value\n");
        eError = OMX_ErrorUnsupportedSetting;
        goto EXIT;
    }
    if (atoi(argv[1]) == TESTCASE_TYPE_FLUSHING) {
        pAppData->bFlushReady = OMX_TRUE;
    }
    else {
        pAppData->bFlushReady = OMX_FALSE;
    }
    pAppData->nTestCase = atoi(argv[1]);
    APP_PRINT("Input Filename %s testcase %d\n", argv[2], pAppData->nTestCase);
    pAppData->szInFile      = (OMX_U8*)argv[2];
    if(pAppData->ProcessMode == 0)
        pAppData->szFrmFile     = (OMX_U8*)argv[3];
    if(pAppData->ProcessMode == 1 && pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL){
        pAppData->szFrmFile     = (OMX_U8*)argv[3];
    }
    pAppData->szOutFile         = (OMX_U8*)argv[4];
    pAppData->nWidth            = atoi(argv[5]);
    pAppData->nHeight           = atoi(argv[6]);
    pAppData->nOutputFormat     = atoi(argv[8]);
#ifndef __GET_BC_VOP__
    pAppData->ProcessMode       = atoi(argv[9]);
    pAppData->nRenamingOut      = atoi(argv[13]);
#else
    if( pAppData->nTestCase != 10 )
    {
        pAppData->ProcessMode   = atoi(argv[9]);
        pAppData->nRenamingOut  = atoi(argv[13]);
    }
    else
    {
        pAppData->ProcessMode   = 1;
        pAppData->nRenamingOut   = 0;
    }
#endif
    pAppData->cInputBuffers     = atoi(argv[10]);
    pAppData->cOutputBuffers    = atoi(argv[11]);

    /* Set/Clear the Deblocking Switch
     * */
    if(pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4
    && argc > DEBLOCKING_ARG_POS){
       pAppData->deblockSW = atoi(argv[DEBLOCKING_ARG_POS]);
    } else {
       /* If the parameter is not provided the deblocking will not be disabled
        * */
       pAppData->deblockSW = !0;
    }
    /* Set/Clear the MB Error Checking logic
     * */
    if (argc > MB_ERROR_LOGIC_SWITCH_POS && pAppData->nTestCase != 10){
        pAppData->MB_Error_Logic_Switch = atoi(argv[MB_ERROR_LOGIC_SWITCH_POS]);
    }
    else {
       /* If the parameter is not provided the macroblock error information will
        * be Disabled.
        * */
        pAppData->MB_Error_Logic_Switch = 0;
    }
#ifdef __GET_BC_VOP__
    if( pAppData->nTestCase == 10 ){
        if(argc == 18)
        {
            rate                    = 1000 / atoi((char*)argv[17]);
        }
    }
#endif

    if (pAppData->nOutputFormat == 1) {
        pAppData->eColorFormat = COLORFORMAT422;
    }
    else if (pAppData->nOutputFormat == 0) {
        pAppData->eColorFormat = COLORFORMAT420;
    }
    else {
        ERR_PRINT("Error: incorrect color format %x \n", (int)pAppData->nOutputFormat);
        goto EXIT;
    }

    pAppData->fIn = fopen((char*)pAppData->szInFile, "r");
    if (pAppData->fIn == NULL) {
        eError = OMX_ErrorBadParameter;
        APP_PRINT("Error: failed to open file <%s> for reading \n", pAppData->szInFile);
        goto EXIT;
    }
    if(pAppData->ProcessMode == 0 && pAppData->eCompressionFormat != OMX_VIDEO_CodingWMV){
        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263 ||
            pAppData->H264BCType != VIDDECTEST_H264FILETYPE_BC){
            pAppData->fBC = fopen((char*)pAppData->szFrmFile, "r");
            if (pAppData->fBC == NULL) {
                eError = OMX_ErrorBadParameter;
                APP_PRINT("Error: failed to open file <%s> for reading bc\n", pAppData->szFrmFile);
                goto EXIT;
            }
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC){
            if(pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL){
                pAppData->fBC = fopen((char*)pAppData->szFrmFile, "r");
                if (pAppData->fBC == NULL) {
                    eError = OMX_ErrorBadParameter;
                    APP_PRINT("Error: failed to open file <%s> for reading bc-vop\n", pAppData->szFrmFile);
                    goto EXIT;
                }
            }
            else if(pAppData->H264BCType == VIDDECTEST_H264FILETYPE_BC){
                pAppData->fBC = fopen((char*)pAppData->szFrmFile, "rb");
                if (pAppData->fBC == NULL) {
                    eError = OMX_ErrorBadParameter;
                    APP_PRINT("Error: failed to open file <%s> for reading bc-vop\n", pAppData->szFrmFile);
                    goto EXIT;
                }
            }
            else {
                pAppData->fBC = fopen((char*)pAppData->szFrmFile, "r");
                if (pAppData->fBC == NULL) {
                    eError = OMX_ErrorBadParameter;
                    APP_PRINT("Error: failed to open file <%s> for reading vop\n", pAppData->szFrmFile);
                    goto EXIT;
                }
            }
        }
    }
    if(pAppData->ProcessMode == 1 && pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL){
        pAppData->fBC = fopen((char*)pAppData->szFrmFile, "rb");
        if (pAppData->fBC == NULL) {
            eError = OMX_ErrorBadParameter;
            APP_PRINT("Error: failed to open file <%s> for reading bc-vop\n", pAppData->szFrmFile);
            goto EXIT;
        }
    }
    if((pAppData->ProcessMode == 0) &&
        (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) &&
        (pAppData->nWMVFileType == WMV_ELEMSTREAM)){
        pAppData->fBC = fopen((char*)pAppData->szFrmFile, "r");
        if (pAppData->fBC == NULL) {
            eError = OMX_ErrorBadParameter;
            APP_PRINT("Error: failed to open file <%s> for reading vop\n", pAppData->szFrmFile);
            goto EXIT;
        }
    }
#ifdef VIDDEC_SPARK_CODE
    if ((pAppData->ProcessMode == 0) && VIDDEC_SPARKCHECK) {
        pAppData->fBC = fopen((char*)pAppData->szFrmFile, "r");
        if (pAppData->fBC == NULL) {
            eError = OMX_ErrorBadParameter;
            APP_PRINT("Error: failed to open file <%s> for reading vop\n", pAppData->szFrmFile);
            goto EXIT;
        }
    }
#endif
    pAppData->eState = OMX_StateInvalid;
    int fdmax = maxint(pAppData->IpBuf_Pipe[0], pAppData->OpBuf_Pipe[0]);
    fdmax = maxint(fdmax, pAppData->Error_Pipe[0]);

    retval = sem_init( &pAppData->sWaitCommandReceived, 0, 0);
    retval = sem_init( &pAppData->sWaitFirstBReturned, 0, 0);

    if(pAppData->bCallGetHnd == OMX_TRUE){
        pAppData->pCb = &VidDecCaBa;
        eError = TIOMX_Init();
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("%d :: Error returned by TIOMX_Init() %x\n", __LINE__, eError);
            goto EXIT;
        }

        eError = TIOMX_GetHandle(&pHandle, StrVideoDecoder, pAppData, pAppData->pCb);
        if ((eError != OMX_ErrorNone) || (pHandle == NULL)) {
            ERR_PRINT ("Error in Get Handle function\n");
            goto EXIT;
        }
        APP_DPRINT("TIOMX_GetHandle 0x%x\n",(int)pHandle);

        pAppData->pHandle = pHandle;
        if (!pAppData->pHandle) {
            ERR_PRINT("Error in Handle Assignment\n");
            eError = OMX_ErrorUndefined;
            goto EXIT;
        }
    }
    else {
        pHandle = pAppData->pHandle;
    }

#ifndef __GET_BC_VOP__
    pAppData->nTimes = atoi(argv[12]);
#else
    if( pAppData->nTestCase != 10 )
    {
        pAppData->nTimes = atoi(argv[12]);
    }
    else
    {
        pAppData->nTimes = 1;
    }
    vopname = malloc(strlen((char*)pAppData->szInFile)+5);
    if(!vopname)
    {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        goto FREEHANDLES;
    }
#endif
    pAppData->nTimesCount = 0;

    if(pAppData->nRenamingOut == 0) {
        fileOutName = malloc(strlen((char*)pAppData->szOutFile) + 2);
    }
    else {
        fileOutName = malloc(strlen((char*)pAppData->szOutFile) + ADD_CHARS_TO_OUTPUT + 2);
    }

    if(fileOutName == NULL)
    {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    strcpy((char*)fileOutName, (char*)pAppData->szOutFile);
    pAppData->szOutFile[strlen((char*)pAppData->szOutFile) - 4] = 0;
    sprintf(cmd_format, "%%s_%%0%dd.yuv", ADD_CHARS_TO_OUTPUT);

    do
    {
        pAppData->nDone             = 0;
        pAppData->bDeinit           = 0;
        pAppData->bExit             = 0;
        pAppData->nCurrentFrameIn   = 0;
        pAppData->nCurrentFrameOut  = 0;
        pAppData->nCurrentFrameOutCorrupt = 0;
        pAppData->nRetVal           = 0;
        pAppData->bInputEOS         = OMX_FALSE;
        pAppData->bOutputEOS        = 0;
        pAppData->nDecodeFrms      = 0;
        pAppData->pStartTime->bInitTime = OMX_TRUE;

        nPassCount = 0;
        ntimeoutcount = 0;
        retval = 0;
        if(pAppData->nTestCase == 0) {
            bDoPauseResume        = OMX_FALSE;
            bDoStopResume       = OMX_FALSE;
        }
        else if(pAppData->nTestCase == 1){
            bDoPauseResume        = OMX_TRUE;
            bDoStopResume       = OMX_FALSE;
        }
        else if(pAppData->nTestCase == 2){
            bDoPauseResume        = OMX_FALSE;
            bDoStopResume       = OMX_TRUE;
        }
#ifdef __GET_BC_VOP__
        else if(pAppData->nTestCase == 10){
            bDoPauseResume        = OMX_FALSE;
            bDoStopResume       = OMX_FALSE;
        }
#endif
        else if(pAppData->nTestCase == 3 || pAppData->nTestCase == 4 || pAppData->nTestCase == 5 ||
                pAppData->nTestCase == 6 || pAppData->nTestCase == 7 || pAppData->nTestCase == 8 ||
                pAppData->nTestCase == 11 || pAppData->nTestCase == 9 ||
                pAppData->nTestCase == TESTCASE_TYPE_PLAYAFTEREOS ||
                pAppData->nTestCase == TESTCASE_TYPE_DERINGINGENABLE) {
            bDoPauseResume        = OMX_FALSE;
            bDoStopResume       = OMX_FALSE;
        }
        else {
            eError = OMX_ErrorUnsupportedSetting;
            ERR_PRINT("Invalid testcase value %X\n", eError);
            goto EXIT;
        }

        APP_PRINT("Loop: %ld\n", pAppData->nTimesCount + 1);

        if(pAppData->nRenamingOut) {
            sprintf(fileOutName, cmd_format, pAppData->szOutFile, pAppData->nTimesCount + 1);
        }
        if(pAppData->nRenamingOut || !pAppData->nTimesCount) {
            if(stat(fileOutName, &filestat) == 0) {
                if (unlink(fileOutName) == -1) {
                    ERR_PRINT("Error: failed to erase the file <%s> for writing \n", fileOutName);
                    goto EXIT;
                }
            }
            pAppData->fOut = fopen(fileOutName, "wb");
            if (pAppData->fOut == NULL) {
                eError = OMX_ErrorBadParameter;
                APP_PRINT("Error: failed tof open the file <%s> for writing \n", fileOutName);
                goto EXIT;
            }
            if(chmod(fileOutName,0x1b6) == -1) {/*1b6/1FF*/
                eError = OMX_ErrorBadParameter;
                ERR_PRINT("Error: failed to chmod the file <%s>\n", fileOutName);
                goto EXIT;
            }
        }

        rewind(pAppData->fIn);
        if(pAppData->fBC != NULL)
        {
            rewind(pAppData->fBC);
        }

#ifdef __GET_BC_VOP__

        if( pAppData->nTestCase == 10)
        {
            strcpy(vopname, (char*)pAppData->szInFile);
            if(strrchr(vopname, '.'))
            {
                char *pTemp = NULL;
                pTemp = (char *)strrchr(vopname, '.');
                (*pTemp) = 0;
            }
            strcat(vopname, ".vop");

#ifdef __GET_BC_VOP_ADVANCE__
            pAppData->fvopData = fopen(vopname, "r+");
            if(pAppData->fvopData)
            {
                int frame, val, frameR, nItems;
                char *oldname, ext[] = ".old";
                int sizeoldname;

                nItems = fscanf(pAppData->fvopData, "%d %d %d\n", &frame, &val, &frameR);
                nItems = fscanf(pAppData->fvopData, "%d %d %d\n", &frame, &val, &frameR);
                if(nItems == 3)
                {
                    rate = frameR;
                }

                fclose(pAppData->fvopData);
            }
#endif
            OMX_S32 nRetValue = 0;
            struct stat filestat;
            nRetValue = stat ((char*)vopname, &filestat);
            if(nRetValue == 0)
            {
                char *oldname, ext[] = ".old";
                int sizeoldname;
                sizeoldname = strlen(vopname)+strlen(ext)+1;
                oldname = malloc(sizeoldname);
                if(oldname) {
                    strcpy(oldname, vopname);
                    strcat(oldname, ext);
                    rename(vopname, oldname);
                    free(oldname);
                }
                else {
                    ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
                    goto EXIT;
                }
            }

            pAppData->fvopData = fopen(vopname, "w+");
            if(pAppData->fvopData == NULL)
            {
                eError = OMX_ErrorBadParameter;
                APP_PRINT("App Error: Can't create vop file...\n");
                goto FREEHANDLES;
            }

            strcpy(vopname, (char*)pAppData->szInFile);
            if(strrchr(vopname, '.'))
            {
                char *pTemp = NULL;
                pTemp = (char *)strrchr(vopname, '.');
                (*pTemp) = 0;
            }
            strcat(vopname, ".bc");
            if(pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC)
            {
                pAppData->fbcData = fopen(vopname, "wb+");
                if(pAppData->fbcData == NULL)
                {
                    eError = OMX_ErrorBadParameter;
                    APP_PRINT("App Error: Can't create bc file...\n");
                    goto FREEHANDLES;
                }
            }
        }
#endif

        /*Clean the pipes*/
        do
        {
            FD_ZERO(&rfds);
            FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->Error_Pipe[0], &rfds);


            tv.tv_sec = 0;
            tv.tv_nsec = 1000;

            sigemptyset (&set);
            sigaddset (&set, SIGALRM);
            retval = pselect (fdmax+1, &rfds, NULL, NULL, &tv, &set);
            sigdelset (&set, SIGALRM);
            if (retval == -1) {
                ERR_PRINT("select()");
                ERR_PRINT(" : Error \n");
                break;
            }

            if (retval == 0) {
                break;
            }
            if( FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) )
            {
                read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
            }
            if( FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) )
            {
                read(pAppData->IpBuf_Pipe[0], &pBuf, sizeof(pBuf));
            }
            if( FD_ISSET(pAppData->Error_Pipe[0], &rfds) )
            {
                read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
            }

        } while(FD_ISSET(pAppData->Error_Pipe[0], &rfds) || FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) || FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds));

        /*Inform OMX that cachable output buffer is used */
        eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.CacheableBuffers", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
            if(eError != OMX_ErrorNone) {
                ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
                goto FREEHANDLES;
            }
        APP_PRINT("Custom index = %x\n",nVidDecodeCustomParamIndex);
        unsigned int dummy=1;
        eError = OMX_SetConfig(pAppData->pHandle, nVidDecodeCustomParamIndex, &dummy);
            if (eError != OMX_ErrorNone) {
                APP_PRINT("Not Supported setting %x\n",eError);
                goto FREEHANDLES;
            }
        nVidDecodeCustomParamIndex = 0;

        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
            eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.WMVFileType", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
            if(eError != OMX_ErrorNone) {
                ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
                goto FREEHANDLES;
            }
            eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &pAppData->nWMVFileType);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
        }

        eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.ProcessMode", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
        if(eError != OMX_ErrorNone) {
            ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
            goto FREEHANDLES;
        }

        eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &pAppData->ProcessMode);
        if (eError != OMX_ErrorNone) {
            goto FREEHANDLES;
        }

        eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.H264BitStreamFormat", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
        if(eError != OMX_ErrorNone) {
            ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
            goto FREEHANDLES;
        }

        eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &pAppData->H264BitStreamFormat);
        if (eError != OMX_ErrorNone) {
            goto FREEHANDLES;
        }
        if (pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL_TWO_BE ||
            pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL_TWO_LE ||
            pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL) {
            OMX_BOOL bBigEnd = OMX_FALSE;
            eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.IsNALBigEndian", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
            if(eError != OMX_ErrorNone) {
                ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
                goto FREEHANDLES;
            }
            if (pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL_TWO_LE) {
                bBigEnd = OMX_FALSE;
            }
            else {
                bBigEnd = OMX_TRUE;
            }
            eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &bBigEnd);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
        }
        eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.WMVProfile", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
        if(eError != OMX_ErrorNone) {
            ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
            goto FREEHANDLES;
        }

        eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &pAppData->nLevelProfile);
        if (eError != OMX_ErrorNone) {
            goto FREEHANDLES;
        }
#ifdef VIDDEC_SPARK_CODE
        if (VIDDEC_SPARKCHECK) {
            eError = OMX_GetExtensionIndex(pHandle,"OMX.TI.VideoDecode.Param.IsSparkInput", (OMX_INDEXTYPE *)&nVidDecodeCustomParamIndex);
            if(eError != OMX_ErrorNone) {
                ERR_PRINT ("Error in OMX_GetExtensionIndex function %X\n", eError);
                goto FREEHANDLES;
            }

            eError = OMX_SetParameter (pHandle, nVidDecodeCustomParamIndex, &pAppData->bIsSparkInput);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
        }
#endif
        if (pAppData->nTestCase == TESTCASE_TYPE_DERINGINGENABLE) {
            OMX_CONFIG_IMAGEFILTERTYPE pDeringingParamType;
            CONFIG_SIZE_AND_VERSION(pDeringingParamType);
            pDeringingParamType.nPortIndex = 1;
            pDeringingParamType.eImageFilter = OMX_ImageFilterDeRing;
            eError = OMX_SetParameter (pHandle, OMX_IndexConfigCommonImageFilter, &pDeringingParamType);
            if (eError != OMX_ErrorNone && eError != OMX_ErrorUnsupportedIndex) {
                ERR_PRINT ("Error in Enabling Deringing %X\n", eError);
                goto FREEHANDLES;
            }
        }
#ifdef KHRONOS_1_1
 #ifdef TEST_ROLES
        if(pHandle != NULL){
            OMX_U32 nFoundVal = -1;
            ComponentTable myComponentTable;
            strcpy(myComponentTable.name, StrVideoDecoder);
            eError = OMX_GetRolesOfComponent ( myComponentTable.name, &myComponentTable.nRoles, NULL);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            if (myComponentTable.nRoles == 0) {
                eError = OMX_ErrorUnsupportedSetting;
                ERR_PRINT ("Error in Unsupported setting %X\n", eError);
                goto FREEHANDLES;
            }
            for (i = 0; i < myComponentTable.nRoles; i++) {
               myComponentTable.pRoleArray[i] = malloc(OMX_MAX_STRINGNAME_SIZE);
            }
            eError = OMX_GetRolesOfComponent ( myComponentTable.name, &myComponentTable.nRoles, myComponentTable.pRoleArray);
            if ( eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            for (i = 0; i < myComponentTable.nRoles; i++) {
                if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_MPEG4);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_H263);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_MPEG2);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_H264);
                }
                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_WMV9);
                }
#ifdef VIDDEC_SPARK_CODE
                else if (VIDDEC_SPARKCHECK) {
                    nFoundVal = strcmp((char*)myComponentTable.pRoleArray[i], COMPONENTROLES_SPARK);
                }
#endif

                else {
                    eError = OMX_ErrorUnsupportedSetting;
                    ERR_PRINT ("Error in Unsupported setting %X\n", eError);
                    goto FREEHANDLES;
                }
                if (nFoundVal == 0) {
                    break;
                }
            }
            if (nFoundVal != 0) {
                eError = OMX_ErrorUnsupportedSetting;
                ERR_PRINT ("Error in Unsupported setting %X\n", eError);
                goto FREEHANDLES;
            }
            pAppData->pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)malloc(sizeof(OMX_PARAM_COMPONENTROLETYPE));
            if (!pAppData->pComponentRole) {
                ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
                eError = OMX_ErrorInsufficientResources;
                goto FREEHANDLES;
            }
            memset(pAppData->pComponentRole, VAL_ZERO, sizeof(OMX_PARAM_COMPONENTROLETYPE));

            pAppData->pComponentRole->nSize                     = sizeof(OMX_PARAM_COMPONENTROLETYPE);
            pAppData->pComponentRole->nVersion.s.nVersionMajor  = VERSION_MAJOR;
            pAppData->pComponentRole->nVersion.s.nVersionMinor  = VERSION_MINOR;
            pAppData->pComponentRole->nVersion.s.nRevision      = VERSION_REVISION;
            pAppData->pComponentRole->nVersion.s.nStep          = VERSION_STEP;

            eError = OMX_GetParameter(pAppData->pHandle, OMX_IndexParamStandardComponentRole, pAppData->pComponentRole);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            memset(pAppData->pComponentRole->cRole, 0x0, OMX_MAX_STRINGNAME_SIZE);
            if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_MPEG4);
            }
            else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_H263);
            }
            else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_MPEG2);
            }
            else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_H264);
            }
            else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_WMV9);
            }
#ifdef VIDDEC_SPARK_CODE
            else if (VIDDEC_SPARKCHECK) {
                strcpy((char*)pAppData->pComponentRole->cRole, COMPONENTROLES_SPARK);
            }
#endif
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("Error initializing the portdefs %X\n", eError);
                goto FREEHANDLES;
            }

            eError = OMX_SetParameter(pAppData->pHandle, OMX_IndexParamStandardComponentRole, pAppData->pComponentRole);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            eError = OMX_GetParameter(pAppData->pHandle, OMX_IndexParamStandardComponentRole, pAppData->pComponentRole);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            for (i = 0; i<myComponentTable.nRoles;i++) {
                free(myComponentTable.pRoleArray[i]);
            }
            if (pAppData->pComponentRole != NULL){
                free(pAppData->pComponentRole);
                pAppData->pComponentRole = NULL;
            }
        }
 #endif
#endif
        {
            #define VIDDEC_MAXCOLORFORMAT 3
            OMX_U32 nCount = 0;
            /* Get video color format*/
            OMX_VIDEO_PARAM_PORTFORMATTYPE VideoPortFormat;
            CONFIG_SIZE_AND_VERSION(VideoPortFormat);
            VideoPortFormat.nPortIndex = pAppData->nOutputPortIndex;
            for ( nCount = 0; nCount <= VIDDEC_MAXCOLORFORMAT; nCount++) {
                VideoPortFormat.nIndex = nCount;
                eError = OMX_GetParameter(pAppData->pHandle, OMX_IndexParamVideoPortFormat, &VideoPortFormat);
                if (VideoPortFormat.eColorFormat == pAppData->eColorFormat) {
                    break;
                }
            }
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("Problem setting video port format %x\n", eError);
                goto FREEHANDLES;
            }
        }
        /*initialize the portdefs for application*/
        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
            eError = MPEG4VIDDEC_SetParamPortDefinition(pAppData);
            /* Set the deblocking parameter at the OMX Component
             * */
            eError = MPEG4VIDDEC_SetParamDeblocking(pAppData);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
            eError = MPEG2VIDDEC_SetParamPortDefinition(pAppData);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
            eError = H264VIDDEC_SetParamPortDefinition(pAppData);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
            eError = WMVVIDDEC_SetParamPortDefinition(pAppData);
        }

#ifdef VIDDEC_SPARK_CODE
        else if (VIDDEC_SPARKCHECK) {
            eError = MPEG4VIDDEC_SetParamPortDefinition(pAppData);
            pAppData->bIsSparkInput = OMX_TRUE;
            pAppData->pInPortDef->format.video.cMIMEType = MIMETYPESPARK;
        }
#endif
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Error initializing the portdefs %X\n", eError);
            goto FREEHANDLES;
        }

        eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT ("Error from SendCommand-Idle(Init) State function %X\n", eError);
            goto FREEHANDLES;
        }

        for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
            eError = OMX_AllocateBuffer(pHandle, &pAppData->pInBuff[i], pAppData->pInPortDef->nPortIndex, pAppData, pAppData->pInPortDef->nBufferSize);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT ("Error from allocating input buffers %x\n",eError);
                goto DEINIT;
            }
        }

        for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
            eError = OMX_AllocateBuffer(pHandle, &pAppData->pOutBuff[i], pAppData->pOutPortDef->nPortIndex, pAppData, pAppData->pOutPortDef->nBufferSize);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT ("Error from allocating output buffers %x\n",eError);
                goto DEINIT;
            }
        }

        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateIdle);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
            goto DEINIT;
        }

        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateExecuting, NULL);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Error from SendCommand-Executing State function %X\n", eError);
            goto DEINIT;
        }

        pAppData->pComponent = (OMX_COMPONENTTYPE *)pHandle;
        if (!pAppData->pComponent) {
            eError = OMX_ErrorUndefined;
            goto DEINIT;
        }

        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateExecuting);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
            goto DEINIT;
        }

        /* FPS measure start */
        #ifdef MEASURE_TIME
            if (pAppData->pStartTime->bInitTime == OMX_TRUE) {
                time(&(pAppData->pStartTime->rawTime));
                pAppData->pStartTime->pTimeInfo = localtime(&(pAppData->pStartTime->rawTime));

                pAppData->pStartTime->nHrs = pAppData->pStartTime->pTimeInfo->tm_hour;
                pAppData->pStartTime->nMin = pAppData->pStartTime->pTimeInfo->tm_min;
                pAppData->pStartTime->nSec = pAppData->pStartTime->pTimeInfo->tm_sec;

                APP_DPRINT("Start Time = hrs = %d, min = %d, sec = %d, Frame = %d\n",
                                          pAppData->pStartTime->nHrs,
                                          pAppData->pStartTime->nMin,
                                          pAppData->pStartTime->nSec,
                                          pAppData->nCurrentFrameIn);
                 pAppData->pStartTime->bInitTime = OMX_FALSE;
            }
        #endif

        pAppData->bFirstPortSettingsChanged = OMX_FALSE;
        pAppData->bSecondPortSettingsChanged = OMX_FALSE;
        /***************** MBError Code ********************/
        /* Propagate MBError Enable Code Switch to OMX IL
         * */
        if (pAppData->nTestCase == TESTCASE_TYPE_MBERRORCHECK || pAppData->MB_Error_Logic_Switch) {
            OMX_PARAM_MACROBLOCKSTYPE pMBBlocksType;
            APP_PRINT("MBError Test Case -> %d\n",pAppData->nTestCase);
            if (pAppData->nTestCase == TESTCASE_TYPE_MBERRORCHECK) {
               pAppData->MB_Error_Logic_Switch = 1;
            }
            memset(&pMBBlocksType, 0, sizeof(OMX_PARAM_MACROBLOCKSTYPE));
            pMBBlocksType.nSize                     = sizeof(OMX_PARAM_MACROBLOCKSTYPE);
            pMBBlocksType.nVersion.s.nVersionMajor  = VERSION_MAJOR;
            pMBBlocksType.nVersion.s.nVersionMinor  = VERSION_MINOR;
            pMBBlocksType.nVersion.s.nRevision      = VERSION_REVISION;
            pMBBlocksType.nVersion.s.nStep          = VERSION_STEP;
            eError = OMX_GetConfig(pAppData->pHandle, OMX_IndexParamVideoMacroblocksPerFrame, &pMBBlocksType);
            if (eError != OMX_ErrorNone) {
                APP_PRINT("Not Supported setting %x\n",eError);
                goto DEINIT;
            }
            OMX_CONFIG_MBERRORREPORTINGTYPE pMBErrorType;
            memset(&pMBErrorType, 0, sizeof(OMX_CONFIG_MBERRORREPORTINGTYPE));
            pMBErrorType.nSize                     = sizeof(OMX_CONFIG_MBERRORREPORTINGTYPE);
            pMBErrorType.nVersion.s.nVersionMajor  = VERSION_MAJOR;
            pMBErrorType.nVersion.s.nVersionMinor  = VERSION_MINOR;
            pMBErrorType.nVersion.s.nRevision      = VERSION_REVISION;
            pMBErrorType.nVersion.s.nStep          = VERSION_STEP;
            /* Enable/Disable MB Error Logic on Below Layers
             * */
            pMBErrorType.bEnabled         = pAppData->MB_Error_Logic_Switch;
            eError = OMX_SetConfig(pAppData->pHandle, OMX_IndexConfigVideoMBErrorReporting, &pMBErrorType);
            if (eError != OMX_ErrorNone) {
                APP_PRINT("Not Supported setting %x\n",eError);
                goto DEINIT;
            }
        }

        for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
            eError = OMX_FillThisBuffer(pHandle, pAppData->pOutBuff[i]);
            if (eError != OMX_ErrorNone) {
                if (eError != OMX_ErrorHardware) {
                    ERR_PRINT ("Error OMX_FillThisBuffer function %x\n",eError);
                }
                goto DEINIT;
            }
        }
        pAppData->nRewind = 1;
        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
            eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
            eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
            eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
            FILE *fp = NULL;
            eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
            if(pAppData->nWMVFileType != WMV_ELEMSTREAM){
                OMX_U32 * pWord = NULL;
                /*Verify the size of the Codec Specific Data to avoid data corruption*/
                if(pAppData->pInBuff[0]->nFilledLen > 51){
                    goto DEINIT;
                }
                /*Copy Codec Specific data to expected position in the buffer*/
                for(i = 0; i < (pAppData->pInBuff[0]->nFilledLen); i++){
                    pAppData->pInBuff[0]->pBuffer[i+51] = pAppData->pInBuff[0]->pBuffer[i];
                }
                /*Update the filled Len*/
                pAppData->pInBuff[0]->nFilledLen+=51;
                /*Add resolution to the buffer so then the component could be able to parse*/
                pWord = (pAppData->pInBuff[0]->pBuffer)+15;
                *pWord = pAppData->nWidth;

                pWord = (pAppData->pInBuff[0]->pBuffer)+19;
                *pWord = pAppData->nHeight;
                strcpy((pAppData->pInBuff[0]->pBuffer)+27, "WMV3");
            }
        }
#ifdef VIDDEC_SPARK_CODE
        else if (VIDDEC_SPARKCHECK) {
            eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[0]);
        }
#endif
        if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
            ERR_PRINT ("Error from Fill_Data function %x\n",eError);
            eError = OMX_ErrorHardware;
            goto DEINIT;
        }
        ++pAppData->nCurrentFrameIn;
        APP_DPRINT("b Input Buff FRAME %d %x %x s(%d 0x%x)\n", (int)pAppData->nCurrentFrameIn,
                                 (int)pAppData->pInBuff[0], (int)pAppData->pInBuff[0]->nFlags,
                                 (int)pAppData->pInBuff[0]->nFilledLen,
                                 (int)pAppData->pInBuff[0]->nFilledLen);
        TickCounPropatation( pAppData, pAppData->pInBuff[0], pAppData->nCurrentFrameIn);
        if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
            APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[0]->nTickCount,
                (unsigned int)pAppData->pInBuff[0]);
        }
        else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
            APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n",
                pAppData->pInBuff[0]->nTimeStamp,
                (unsigned int)(unsigned int)pAppData->pInBuff[0]);
        }
        eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[0]);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT ("Error from OMX_EmptyThisBuffer function %x\n",eError);
            goto DEINIT;
        }
        if((pAppData->pInBuff[0]->nFlags & OMX_BUFFERFLAG_EOS)){
            pAppData->bInputEOS = OMX_TRUE;
        }
        /*wait for the first buffer to be parsed*/
        if(!pAppData->bWaitFirstBReturned){
            sem_wait( &pAppData->sWaitFirstBReturned);
            pAppData->bWaitFirstBReturned = OMX_TRUE;
        }
        if (pAppData->bFirstPortSettingsChanged == OMX_TRUE) {
            eError = PortSettingChanged( pAppData, pAppData->cFirstPortChanged);
            if (eError != OMX_ErrorNone) {
                goto FREEHANDLES;
            }
            pAppData->bFirstPortSettingsChanged = OMX_FALSE;
            if (pAppData->bSecondPortSettingsChanged == OMX_TRUE) {
                eError = PortSettingChanged( pAppData, pAppData->cSecondPortChanged);
                if (eError != OMX_ErrorNone) {
                    goto FREEHANDLES;
                }
                pAppData->bSecondPortSettingsChanged = OMX_FALSE;
            }
        }
        else{
            for (i = 1; i < pAppData->pInPortDef->nBufferCountActual; i++) {
                if(pAppData->bInputEOS != VAL_ONE){
                    if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                        pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                         eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                        eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                        eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                        eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                    }
#ifdef VIDDEC_SPARK_CODE
                    else if (VIDDEC_SPARKCHECK) {
                        eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                    }
#endif
                    if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                        ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                        eError = OMX_ErrorHardware;
                        goto DEINIT;
                    }
                    ++pAppData->nCurrentFrameIn;
                    APP_DPRINT("a Input Buff FRAME %d %x %x s(%d %x)\n", (int)pAppData->nCurrentFrameIn,
                         (int)pAppData->pInBuff[i], (int)pAppData->pInBuff[i]->nFlags,
                         (int)pAppData->pInBuff[i]->nFilledLen,
                         (int)pAppData->pInBuff[i]->nFilledLen);
                    TickCounPropatation( pAppData, pAppData->pInBuff[i], pAppData->nCurrentFrameIn);
                    if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                        APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[i]->nTickCount,
                            (unsigned int)pAppData->pInBuff[i]);
                    }
                    else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                        APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n",
                            pAppData->pInBuff[i]->nTimeStamp,
                            (unsigned int)pAppData->pInBuff[i]);
                    }
                    eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[i]);
                    if (eError != OMX_ErrorNone) {
                        ERR_PRINT ("Error from OMX_EmptyThisBuffer function %x\n", eError);
                        goto DEINIT;
                    }
                    if((pAppData->pInBuff[i]->nFlags & OMX_BUFFERFLAG_EOS)){
                        pAppData->bInputEOS = 1;
                        break;
                    }

                }
            }
        }

        eError = OMX_GetState(pHandle, &pAppData->eState);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Error getstate() %X\n", eError);
            goto DEINIT;
        }
        while ((eError == OMX_ErrorNone) && (pAppData->eState != OMX_StateIdle) && (pAppData->eState != OMX_StateInvalid)) {
            FD_ZERO(&rfds);
            FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
            FD_SET(pAppData->Error_Pipe[0], &rfds);
            tv.tv_sec = 0;
            tv.tv_nsec =  30000;

            sigemptyset (&set);
            sigaddset (&set, SIGALRM);
            retval = pselect (fdmax+1, &rfds, NULL, NULL, NULL, &set);
            sigdelset (&set, SIGALRM);
            if (retval == -1) {
                ERR_PRINT("select()");
                ERR_PRINT(" : Error \n");
                break;
            }
            if (retval == 0) {
                /*Do nothing*/
            }
            eError = OMX_GetState(pHandle, &pAppData->eState);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("getstate(), while, Error %X\n", eError);
                goto DEINIT;
            }
            if (FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) && (!pAppData->bDeinit) && (pAppData->eState != OMX_StateIdle)) {
                OMX_BUFFERHEADERTYPE* pBuffer;
                FD_CLR(pAppData->IpBuf_Pipe[0], &rfds);
                ntimeoutcount = 0;
                read(pAppData->IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                if(pBuffer != NULL && !pAppData->bInputEOS) {
                    if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                        eError = MPEG4VIDDEC_Fill_Data(pAppData,pBuffer);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                        eError = MPEG2VIDDEC_Fill_Data(pAppData,pBuffer);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                        eError = H264VIDDEC_Fill_Data(pAppData,pBuffer);
                    }
                    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                        eError = WMVVIDDEC_Fill_Data(pAppData,pBuffer);
                    }
#ifdef VIDDEC_SPARK_CODE
                    else if (VIDDEC_SPARKCHECK) {
                        eError = MPEG4VIDDEC_Fill_Data(pAppData,pBuffer);
                    }
#endif
                    if(pBuffer->nFilledLen > pBuffer->nAllocLen) {
                       APP_PRINT("Buffer size overflow alloc %x/fill %x\n",
                            (int)pBuffer->nAllocLen,(int)pBuffer->nFilledLen);
                    }
                    if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                        ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                        eError = OMX_ErrorHardware;
                        goto DEINIT;
                    }
                    if(pBuffer->nFilledLen != 0) {
                        ++pAppData->nCurrentFrameIn;
                        TickCounPropatation( pAppData, pBuffer, pAppData->nCurrentFrameIn);
                        if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                            APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pBuffer->nTickCount, (unsigned int)pBuffer);
                        }
                        else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                            APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n", pBuffer->nTimeStamp, (unsigned int)pBuffer);
                        }
                    }
                    APP_DPRINT(" Input Buff FRAME %d %x %x s(%d %x) - %x\n", (int)pAppData->nCurrentFrameIn,
                        (int)pBuffer, (int)pBuffer->nFlags, (int)pBuffer->nFilledLen, (int)pBuffer->nFilledLen, (int)pAppData->bInputEOS);
                    if((pBuffer->nFlags & OMX_BUFFERFLAG_EOS)){
                            pAppData->bInputEOS = OMX_TRUE;
                    }
                    eError = OMX_EmptyThisBuffer(pHandle, pBuffer);
                    if (eError != OMX_ErrorNone) {
                        ERR_PRINT ("Error OMX_EmptyThisBuffer function %x\n", eError);
                        goto DEINIT;
                    }
                }
            }
            if (FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) && (!pAppData->bDeinit) ) {
                FD_CLR(pAppData->OpBuf_Pipe[0], &rfds);
                ntimeoutcount = 0;
                read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                if(pBuf != NULL) {
                    APP_DPRINT("nflags %x  %x\n", (int)pBuf->nFlags, (int)pBuf->nFlags);
#ifndef __GET_BC_VOP__
                    if(pBuf->nFilledLen != 0) {
                        if(pAppData->nRenamingOut || !pAppData->nTimesCount) {
                            if((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY) == 0) {
                                APP_DPRINT("%d writing %x  %x\n",pAppData->nCurrentFrameOut,((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY)),pBuf->nFlags);
                                fwrite(pBuf->pBuffer, 1, pBuf->nFilledLen, pAppData->fOut);
                                fflush(pAppData->fOut);
                            }
                            else {
                                APP_DPRINT("%d erased %x  %x\n",pAppData->nCurrentFrameOut,((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY)),pBuf->nFlags);
                            }
                        }
                    }
#else
                    if( pAppData->nTestCase != 10 ) {
                        if(pBuf->nFilledLen != 0) {
                            if(pAppData->nRenamingOut || !pAppData->nTimesCount) {
                                if((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY) == 0) {
                                    APP_DPRINT("%d writing %x  %x\n",(int)(pAppData->nCurrentFrameOut),
                                        (unsigned int)((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY)),(unsigned int)pBuf->nFlags);
                                    fwrite(pBuf->pBuffer, 1, pBuf->nFilledLen, pAppData->fOut);
                                    fflush(pAppData->fOut);
                                }
                                else {
                                    APP_DPRINT("%d erased %x  %x\n",(int)(pAppData->nCurrentFrameOut),
                                        (unsigned int)((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY)),(unsigned int)pBuf->nFlags);
                                }
                            }
                        }
                    }
                    else {
                        OMX_U32 cBufferType = 0;
                        if ((pBuf->nFlags & FRAMETYPE_I_FRAME) != 0) {
                            cBufferType = I_FRAME;
                        }
                        else if ((pBuf->nFlags & FRAMETYPE_P_FRAME) != 0) {
                            cBufferType = P_FRAME;
                        }
                        else if ((pBuf->nFlags & FRAMETYPE_B_FRAME) != 0) {
                            cBufferType = B_FRAME;
                        }
                        else if ((pBuf->nFlags & FRAMETYPE_IDR_FRAME) != 0) {
                            cBufferType = IDR_FRAME;
                        }
                        else {
                            cBufferType = 1;
                        }
                        currentUseFrame = ((H264VDEC_UALGOutputParam *)(((VIDDEC_BUFFER_PRIVATE *)(pBuf->pOutputPortPrivate))->pUalgParam))->ulBytesConsumed;
                        if (currentUseFrame != 0){
                            totalUseFrame += currentUseFrame;
                            fprintf(pAppData->fvopData, "%d %d %d\n", totalUseFrame, (int)cBufferType, (int)pAppData->nCurrentFrameOut * rate);
                            if(pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC)
                            {
                                fwrite(&currentUseFrame, 1, 4, pAppData->fbcData);
                                if (pAppData->H264BCType == VIDDECTEST_FILLDATA_TYPE_FRAME_NAL_TWO_BE) {
                                    VIDDECTEST_SplitNAL( pBuf, (OMX_STRING)pAppData->szInFile, currentUseFrame, OMX_TRUE, OMX_FALSE);
                                }
                                else {
                                    VIDDECTEST_SplitNAL( pBuf, (OMX_STRING)pAppData->szInFile, currentUseFrame, OMX_FALSE, OMX_FALSE);
                                }
                            }
                        }
                    }
#endif
                    if (((pBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT) != 0) && ((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY) == 0))
                    {
                        ++pAppData->nCurrentFrameOutCorrupt;
                    }
                    if( pBuf->nFilledLen != 0  && ((pBuf->nFlags & OMX_BUFFERFLAG_DECODEONLY) == 0)) {
                        ++pAppData->nCurrentFrameOut;
                        ++pAppData->nDecodeFrms;
                    }
                    APP_DPRINT("Output Buffer FRAME %d %x  -  -  %x  -- %x\n", (int)pAppData->nCurrentFrameOut,(int)pBuf,(int)pBuf->nFilledLen,(int)pBuf->nFlags);
                    if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                        APP_PRINT("Out TickCount %d for output Buffer 0x%x\n", (int)pBuf->nTickCount, (unsigned int)pBuf);
                    }
                    else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                        APP_PRINT("Out TimeStamp %lld for output Buffer 0x%x\n", pBuf->nTimeStamp, (unsigned int)pBuf);
                    }
                    /* If the MB Error logic is enabled, print the MB Errors
                     * */
                    if (pAppData->MB_Error_Logic_Switch){
                       MBErrorPrintRoutine(pAppData->pHandle, &bMBErrorCount, pAppData);
                    }
                    if ((pBuf->nFlags & OMX_BUFFERFLAG_EOS) == 0) {
                        pBuf->nFlags &= ~(FRAMETYPE_MASK);
                        if (bDoStopResume || pAppData->nTestCase == TESTCASE_TYPE_FLUSHING){
                            nStreamFlag = OMX_FALSE;
                        }
                        eError = OMX_FillThisBuffer(pHandle, pBuf);
                        if (eError != OMX_ErrorNone) {
                            if (eError != OMX_ErrorHardware) {
                                ERR_PRINT ("Error OMX_FillThisBuffer function %x\n", (int)eError);
                            }
                            goto DEINIT;
                        }
                    }
                    else {
                        if (pAppData->nTestCase != TESTCASE_TYPE_PLAYAFTEREOS) {
                            pAppData->bDeinit = 1;
                            pBuf->nFlags = 0;
                        }
                        else {
                            if (nPassCount++ < FRAMECOUNTEOS) {
                                pAppData->bInputEOS = OMX_FALSE;
                                eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT("Error from SendCommand-Idle(Stop) State function %X\n", eError);
                                    goto FREEHANDLES;
                                }
                                eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateIdle);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT("Error:  VideoDec->VidDec_WaitForState has timed out %X\n", eError);
                                    goto FREEHANDLES;
                                }
                                int fdmax = maxint(pAppData->IpBuf_Pipe[0], pAppData->OpBuf_Pipe[0]);
                                fdmax = maxint(fdmax, pAppData->Error_Pipe[0]);
                                do
                                {
                                    FD_ZERO(&rfds);
                                    FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
                                    FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
                                    FD_SET(pAppData->Error_Pipe[0], &rfds);
                                    tv.tv_sec = 0;
                                    tv.tv_nsec = 100;

                                    sigemptyset (&set);
                                    sigaddset (&set, SIGALRM);
                                    retval = pselect (fdmax+1, &rfds, NULL, NULL,&tv, &set);
                                    sigdelset (&set, SIGALRM);

                                    if (retval == -1) {
                                        ERR_PRINT("select()");
                                        ERR_PRINT(" : Error \n");
                                        break;
                                    }

                                    if (retval == 0) {
                                        break;
                                    }

                                    if( FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) )
                                    {
                                        read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                                    }
                                    if( FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) )
                                    {
                                        read(pAppData->IpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                                    }
                                    if( FD_ISSET(pAppData->Error_Pipe[0], &rfds) )
                                    {
                                        read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
                                    }
                                } while(FD_ISSET(pAppData->Error_Pipe[0], &rfds) || FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) || FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds));


                                eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateExecuting, NULL);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT("Error from SendCommand-Executing State function %X\n", eError);
                                    goto DEINIT;
                                }
                                eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateExecuting);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT("Error:  VideoDec->VidDec_WaitForState has timed out %X\n", eError);
                                    goto FREEHANDLES;
                                }

                                eError = OMX_GetState(pHandle, &pAppData->eState);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
                                    goto DEINIT;
                                 }

                                for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
                                    pAppData->pOutBuff[i]->nFlags = 0;
                                    eError = OMX_FillThisBuffer(pHandle, pAppData->pOutBuff[i]);
                                    if (eError != OMX_ErrorNone) {
                                        if (eError != OMX_ErrorHardware) {
                                            ERR_PRINT ("Error OMX_FillThisBuffer function %x\n",eError);
                                        }
                                        goto DEINIT;
                                    }
                                }
                                pAppData->nRewind = 1;
                                for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
                                    if(pAppData->bInputEOS != VAL_ONE){
                                        pAppData->pInBuff[i]->nFlags = 0;
                                        if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                                            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                                             eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                        }
                                        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                                            eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                        }
                                        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                                            eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                        }
                                        else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                                            eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                        }
                    #ifdef VIDDEC_SPARK_CODE
                                        else if (VIDDEC_SPARKCHECK) {
                                            eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                        }
                    #endif
                                        if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                                            ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                                            eError = OMX_ErrorHardware;
                                            goto DEINIT;
                                        }
                                        ++pAppData->nCurrentFrameIn;
                                        TickCounPropatation( pAppData, pAppData->pInBuff[i], pAppData->nCurrentFrameIn);
                                        if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                                            APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[i]->nTickCount,
                                                (unsigned int)pAppData->pInBuff[i]);
                                        }
                                        else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                                            APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n",
                                                pAppData->pInBuff[i]->nTimeStamp,
                                                (unsigned int)(unsigned int)pAppData->pInBuff[i]);
                                        }

                                        eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[i]);
                                        if (eError != OMX_ErrorNone) {
                                            ERR_PRINT ("Error from OMX_EmptyThisBuffer function %x\n", eError);
                                            goto DEINIT;
                                        }
                                        if((pAppData->pInBuff[i]->nFlags & OMX_BUFFERFLAG_EOS)){
                                            pAppData->bInputEOS = 1;
                                            break;
                                        }

                                    }
                                }
                            }
                            else {
                                pAppData->bDeinit = 1;
                                pBuf->nFlags = 0;
                            }
                        }
                    }
                }
                else {
                    APP_DPRINT("Null buffer pointer\n");
                    pAppData->bDeinit = 1;
                    pBuf->nFlags = 0;
                }
            }
            if (FD_ISSET(pAppData->Error_Pipe[0], &rfds) && (!pAppData->bDeinit) ) {
                pAppData->bDeinit = OMX_TRUE;
                read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
            }
            if (pAppData->bDeinit) {
                pAppData->nDone = 1;
                if( pAppData->nTestCase != 10 ) {
                    if(pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                        VIDDECTEST_SplitNAL( pBuf, (OMX_STRING)pAppData->szInFile, 0, OMX_FALSE, OMX_TRUE);
                    }
                }

    #ifdef MEASURE_TIME
                time(&(pAppData->pEndTime->rawTime));
                pAppData->pEndTime->pTimeInfo = localtime(&(pAppData->pEndTime->rawTime));
                pAppData->pEndTime->nHrs = pAppData->pEndTime->pTimeInfo->tm_hour;
                pAppData->pEndTime->nMin = pAppData->pEndTime->pTimeInfo->tm_min;
                pAppData->pEndTime->nSec = pAppData->pEndTime->pTimeInfo->tm_sec;
                APP_DPRINT("End Time = hrs = %d, min = %d, sec = %d, frame = %d\n",
                                              pAppData->pEndTime->nHrs,
                                              pAppData->pEndTime->nMin,
                                              pAppData->pEndTime->nSec,
                                              ++pAppData->nCurrentFrameIn);
                calc_time(pAppData);
    #endif
#ifdef __GET_BC_VOP__
                if(pAppData->fvopData != NULL)
                {
                    fclose(pAppData->fvopData);
                    pAppData->fvopData = NULL;
                }

                if(pAppData->fbcData != NULL)
                {
                    fclose(pAppData->fbcData);
                    pAppData->fbcData = NULL;
                }
#endif
                /*send the output time*/

                eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateIdle, NULL);
                if (eError != OMX_ErrorNone) {
                    ERR_PRINT("Error from SendCommand-Idle(Stop) State function %X\n", eError);
                    goto DEINIT;
                }

                eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateIdle);
                if (eError != OMX_ErrorNone) {
                    ERR_PRINT("Error:  VideoDec->VidDec_WaitForState has timed out %X\n", eError);
                    goto DEINIT;
                }
                eError = OMX_GetState(pHandle, &pAppData->eState);
                if (eError != OMX_ErrorNone) {
                    ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
                    goto DEINIT;
                 }
                 /* Print the deblocking algorith status (Enabled/Disabled)
                  * */
                 eError = OMX_GetParameter(pAppData->pHandle,
                             OMX_IndexParamCommonDeblocking,
                             &(pAppData->deblockSwType));
                 if(eError != OMX_ErrorNone) {
                     APP_PRINT("Error getting the deblocking status\n");
                 }
                 else if (pAppData->deblockSwType.bDeblocking){
                     APP_PRINT("Deblocking is: ENABLED\n");
                 }
                 else{
                     APP_PRINT("Deblocking is: DISABLED\n");
                 }
                 /* If the MB Error Code Logic is Enabled Print the Error Num.
                  * */
                 /***************** MBError Code ********************/
                 if (pAppData->nTestCase == TESTCASE_TYPE_MBERRORCHECK || pAppData->MB_Error_Logic_Switch){
                    if (bMBErrorCount == 0){
                       APP_PRINT("MBErrors Didn't Occur\n");
                    }
                    else {
                       APP_PRINT("MBErrors: %d\n",(int)bMBErrorCount);
                    }
                 }
            }

            if (pAppData->nDone == 1) {
                eError = OMX_GetState(pHandle, &pAppData->eState);
                if (eError != OMX_ErrorNone) {
                    ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
                    goto DEINIT;
                 }
            }

            if (bDoPauseResume && (pAppData->nCurrentFrameOut > 0)) {
                if(((pAppData->nCurrentFrameOut % PAUSERESUMEFRAME) == 0)) {
                    if(nPassCount++ >= FRAMECOUNTMULT){
                        bDoPauseResume = OMX_FALSE;
                    }
                    else {
                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StatePause, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Pause State function %X\n", eError);
                            goto DEINIT;
                        }
                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StatePause);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }

                        sleep(SLEEPTIME);

                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateExecuting, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Executing State function %X\n", eError);
                            goto DEINIT;
                        }
                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateExecuting);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }
                    }
                }
            }

            if (pAppData->nTestCase == TESTCASE_TYPE_FLUSHING && (pAppData->nCurrentFrameOut > 0)) {
                if(((pAppData->nCurrentFrameOut % FLUSHINGFRAME) == 0) && !nStreamFlag) {
                    if(nPassCount++ < FRAMECOUNTFLUSH){
                        APP_PRINT("Flushing %d time\n",(int)nPassCount);
                        ++pAppData->nCurrentFrameOut;
                        pAppData->nRewind = 1;
                        nStreamFlag = OMX_TRUE;

                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StatePause, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Pause State function %X\n", eError);
                            goto DEINIT;
                        }
                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StatePause);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }

                        eError = OMX_SendCommand(pHandle,OMX_CommandFlush, -1, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Pause State function %X\n", eError);
                            goto DEINIT;
                        }
                        sem_wait( &pAppData->sWaitCommandReceived);

                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateExecuting, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Executing State function %X\n", eError);
                            goto DEINIT;
                        }

                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateExecuting);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }
                        /*clean the pipes of buffers*/
                        int fdmax = maxint(pAppData->IpBuf_Pipe[0], pAppData->OpBuf_Pipe[0]);
                        fdmax = maxint(fdmax, pAppData->Error_Pipe[0]);
                        do
                        {
                            FD_ZERO(&rfds);
                            FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
                            FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
                            FD_SET(pAppData->Error_Pipe[0], &rfds);

                            tv.tv_sec = 0;
                            tv.tv_nsec = 100000;

                            sigemptyset (&set);
                            sigaddset (&set, SIGALRM);
                            retval = pselect (fdmax+1, &rfds, NULL, NULL,&tv, &set);
                            sigdelset (&set, SIGALRM);

                            if (retval == -1) {
                                ERR_PRINT("select()");
                                ERR_PRINT(" : Error \n");
                                break;
                            }

                            if (retval == 0) {
                                break;
                            }

                            if( FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) )
                            {
                                read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                            }
                            if( FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) )
                            {
                                read(pAppData->IpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                            }
                            if( FD_ISSET(pAppData->Error_Pipe[0], &rfds) )
                            {
                                read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
                            }

                        } while(FD_ISSET(pAppData->Error_Pipe[0], &rfds) || FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) || FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds));

                        for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
                            pAppData->pOutBuff[i]->nFlags = 0;
                            eError = OMX_FillThisBuffer(pHandle, pAppData->pOutBuff[i]);
                            if (eError != OMX_ErrorNone) {
                                if (eError != OMX_ErrorHardware) {
                                    ERR_PRINT ("Error OMX_FillThisBuffer function %x\n",eError);
                                }
                                goto DEINIT;
                            }
                        }
                        pAppData->nRewind = 1;
                        for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
                            if(pAppData->bInputEOS != VAL_ONE){
                                pAppData->pInBuff[i]->nFlags = 0;
                                if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                                    pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                                     eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                                    eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                                    eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                                    eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
            #ifdef VIDDEC_SPARK_CODE
                                else if (VIDDEC_SPARKCHECK) {
                                    eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
            #endif
                                if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                                    ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                                    eError = OMX_ErrorHardware;
                                    goto DEINIT;
                                }
                                ++pAppData->nCurrentFrameIn;
                                TickCounPropatation( pAppData, pAppData->pInBuff[i], pAppData->nCurrentFrameIn);
                                if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                                    APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[i]->nTickCount,
                                        (unsigned int)pAppData->pInBuff[i]);
                                }
                                else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                                    APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n",
                                        pAppData->pInBuff[i]->nTimeStamp,
                                        (unsigned int)(unsigned int)pAppData->pInBuff[i]);
                                }

                                eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[i]);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT ("Error from OMX_EmptyThisBuffer function %x\n", eError);
                                    goto DEINIT;
                                }
                                if((pAppData->pInBuff[i]->nFlags & OMX_BUFFERFLAG_EOS)){
                                    pAppData->bInputEOS = 1;
                                    break;
                                }

                            }
                        }
                    }

                }
            }
            if (bDoStopResume && (pAppData->nCurrentFrameOut > 0)) {
                if(((pAppData->nCurrentFrameOut % STOPRESUMEFRAME) == 0) && !nStreamFlag) {
                    if(nPassCount++ >= FRAMECOUNTMULT){
                        bDoStopResume = OMX_FALSE;
                    }
                    else {
                        nStreamFlag = OMX_TRUE;
                        ++pAppData->nCurrentFrameOut;
                        pAppData->nRewind = 1;
                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateIdle, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Pause State function %X\n", eError);
                            goto DEINIT;
                        }
                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateIdle);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }

                        sleep(SLEEPTIME);

                        eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateExecuting, NULL);
                        if(eError != OMX_ErrorNone) {
                            ERR_PRINT ("Error from SendCommand-Executing State function %X\n", eError);
                            goto DEINIT;
                        }

                        eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateExecuting);
                        if (eError != OMX_ErrorNone) {
                            ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                            goto DEINIT;
                        }
                        /*clean the pipes of buffers*/
                        int fdmax = maxint(pAppData->IpBuf_Pipe[0], pAppData->OpBuf_Pipe[0]);
                        fdmax = maxint(fdmax, pAppData->Error_Pipe[0]);
                        do
                        {
                            FD_ZERO(&rfds);
                            FD_SET(pAppData->IpBuf_Pipe[0], &rfds);
                            FD_SET(pAppData->OpBuf_Pipe[0], &rfds);
                            FD_SET(pAppData->Error_Pipe[0], &rfds);
                            tv.tv_sec = 0;
                            tv.tv_nsec = 100000;

                            sigemptyset (&set);
                            sigaddset (&set, SIGALRM);
                            retval = pselect (fdmax+1, &rfds, NULL, NULL,&tv, &set);
                            sigdelset (&set, SIGALRM);
                            if (retval == -1) {
                                ERR_PRINT("select()");
                                ERR_PRINT(" : Error \n");
                                break;
                            }

                            if (retval == 0) {
                                break;
                            }
                            if( FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds) )
                            {
                                read(pAppData->OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                            }
                            if( FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) )
                            {
                                read(pAppData->IpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                            }
                            if( FD_ISSET(pAppData->Error_Pipe[0], &rfds) )
                            {
                                read(pAppData->Error_Pipe[0], &eError, sizeof(eError));
                            }

                        } while(FD_ISSET(pAppData->Error_Pipe[0], &rfds) || FD_ISSET(pAppData->IpBuf_Pipe[0], &rfds) || FD_ISSET(pAppData->OpBuf_Pipe[0], &rfds));

                        for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
                            eError = OMX_FillThisBuffer(pHandle, pAppData->pOutBuff[i]);
                            if (eError != OMX_ErrorNone) {
                                if (eError != OMX_ErrorHardware) {
                                    ERR_PRINT ("Error OMX_FillThisBuffer function %x\n",eError);
                                }
                                goto DEINIT;
                            }
                        }
                        for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
                            if(pAppData->bInputEOS != VAL_ONE){
                                if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
                                    pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
                                     eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
                                    eError = MPEG2VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                                    eError = H264VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
                                else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
                                    eError = WMVVIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
            #ifdef VIDDEC_SPARK_CODE
                                else if (VIDDEC_SPARKCHECK) {
                                    eError = MPEG4VIDDEC_Fill_Data(pAppData,pAppData->pInBuff[i]);
                                }
            #endif
                                if (eError != OMX_ErrorNone && eError != OMX_ErrorNoMore) {
                                    ERR_PRINT ("Error from Fill_Data function %x\n",eError);
                                    eError = OMX_ErrorHardware;
                                    goto DEINIT;
                                }
                                ++pAppData->nCurrentFrameIn;
                                TickCounPropatation( pAppData, pAppData->pInBuff[i], pAppData->nCurrentFrameIn);
                                if (pAppData->nTestCase == TESTCASE_TYPE_PROP_TICKCOUNT) {
                                    APP_PRINT("In TickCount %d for input Buffer 0x%x\n", (int)pAppData->pInBuff[i]->nTickCount,
                                        (unsigned int)pAppData->pInBuff[i]);
                                }
                                else if(pAppData->nTestCase == TESTCASE_TYPE_PROP_TIMESTAMPS) {
                                    APP_PRINT("In TimeStamp %lld for input Buffer 0x%x\n",
                                        pAppData->pInBuff[i]->nTimeStamp,
                                        (unsigned int)(unsigned int)pAppData->pInBuff[i]);
                                }

                                eError = OMX_EmptyThisBuffer(pHandle, pAppData->pInBuff[i]);
                                if (eError != OMX_ErrorNone) {
                                    ERR_PRINT ("Error from OMX_EmptyThisBuffer function %x\n", eError);
                                    goto DEINIT;
                                }
                                if((pAppData->pInBuff[i]->nFlags & OMX_BUFFERFLAG_EOS)){
                                    pAppData->bInputEOS = 1;
                                    break;
                                }

                            }
                        }
                    }

                }
            }
        } /* end while */

DEINIT:
        eError = OMX_GetState(pHandle, &pAppData->eState);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
            goto FREEHANDLES;
        }
        if(pAppData->eState == OMX_StateInvalid)
        {
            goto FREEBUFFERS;
        }
        if((pAppData->eState == OMX_StateExecuting) || (pAppData->eState == OMX_StatePause)){
            APP_DPRINT("Calling OMX_StateIdle\n");
            eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("Error from SendCommand-Idle(Stop) State function %X\n", eError);
                goto FREEHANDLES;
            }
            eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateIdle);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("Error:  VideoDec->VidDec_WaitForState has timed out %X\n", eError);
                goto FREEHANDLES;
            }
        }
        eError = OMX_GetState(pHandle, &pAppData->eState);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
            goto FREEHANDLES;
        }
        if((pAppData->eState == OMX_StateIdle)){
            APP_DPRINT("Calling OMX_StateLoaded\n");
            eError = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateLoaded, NULL);
            if(eError != OMX_ErrorNone) {
                ERR_PRINT ("Error from SendCommand-Idle State function %X\n", eError);
                goto FREEHANDLES;
            }

FREEBUFFERS:
            for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
                eError = OMX_FreeBuffer(pHandle, pAppData->pInPortDef->nPortIndex, pAppData->pInBuff[i]);
                if(eError != OMX_ErrorNone) {
                    ERR_PRINT ("Error from OMX_FreeBuffer Input %X\n", eError);
                    goto FREEHANDLES;
                }
            }
            for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
                eError = OMX_FreeBuffer(pHandle, pAppData->pOutPortDef->nPortIndex, pAppData->pOutBuff[i]);
                if(eError != OMX_ErrorNone) {
                    ERR_PRINT ("Error from OMX_FreeBuffer Output %X\n", eError);
                    goto FREEHANDLES;
                }
            }
            if(pAppData->eState == OMX_StateInvalid) {
                goto CLOSEFILES;
            }

            eError = VidDec_WaitForState(pHandle, pAppData, OMX_StateLoaded);
            if (eError != OMX_ErrorNone) {
                ERR_PRINT("Error:  VideoDec->WaitForState has timed out %X\n", eError);
                goto FREEHANDLES;
            }
        }
CLOSEFILES:
        eError = OMX_GetState(pHandle, &pAppData->eState);
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Warning:  VideoDec->GetState has returned status %X\n", eError);
            goto FREEHANDLES;
        }

        pAppData->nTimesCount++;
        if(pAppData->fOut != NULL)
        {
            eError = fflush(pAppData->fOut);
            if (eError == EOF) {
                ERR_PRINT("Failed to close the output file %X\n", eError);
                goto FREEHANDLES;
            }
            fclose(pAppData->fOut);
            pAppData->fOut = NULL;
        }

    #ifndef UNCER_CE
            if(chmod(fileOutName,0x1B6) == -1) {
                ERR_PRINT("Error: failed to erase the file <%s> for writing \n", fileOutName);
                goto EXIT;
            }
    #endif
        if ( (int)pAppData->nCurrentFrameOutCorrupt != 0 ) {
            APP_PRINT("-filename %s frames %d/corrupt %d mode %d\n",argv[2],(int)pAppData->nDecodeFrms, (int)pAppData->nCurrentFrameOutCorrupt,(int)pAppData->ProcessMode);
        }
        else {
            APP_PRINT("-filename %s frames %d mode %d\n",argv[2],(int)pAppData->nDecodeFrms, (int)pAppData->ProcessMode);
        }
    } while(pAppData->nTimesCount < pAppData->nTimes);

FREEHANDLES:
    if(eError != OMX_ErrorNone){
        pAppData->eAppError = eError;
    }

    if(pAppData->bCallGetHnd == OMX_TRUE){
        APP_DPRINT("Calling Freehandle\n");
        eError = TIOMX_FreeHandle(pHandle);
        if ((eError != OMX_ErrorNone)) {
            ERR_PRINT ("Error in Free Handle function %X\n", eError);
            goto EXIT;
        }

        /* De-Initialize OMX Core */
        APP_DPRINT("Calling Deinit\n");
        eError = TIOMX_Deinit();
        if (eError != OMX_ErrorNone) {
            ERR_PRINT("Failed to de-init OMX Core %X\n", eError);
            goto EXIT;
        }
    }

    retval = sem_destroy( &pAppData->sWaitCommandReceived);
    retval = sem_destroy( &pAppData->sWaitFirstBReturned);

EXIT:
    if(eError != OMX_ErrorNone){
        pAppData->eAppError = eError;
    }
#ifdef __GET_BC_VOP__
    if(vopname != NULL)
    {
        free(vopname);
        vopname = NULL;
    }
#endif
    if(fileOutName != NULL)
    {
        free(fileOutName);
        fileOutName = NULL;
    }

    /**
     * Closing files.
    **/
    APP_DPRINT("Closing files\n");
    if(pAppData->fIn != NULL)
        fclose(pAppData->fIn);
    if((pAppData->ProcessMode == 0)){
            if(pAppData->fBC != NULL){
                fclose(pAppData->fBC);
                pAppData->fBC = NULL;
            }
        }
    if(pAppData->fBC != NULL){
        fclose(pAppData->fBC);
        pAppData->fBC = NULL;
    }

    if(pAppData->ProcessMode == 1 && pAppData->H264BCType == VIDDECTEST_H264FILETYPE_NAL) {
        if(pAppData->fBC != NULL){
            fclose(pAppData->fBC);
            pAppData->fBC = NULL;
        }
    }
    APP_DPRINT("Free Resources\n");
    if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ||
            pAppData->eCompressionFormat == OMX_VIDEO_CodingH263) {
        MPEG4VIDDEC_FreeResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2) {
        MPEG2VIDDEC_FreeResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC) {
        H264VIDDEC_FreeResources(pAppData);
    }
    else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingWMV) {
        WMVVIDDEC_FreeResources(pAppData);
    }
#ifdef VIDDEC_SPARK_CODE
    else if (VIDDEC_SPARKCHECK) {
        MPEG4VIDDEC_FreeResources(pAppData);
    }
#endif
    if(eError != OMX_ErrorNone){
        pAppData->eAppError = eError;
    }
    else if(pAppData->eAppError != OMX_ErrorNone){
        eError = pAppData->eAppError;
    }
    if(pAppData->eAppError == OMX_ErrorNone){
        printf("App finished 0x%X\n", pAppData->eAppError);
    }
    else {
        printf("App finished with errors 0x%X\n", pAppData->eAppError);
    }

    if(pAppData->bCallGetHnd == OMX_TRUE){
        free(pAppData);
    }
    if (eError != OMX_ErrorNone) {
        return -1;
    }
    else {
        return 0;
    }
}

int ResourceExhaustationTest(int argc, char** argv)
{
    OMX_HANDLETYPE pHandle;
    MYDATATYPE* pAppData = NULL;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    char cFindstring[MAX_TEXT_STRING];
    char* pFindString = NULL;
    char* cTempChar = NULL;
    FILE* fCommand;
    char cCommandFilename[MAX_TEXT_STRING];
    char *cTempArgs[20];
    int i = 0;
    int iCount = 0;
    OMX_U32 iRound = 0;

    OMX_CALLBACKTYPE VidDecCaBa = {(void *)VidDec_EventHandler,
                                   (void *)VidDec_EmptyBufferDone,
                                   (void *)VidDec_FillBufferDone};

    if(argc != 4) {
        APP_PRINT("************************************************************************************\n");
        APP_PRINT("MPEG4-H263/H.264/WMV Decode Usage: \n");
        APP_PRINT("usage : %s <3> <input command file> <exit on error = 1, no exit = 0>\n", argv[0]);
        APP_PRINT("************************************************************************************\n");
        return -1;
    }

    /***************************************************/
    strcpy((char*)&cCommandFilename, argv[2]);
    fCommand = fopen((char*)cCommandFilename, "r");
    if (fCommand == NULL) {
        ERR_PRINT("Error: failed to open file <%s> for reading \n", cCommandFilename);
        eError = OMX_ErrorHardware;
        goto EXIT;
    }
    /***************************************************/
    pAppData = (MYDATATYPE *)malloc(sizeof(MYDATATYPE));
    if (pAppData == NULL) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    /***************************************************/
    pAppData->pCb = &VidDecCaBa;
    eError = TIOMX_Init();
    if (eError != OMX_ErrorNone) {
        ERR_PRINT("%d :: Error returned by TIOMX_Init() %x\n", __LINE__, eError);
        goto FREEHANDLES2;
    }
    eError = TIOMX_GetHandle(&pHandle, StrVideoDecoder, pAppData, pAppData->pCb);
    if ((eError != OMX_ErrorNone) || (pHandle == NULL)) {
        ERR_PRINT ("%x Error in Get Handle function %x\n", __LINE__, eError);
        goto EXIT;
    }
    pAppData->pHandle = pHandle;
    if (!pAppData->pHandle) {
        ERR_PRINT("Error in Handle Assignment\n");
        eError = OMX_ErrorInsufficientResources;
        goto FREEHANDLES;
    }
    /***************************************************/
    while(!feof(fCommand)) {
        iRound += 1;
        fgets( cFindstring, MAX_TEXT_STRING, fCommand);

        if(feof(fCommand))
            break;
        APP_PRINT("Round#%d - %s", (int)iRound, cFindstring);
        cTempChar = cFindstring;
        iCount = 0;
        pFindString = cTempChar;
        cTempChar = strtok(cTempChar, " ");
        while(cTempChar != NULL){
            pFindString = cTempChar;
            cTempChar = strtok(NULL, " ");
            cTempArgs[iCount] = malloc(250);
            if (cTempArgs[iCount] == NULL) {
                ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
                goto FREEHANDLES;
            }
            strcpy( cTempArgs[iCount], pFindString);
            iCount += 1;
            if(iCount > 19) {
                ERR_PRINT ("Error too many parameter in command line\n");
                break;
            }
        }
        if(iCount <= 19) {
            eError = NormalRunningTest( iCount, &cTempArgs[0], (MYDATATYPE*)pAppData);
            if ((eError != OMX_ErrorNone) && (atoi(argv[3]) != 0)) {
                ERR_PRINT ("Error code %X **Exiting**\n", eError);
                break;
            }
            if (eError != OMX_ErrorNone ) {
                ERR_PRINT("===RECOVERING FROM ERROR\n");
                eError = TIOMX_FreeHandle(pHandle);
                if ((eError != OMX_ErrorNone)) {
                    ERR_PRINT ("Error in Free Handle function %X\n", eError);
                    goto EXIT;
                }
                eError = TIOMX_GetHandle(&pHandle, StrVideoDecoder, pAppData, pAppData->pCb);
                if ((eError != OMX_ErrorNone) || (pHandle == NULL)) {
                    ERR_PRINT ("%x Error in Get Handle function %x\n", __LINE__, eError);
                    goto EXIT;
                }
                pAppData->pHandle = pHandle;
                if (!pAppData->pHandle) {
                    ERR_PRINT("Error in Handle Assignment\n");
                    eError = OMX_ErrorInsufficientResources;
                    goto FREEHANDLES;
                }
            }
        }
        else {
            ERR_PRINT ("Error running command line \n %s\n",cFindstring);
        }
        for(i = 0; i < iCount; i++){
            free(cTempArgs[i]);
        }
    }
    /***************************************************/
FREEHANDLES:
    APP_DPRINT("Calling Freehandle\n");
    eError = TIOMX_FreeHandle(pHandle);
    if ((eError != OMX_ErrorNone)) {
        ERR_PRINT ("Error in Free Handle function %X\n", eError);
        goto EXIT;
    }
FREEHANDLES2:
    /* De-Initialize OMX Core */
    APP_DPRINT("Calling Deinit\n");
    eError = TIOMX_Deinit();
    if (eError != OMX_ErrorNone) {
        ERR_PRINT("Failed to de-init OMX Core %X\n", eError);
        goto EXIT;
    }
    /***************************************************/

EXIT:
    /**
     * Closing files.
    **/
    APP_DPRINT("Closing files\n");
    if(fCommand != NULL)
        fclose(fCommand);

    free(pAppData);

    if (eError != OMX_ErrorNone)
        return -1;
    else
        return 0;
}

