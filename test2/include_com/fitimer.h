#pragma once

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <assert.h>
#include <set>
#include "libcfg.h"
#include "difstypes.h"
#include "OSThread.h"


using namespace std;

typedef void (*FiTimerCallback)(int, void *);

struct FiTimerParam {
        int         id;
        timer_t     tid;
        void *pObj;

        void *pContext;
        FiTimerCallback pfnTimerCallBack;

        FiTimerParam() {
                id = 0;
                pObj = NULL;
                pContext = NULL;
                pfnTimerCallBack = NULL;
        }
};

/**
 * ??????
 * 
 * ??????????
 * 1. ????ontimer?????
 * 2. ???settimer??????
 * 
 * @author like
 */
class FiTimer {
public:
        FiTimer();
        /**
         * ??????killtimer
         */
        virtual ~FiTimer();

        /**
         * ?????
         * 
         * @param nSeconds   ????
         * @param pfCallBack ??????????;?????NULL????????????ontimer????????FiTimer????.
         * @param pContext   ??????
         * 
         * @return ??????timerid?killtimer???
         *         ??0???????
         */
        int             SetTimer(int nSeconds, FiTimerCallback pfCallBack = NULL, void *pContext = NULL, long nNanoSec = 0);

        /**
         * ?????
         * 
         * @param nId    SetTimer??????id
         */

        void            KillTimer(int nId);

        /**
         * ????FiTime???????????
         * 
         * @param id     SetTimer???????id
         */
        virtual void    OnTimer(int id);

private:
        static void TimerFunc(union sigval  v);

        void        KillTimerNolock(int nId);
        void        Addref();
        void        DecRef();

private:
        map<int, FiTimerParam *>    m_timerParam;
        static  set<void *>          s_objList;
        static  set<int>            s_nActvieIdList;
        static  u32                 s_nTimerId;
        static  CCritSec            m_lock;
        c_event                     m_refEvt;
        int                         m_refCnt;
};

namespace FiTimerNS {

extern map<int, FiTimerParam *>    gTimerParam;

FiTimerParam*    Find(int nId);

};
