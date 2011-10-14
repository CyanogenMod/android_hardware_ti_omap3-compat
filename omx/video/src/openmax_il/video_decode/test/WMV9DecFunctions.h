
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
* @file WMV9DecFunctions.h
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
#ifndef WMV9FUNC_H
#define WMV9FUNC_H

#include "VidDecTest.h"

#define OMX_WMV9VIDDEC_NonMIME  1

/*function prototypes*/
OMX_ERRORTYPE WMVVIDDEC_Fill_Data(MYDATATYPE* pAppData,OMX_BUFFERHEADERTYPE *pBuf);
OMX_ERRORTYPE WMVVIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
OMX_ERRORTYPE WMVVIDDEC_AllocateResources(MYDATATYPE* pAppData);
void WMVVIDDEC_FreeResources(MYDATATYPE* pAppData);

#endif /*H264FUNC_H*/
