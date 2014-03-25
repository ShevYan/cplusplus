#include "fitimer.h"

u32 FiTimer::s_nTimerId = 1;
CCritSec FiTimer::m_lock;
set<void *> FiTimer::s_objList;
set<int>   FiTimer::s_nActvieIdList;

FiTimer::FiTimer() {
        m_refEvt.set_mode(true);
        m_refEvt.set();
        m_refCnt = 0;

        m_lock.Lock();
        s_objList.insert(this);
        m_lock.Unlock();
}

FiTimer::~FiTimer() {
        m_lock.Lock();
        s_objList.erase(this);
        m_lock.Unlock();

        m_refEvt.wait(-1);
}

void FiTimer::KillTimer(int nId) {
        m_lock.Lock();
        KillTimerNolock(nId);
        m_lock.Unlock();
}

void FiTimer::KillTimerNolock(int nId) {
        FiTimerParam *pTimerParam = FiTimerNS::Find(nId);
        if (NULL == pTimerParam) {
                return;
        }

        timer_delete(pTimerParam->tid);
        delete FiTimerNS::gTimerParam[nId];
        FiTimerNS::gTimerParam.erase(nId);
}

int FiTimer::SetTimer(int nSeconds, FiTimerCallback pfCallBack, void *pContext, long nNanoSec) {
        FiTimerParam *pTimer = NULL;
        int                 nTimerId = 0;
        bool                bCreateTimerSuc = false;
        struct sigevent     se;
        struct itimerspec   ts,   ots;

        __try_c{
                m_lock.Lock();
                pTimer = new FiTimerParam();
                pTimer->id = s_nTimerId++;
                pTimer->pfnTimerCallBack = pfCallBack;
                pTimer->pContext = pContext;


                pTimer->pObj = this;
                assert(FiTimerNS::gTimerParam.find(pTimer->id) == FiTimerNS::gTimerParam.end());
                FiTimerNS::gTimerParam[pTimer->id] = pTimer;

                memset(&se, 0, sizeof(se));
                se.sigev_notify             =  SIGEV_THREAD;
                se.sigev_notify_function    =  TimerFunc;
                se.sigev_value.sival_int = pTimer->id;
                if (timer_create(CLOCK_REALTIME,   &se,   &pTimer->tid) <   0) {
                        __leave;
                }

                bCreateTimerSuc = true;

                ts.it_value.tv_sec   =   0;
                ts.it_value.tv_nsec   =   1;
                ts.it_interval.tv_sec   =   nSeconds;
                ts.it_interval.tv_nsec   =   nNanoSec;
                if (timer_settime(pTimer->tid,   TIMER_ABSTIME,   &ts,   &ots) <   0) {
                        __leave;
                }

                nTimerId = pTimer->id;

        } __finally{

                if (0 == nTimerId && bCreateTimerSuc) {
                        KillTimerNolock(pTimer->id);
                }

                m_lock.Unlock();
        }

        return nTimerId;
}

void FiTimer::TimerFunc(union sigval  v) {
        m_lock.Lock();

        FiTimer      *pClass  = NULL;
        FiTimerParam *pTimerParam   = (FiTimerParam *)FiTimerNS::Find(v.sival_int);
        if (NULL != pTimerParam) {
                pClass = (FiTimer *)pTimerParam->pObj;
        }

        if (NULL == pTimerParam 
            || s_objList.find(pClass) == s_objList.end() || NULL == pClass
            || s_nActvieIdList.find(pTimerParam->id) != s_nActvieIdList.end()) {

                m_lock.Unlock();
                return;
        }

        int          nId            = pTimerParam->id;
        void         *pContext      = pTimerParam->pContext;
        FiTimerCallback     pfn     = pTimerParam->pfnTimerCallBack;

        pClass->Addref();
        s_nActvieIdList.insert(nId);
        m_lock.Unlock();

        if (NULL != pfn) {
                pfn(nId, pContext);
        } else {
                pClass->OnTimer(nId);
        }

        m_lock.Lock();
        //???????????????????pClass??deref?????????????FiTimer??????????????s_nActvieIdList???????????s_nActvieIdList??????
        s_nActvieIdList.erase(nId);
        pClass->DecRef();
        m_lock.Unlock();

}

void FiTimer::Addref() {
        m_refCnt++;
        m_refEvt.reset();
}

void FiTimer::DecRef() {
        m_refCnt--;
        if (0 == m_refCnt) {
                m_refEvt.set();
        }
}

void FiTimer::OnTimer(int id) {
        time_t   t;
        char   p[32];
        time(&t);
        strftime(p,   sizeof(p),   "%T",   localtime(&t));
        printf("%s  thread  %lu,  timerid  =  %d,  captured.\n",   p,   pthread_self(), id);
        return;
}


namespace FiTimerNS {

map<int, FiTimerParam *>    gTimerParam;

FiTimerParam* Find(int nId) {
        map<int, FiTimerParam *>::iterator it = gTimerParam.find(nId);
        if (it == gTimerParam.end()) {
                return NULL;
        }

        return it->second;
}

};
