
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
#include <time.h>
#include <sys/stat.h>
#include "VidDecTest.h"
#include "MPEG4DecFunctions.h"
#include <stdio.h>

OMX_ERRORTYPE MPEG4VIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (pAppData->ProcessMode == PROCESS_MODE_TYPE_FRAME) {
        eError = VIDDECTEST_FillData( pAppData, pBuf, VIDDECTEST_FILLDATA_TYPE_FRAME_VOP);
    }
    else {
        eError = VIDDECTEST_FillData( pAppData, pBuf, VIDDECTEST_FILLDATA_TYPE_STREAM);
    }
    return eError;
}

OMX_ERRORTYPE MPEG4VIDDEC_AllocateResources(MYDATATYPE* pAppData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 retval;

    pAppData->pInPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if (!pAppData->pInPortDef) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    memset(pAppData->pInPortDef, VAL_ZERO, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    pAppData->pOutPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if (!pAppData->pOutPortDef) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    memset(pAppData->pOutPortDef, VAL_ZERO, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    pAppData->pMpeg4 = (OMX_VIDEO_PARAM_MPEG4TYPE*)malloc(sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
    if (!pAppData->pMpeg4) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    memset(pAppData->pMpeg4, VAL_ZERO, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));

    /* Create a pipe used to queue data from the callback. */
    retval = pipe(pAppData->IpBuf_Pipe);
    if (retval != OMX_FALSE) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    retval = pipe(pAppData->OpBuf_Pipe);
    if (retval != OMX_FALSE) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    retval = pipe(pAppData->Error_Pipe);
    if (retval != 0) {
        ERR_PRINT("%s :OMX_ErrorInsufficientResources: %d\n", __FUNCTION__, __LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /* For Time Stamps */
    pAppData->pStartTime = (APP_TIME*)calloc(1, sizeof(APP_TIME));
    if(pAppData->pStartTime != NULL) {
        pAppData->pEndTime = (APP_TIME*)calloc(1, sizeof(APP_TIME));
        if(pAppData->pEndTime != NULL) {
            pAppData->pStartTime->bInitTime = OMX_TRUE;
        }
        else {
            eError = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
    }
    else {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

EXIT:
    return eError;
}


OMX_ERRORTYPE MPEG4VIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    VIDEODEC_PORT_INDEX sVidDecPortIndex;
    OMX_HANDLETYPE pHandle = pAppData->pHandle;
    if(!pHandle){
        eError = OMX_ErrorUndefined;
        goto EXIT;
    }

    eError = GetVidDecPortDef (pAppData->pHandle, &sVidDecPortIndex);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    /* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (input) */
    memset(pAppData->pInPortDef, VAL_ZERO, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    pAppData->pInPortDef->nSize                     = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pAppData->pInPortDef->nVersion.s.nVersionMajor  = VERSION_MAJOR;
    pAppData->pInPortDef->nVersion.s.nVersionMinor  = VERSION_MINOR;
    pAppData->pInPortDef->nVersion.s.nRevision      = VERSION_REVISION;
    pAppData->pInPortDef->nVersion.s.nStep          = VERSION_STEP;
    pAppData->pInPortDef->nPortIndex                = sVidDecPortIndex.nInputPortIndex;

    eError = OMX_GetParameter(pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pInPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    pAppData->pInPortDef->eDir                                  = OMX_DirInput;
    pAppData->pInPortDef->nBufferCountActual                    = pAppData->cInputBuffers;
    pAppData->pInPortDef->nBufferCountMin                       = BUFFERMINCOUNT;
    pAppData->pInPortDef->bEnabled                              = PORTENABLED;
    pAppData->pInPortDef->bPopulated                            = PORTPOPULATED;
    pAppData->pInPortDef->eDomain                               = PORTDOMAIN;

    /* OMX_VIDEO_PORTDEFINITION values for input port */
    pAppData->pInPortDef->format.video.cMIMEType                = MIMETYPEMPEG4;
    pAppData->pInPortDef->format.video.pNativeRender            = INPORTNATIVERENDER;
    pAppData->pInPortDef->format.video.nFrameWidth              = pAppData->nWidth;
    pAppData->pInPortDef->format.video.nFrameHeight             = pAppData->nHeight;
    pAppData->pInPortDef->format.video.nStride                  = INPORTSTRIDE;
    pAppData->pInPortDef->format.video.nSliceHeight             = INPORTSLICEHEIGHT;
    pAppData->pInPortDef->format.video.eCompressionFormat       = pAppData->eCompressionFormat;
    pAppData->pInPortDef->format.video.bFlagErrorConcealment    = INPORTFLAGERRORCONCEALMENT;
    pAppData->pInPortDef->format.video.eColorFormat             = PORTCOMPRESSIONFORMATUNUSED;

    eError = OMX_SetParameter (pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pInPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    eError = OMX_GetParameter (pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pInPortDef);
    if(eError != OMX_ErrorNone) {
        goto EXIT;
    }

    /* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (output) */
    memset(pAppData->pOutPortDef, VAL_ZERO, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    pAppData->pOutPortDef->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pAppData->pOutPortDef->nVersion.s.nVersionMajor             = VERSION_MAJOR;
    pAppData->pOutPortDef->nVersion.s.nVersionMinor             = VERSION_MINOR;
    pAppData->pOutPortDef->nVersion.s.nRevision                 = VERSION_REVISION;
    pAppData->pOutPortDef->nVersion.s.nStep                     = VERSION_STEP;
    pAppData->pOutPortDef->nPortIndex                           = sVidDecPortIndex.nOutputPortIndex;

    eError = OMX_GetParameter(pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pOutPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    pAppData->pOutPortDef->eDir                                 = OMX_DirOutput;
    pAppData->pOutPortDef->nBufferCountActual                   = pAppData->cOutputBuffers;
    pAppData->pOutPortDef->nBufferCountMin                      = BUFFERMINCOUNT;
    pAppData->pOutPortDef->bEnabled                             = PORTENABLED;
    pAppData->pOutPortDef->bPopulated                           = PORTPOPULATED;
    pAppData->pOutPortDef->eDomain                              = PORTDOMAIN;

    /* OMX_VIDEO_PORTDEFINITION values for output port */
    pAppData->pOutPortDef->format.video.cMIMEType               = MIMETYPEYUV;
    pAppData->pOutPortDef->format.video.pNativeRender           = OUTPORTNATIVERENDER;
    pAppData->pOutPortDef->format.video.nFrameWidth             = pAppData->nWidth;
    pAppData->pOutPortDef->format.video.nFrameHeight            = pAppData->nHeight;
    pAppData->pOutPortDef->format.video.nStride                 = OUTPORTSTRIDE;
    pAppData->pOutPortDef->format.video.nSliceHeight            = OUTPORTSLICEHEIGHT;
    pAppData->pOutPortDef->format.video.nBitrate                = OUTPORTBITRATE;
    pAppData->pOutPortDef->format.video.xFramerate              = OUTPORTFRAMERATE;
    pAppData->pOutPortDef->format.video.bFlagErrorConcealment   = OUTPORTFLAGERRORCONCEALMENT;
    pAppData->pOutPortDef->format.video.eCompressionFormat      = PORTCOMPRESSIONFORMATOUTPUT;
    pAppData->pOutPortDef->format.video.eColorFormat            = pAppData->eColorFormat;

    eError = OMX_SetParameter (pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pOutPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    eError = OMX_GetParameter (pAppData->pHandle, OMX_IndexParamPortDefinition, pAppData->pOutPortDef);
    if(eError != OMX_ErrorNone) {
        goto EXIT;
    }

EXIT:
    return eError;
}

void MPEG4VIDDEC_FreeResources(MYDATATYPE* pAppData)
{
    close(pAppData->Error_Pipe[0]);
    close(pAppData->Error_Pipe[1]);
    close(pAppData->IpBuf_Pipe[0]);
    close(pAppData->IpBuf_Pipe[1]);
    close(pAppData->OpBuf_Pipe[0]);
    close(pAppData->OpBuf_Pipe[1]);

    if (pAppData->pInPortDef != NULL){
        free(pAppData->pInPortDef);
        pAppData->pInPortDef = NULL;
    }
    if (pAppData->pOutPortDef != NULL){
        free(pAppData->pOutPortDef);
        pAppData->pOutPortDef = NULL;
    }
    if (pAppData->pMpeg4 != NULL){
        free(pAppData->pMpeg4);
        pAppData->pMpeg4 = NULL;
    }
    if (pAppData->pStartTime != NULL){
        free(pAppData->pStartTime);
        pAppData->pStartTime = NULL;
    }
    if (pAppData->pEndTime != NULL){
        free(pAppData->pEndTime);
        pAppData->pEndTime = NULL;
    }
}

