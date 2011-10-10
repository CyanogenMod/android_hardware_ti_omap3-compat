/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkTime_DEFINED
#define SkTime_DEFINED

#include "SkTypes.h"

/** \class SkTime
    Platform-implemented utilities to return time of day, and millisecond counter.
*/
class SkTime {
public:
    struct DateTime {
        uint16_t fYear;          //!< e.g. 2005
        uint8_t  fMonth;         //!< 1..12
        uint8_t  fDayOfWeek;     //!< 0..6, 0==Sunday
        uint8_t  fDay;           //!< 1..31
        uint8_t  fHour;          //!< 0..23
        uint8_t  fMinute;        //!< 0..59
        uint8_t  fSecond;        //!< 0..59
    };
    static void GetDateTime(DateTime*);

    static SkMSec GetMSecs();
};

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_WIN32)
    extern SkMSec gForceTickCount;
#endif

#define SK_TIME_FACTOR      1

///////////////////////////////////////////////////////////////////////////////

class SkAutoTime {
public:
    // The label is not deep-copied, so its address must remain valid for the
    // lifetime of this object
    SkAutoTime(const char* label = NULL, SkMSec minToDump = 0) : fLabel(label)
    {
        fNow = SkTime::GetMSecs();
        fMinToDump = minToDump;
    }
    ~SkAutoTime()
    {
        SkMSec dur = SkTime::GetMSecs() - fNow;
        if (dur >= fMinToDump) {
            SkDebugf("%s %d\n", fLabel ? fLabel : "", dur);
        }
    }
private:
    const char* fLabel;
    SkMSec      fNow;
    SkMSec      fMinToDump;
};

class AutoTimeMicros {
public:
    AutoTimeMicros(const char label[]) : fLabel(label) {
        if (!fLabel) {
            fLabel = "";
        }
        gettimeofday(&fNow, NULL);
        width = 0;
        height = 0;
        scale_factor = -1;
    }
    ~AutoTimeMicros() {
        struct timeval tv;
        unsigned long timeDiff = 0;
        char *str = NULL;
        gettimeofday(&tv, NULL);
        timeDiff = (unsigned long)((tv.tv_sec - fNow.tv_sec) * 1000000) + (tv.tv_usec - fNow.tv_usec);

        str =(char*) malloc( (strlen(fLabel)+50) );
        if (str){
            strcpy(str, fLabel);
            if(width != 0 && height !=0)
            {
                sprintf(str, "%s Input(%d x %d)", str, width, height);
            }
            if(scale_factor != -1)
            {
                sprintf(str, "%s ScaleFactor(%d)", str, scale_factor);
            }
            SkDebugf("---- Time (ms): %s %lu us\n", str, timeDiff);
            free(str);
        }
    }

    void setResolution(int width, int height){
        this->width=width;
        this->height=height;
    }

    void setScaleFactor(int scale_factor){
        this->scale_factor=scale_factor;
    }

private:
    const char* fLabel;
    struct timeval fNow;
    int width;
    int height;
    int scale_factor;
};

 ///////////////////////////////////////////////////////////////////////////////
#endif

