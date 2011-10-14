
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
* @file H264DecFunctions.h
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
#ifndef H264FUNC_H
#define H264FUNC_H

#include "VidDecTest.h"

#define OMX_H264DEC_NonMIME 1

OMX_ERRORTYPE H264VIDDEC_Fill_Data(MYDATATYPE* pAppData, OMX_BUFFERHEADERTYPE* pBuf);
OMX_ERRORTYPE H264VIDDEC_SetParamPortDefinition(MYDATATYPE* pAppData);
OMX_ERRORTYPE H264VIDDEC_AllocateResources(MYDATATYPE* pAppData);
void H264VIDDEC_FreeResources(MYDATATYPE* pAppData);

#endif /*H264FUNC_H*/
