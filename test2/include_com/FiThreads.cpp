#include "FiThreads.h"

CCritSec FiWorkerQueue::s_lock;
FiWorkerQueue *volatile FiWorkerQueue::s_pInstance = NULL;

int fi_create_thread(pthread_t *__newthread, void* (*start)(void *), void *__arg) {

        int ret = 0;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        ret = pthread_create(__newthread, &attr, start, __arg);
        pthread_attr_destroy(&attr);

        return ret;
}


FiThreads::FiThreads() {
        m_bRun = false;

        m_nDefaultMaxTaskEntryNum = 1000000;
        m_nDefaultMaxThreadNumPerTask = 2;
        m_nDefaultMaxTaskNum = 10000000;
        m_nThreadNum = 10;
        m_strConfigPath = "/FiThreads";
        m_nTaskNum = 0;
        m_bPauseReceiveTask = true;

        m_pfnDefaultCallBack = NULL;
        m_pFiCxt = CFiContext::GetInstance();
}


FiThreads::~FiThreads() {
        ClearAllTaskWhenDestruct();
}

void  FiThreads::OnTimerCheckTaskFinish(int nTimerId) {
        if (m_nWaitAllTaskFinishTimerId == nTimerId) {
                m_lock.Lock();
                if (0 == m_taskidToTaskMap.size() &&  0 == m_busyTaskIdToBusyNum.size()) {
                        m_waitAllTaskFinish.Set();
                }
                m_lock.Unlock();
        }
}

void FiThreads::WaitAllTaskFinish() {
        FiTimer  timerCheck;

        m_waitAllTaskFinish.Reset();
        m_nWaitAllTaskFinishTimerId = timerCheck.SetTimer(0, PfnTimerCheckTaskFinish, this, 50 * 1000 * 1000);
        m_waitAllTaskFinish.Wait();
        timerCheck.KillTimer(m_nWaitAllTaskFinishTimerId);
}

void FiThreads::ClearAllTaskWhenDestruct() {
        m_lock.Lock();
        m_bRun = false;

        for (int i = 0; i < m_hThreadSet.size(); i++) {
                m_sem.Post();
        }

        for (map<u64, list<FiThreadTask*> >::iterator itTaskList = m_taskidToTaskMap.begin(); itTaskList != m_taskidToTaskMap.end(); itTaskList++) {
                for (list<FiThreadTask *>::iterator itTask = itTaskList->second.begin(); itTask != itTaskList->second.end(); itTask++) {
                        if (NULL != (*itTask)->pEvent) {
                                (*itTask)->pEvent->Set();
                        }
                        delete *itTask;
                }
        }

        m_lock.Unlock();

        for (set<pthread_t>::iterator it = m_hThreadSet.begin(); it != m_hThreadSet.end(); it++) {
                void *status;
                pthread_join(*it, &status);
        }
        m_hThreadSet.clear();
}

u32 FiThreads::GetMaxThreadNumForTask(u64 nTaskId){
        map<u64, u32>::iterator it = m_nSpecTaskIdToMaxThreadNumMap.find(nTaskId);
        if (it != m_nSpecTaskIdToMaxThreadNumMap.end()) {
                return it->second;
        }else{
                return m_pFiCxt->GetNum(m_strConfigPath + "/ThreadNumPerTask");
        }
}

FiErr FiThreads::PushTask(FiThreadTask *pTask, bool& bRefTask) {
        FiErr err;

        __try_c {
                m_lock.Lock();

                if (!m_bRun || m_bPauseReceiveTask) {
                        err = FErr(MFics_ThrdPoolStop);
                        __leave;
                }

                if (m_nTaskNum >= m_pFiCxt->GetNum(m_strConfigPath + "/MaxTaskNum")) {
                        err = FErr(MFics_ThrdPoolExcdMaxTask);
                        __leave;
                }

                if ((m_taskidToTaskMap.find(pTask->nTaskId) != m_taskidToTaskMap.end() && m_taskidToTaskMap[pTask->nTaskId].size() >= m_pFiCxt->GetNum(m_strConfigPath + "/MaxTaskEntryNum"))) {
                        err = FErr(MFics_ThrdPoolExcdEntry);
                        __leave;
                }

                if (m_busyTaskIdToWaiters.find(pTask->nTaskId) != m_busyTaskIdToWaiters.end()) {
                        err = FErr(MFics_ThrdPoolTaskClr);
                        __leave;
                }

                if (err.IsErr()) {
                        bRefTask = false;
                        if (NULL != pTask->pEvent) {
                                pTask->pEvent->Set();
                        }
                        __leave;
                }

                if (pTask->bMerged && m_taskidToMegeredTaskMap.find(pTask->nTaskId) != m_taskidToMegeredTaskMap.end()) {
                        m_taskidToMegeredTaskMap[pTask->nTaskId]->clone(pTask);
                        bRefTask = false;
                        err = FSucCode;
                        __leave;
                }

                pTask->itInTaskList.PushBack(m_taskList, pTask);

                pTask->itInTaskIdMapList.PushBack(m_taskidToTaskMap[pTask->nTaskId], pTask);

                if (pTask->bMerged) {
                        if (m_taskidToMegeredTaskMap.find(pTask->nTaskId) != m_taskidToMegeredTaskMap.end()) {
                                delete m_taskidToMegeredTaskMap[pTask->nTaskId];
                        }
                        m_taskidToMegeredTaskMap[pTask->nTaskId] = pTask;
                }

                m_nTaskNum++;

                m_sem.Post();
                bRefTask = true;
                err = FSucCode;

        }__finally{
                if (err.IsErr()) {
                        bRefTask = false;
                }

                m_lock.Unlock();
        }

        return err;
}

FiErr FiThreads::PushTask(u64 nTaskId, void *pContext, FiThreadPfn pfnCallBack, FiEvent *pEvent) {

        bool bRefTask = false;
        FiErr err;

        FiThreadTask *pTask = new FiThreadTask();
        pTask->nTaskId = nTaskId;
        pTask->pContext = pContext;
        pTask->pEvent = pEvent;
        pTask->pfnCallBack = pfnCallBack == NULL ? m_pfnDefaultCallBack : pfnCallBack;
        pTask->bMerged = false;
        pTask->bDelContextAfterMerge = false;

        err = PushTask(pTask, bRefTask);
        if (!bRefTask) {
                delete pTask;
        }

        return err;
}

FiErr FiThreads::PushMergedTask(u64 nTaskId, void *pContext, FiThreadPfn pfnCallBack, bool bDelContextAfterMerge) {
        bool bRefTask = false;
        FiErr err;

        FiThreadTask *pTask = new FiThreadTask();
        pTask->nTaskId = nTaskId;
        pTask->pContext = pContext;
        pTask->pEvent = NULL;
        pTask->pfnCallBack = pfnCallBack == NULL ? m_pfnDefaultCallBack : pfnCallBack;
        pTask->bMerged = true;
        pTask->bDelContextAfterMerge = bDelContextAfterMerge;

        err = PushTask(pTask, bRefTask);
        if (!bRefTask) {
                delete pTask;
        }

        return err;
}

void FiThreads::TaskProc() {
        u64 nTaskId = 0;

        while (true) {
                m_sem.Wait();
                m_lock.Lock();

                if (!m_bRun) {
                        m_lock.Unlock();
                        return;
                }
                if (m_taskList.empty()) {
                        m_lock.Unlock();
                        continue;
                }
                FiThreadTask *pTask = *m_taskList.begin();
                pTask->itInTaskList.Erase();

                while (true) {
                        nTaskId = pTask->nTaskId;

                        if (m_busyTaskIdToBusyNum.find(nTaskId) != m_busyTaskIdToBusyNum.end()
                            && m_busyTaskIdToBusyNum[nTaskId] >= GetMaxThreadNumForTask(nTaskId)) {
                                pTask->itInWaitTaskList.PushBack(m_taskidToWaitTaskMap[nTaskId], pTask);
                                break;
                        }
                        m_busyTaskIdToBusyNum[nTaskId]++;

                        if (!pTask->itInTaskIdMapList.IsNull()) {
                                assert(m_taskidToTaskMap.find(pTask->nTaskId) != m_taskidToTaskMap.end());
                                pTask->itInTaskIdMapList.Erase();
                                if (0 == m_taskidToTaskMap[pTask->nTaskId].size()) {
                                        m_taskidToTaskMap.erase(pTask->nTaskId);
                                }
                        }

                        if (pTask->bMerged) {
                                m_taskidToMegeredTaskMap.erase(pTask->nTaskId);
                                pTask->bMerged = false;
                        }

                        m_lock.Unlock();

                        if (NULL != pTask->pfnCallBack) {
                                pTask->pfnCallBack(nTaskId, pTask->pContext);
                        } else {
                                OnTaskProc(nTaskId, pTask->pContext);
                        }

                        m_lock.Lock();

                        if (m_busyTaskIdToWaiters.find(nTaskId) != m_busyTaskIdToWaiters.end()) {
                                list<FtBusyTaskWaitUnit *>& waitList = m_busyTaskIdToWaiters[nTaskId];
                                for (list<FtBusyTaskWaitUnit *>::iterator it = waitList.begin(); it != waitList.end(); it++) {
                                        (*it)->pEvent->Set();
                                }
                                m_busyTaskIdToWaiters[nTaskId].clear();
                                m_busyTaskIdToWaiters.erase(nTaskId);
                        }

                        m_busyTaskIdToBusyNum[nTaskId]--;
                        if (0 == m_busyTaskIdToBusyNum[nTaskId]) {
                                m_busyTaskIdToBusyNum.erase(nTaskId);
                        }

                        DeleteTask(pTask);

                        map<u64, list<FiThreadTask*> >::iterator waitTaskIt = m_taskidToWaitTaskMap.find(nTaskId);
                        if (waitTaskIt != m_taskidToWaitTaskMap.end() && waitTaskIt->second.size() > 0) {
                                pTask = *waitTaskIt->second.begin();
                                pTask->itInWaitTaskList.Erase();
                                if (0 == waitTaskIt->second.size()) {
                                        m_taskidToWaitTaskMap.erase(nTaskId);
                                }


                        } else {
                                break;
                        }
                }

                m_lock.Unlock();
        }
}

void FiThreads::DeleteTask(FiThreadTask *pTask) {
        if (!pTask->itInTaskList.IsNull()) {
                assert(pTask->itInTaskList.It() != m_taskList.end());
                pTask->itInTaskList.Erase();
        }

        if (!pTask->itInWaitTaskList.IsNull()) {
                assert(m_taskidToWaitTaskMap.find(pTask->nTaskId) != m_taskidToWaitTaskMap.end() && pTask->itInWaitTaskList.It() != m_taskidToWaitTaskMap[pTask->nTaskId].end());
                pTask->itInWaitTaskList.Erase();
                if (0 == m_taskidToWaitTaskMap[pTask->nTaskId].size()) {
                        m_taskidToWaitTaskMap.erase(pTask->nTaskId);
                }
        }

        if (!pTask->itInTaskIdMapList.IsNull()) {
                assert(m_taskidToTaskMap.find(pTask->nTaskId) != m_taskidToTaskMap.end() && pTask->itInTaskIdMapList.It() != m_taskidToTaskMap[pTask->nTaskId].end());
                pTask->itInTaskIdMapList.Erase();
                if (0 == m_taskidToTaskMap[pTask->nTaskId].size()) {
                        m_taskidToTaskMap.erase(pTask->nTaskId);
                }
        }

        if (pTask->bMerged) {
                m_taskidToMegeredTaskMap.erase(pTask->nTaskId);
        }


        if (NULL != pTask->pEvent) {
                pTask->pEvent->Set();
        }
        delete pTask;

        m_nTaskNum--;
}

void FiThreads::ClearTask(u64 nTaskId, bool bWait) {
        m_lock.Lock();

        FtBusyTaskWaitUnit busyWaitUnit;

        if (m_taskidToTaskMap.find(nTaskId) != m_taskidToTaskMap.end()) {
                list<FiThreadTask *> *pTaskIdMapList = &m_taskidToTaskMap[nTaskId];
                while (pTaskIdMapList->size() > 0) {
                        DeleteTask(*pTaskIdMapList->begin());
                }
        }

        if (!bWait) {
                m_lock.Unlock();
                return;
        }

        FiEvent evt;
        evt.Set();
        if (m_busyTaskIdToBusyNum.find(nTaskId) != m_busyTaskIdToBusyNum.end()) {
                busyWaitUnit.pEvent = &evt;
                busyWaitUnit.nTaskId = nTaskId;
                m_busyTaskIdToWaiters[nTaskId].push_back(&busyWaitUnit);

                evt.Reset();
        }

        m_lock.Unlock();

        evt.Wait();

}

void FiThreads::Run() {
        m_lock.Lock();
        assert(!m_bRun);

        m_pFiCxt->SetDefault(m_strConfigPath + "/MaxThreadNum", m_nThreadNum);
        m_pFiCxt->SetDefault(m_strConfigPath + "/MaxTaskNum", m_nDefaultMaxTaskNum);
        m_pFiCxt->SetDefault(m_strConfigPath + "/ThreadNumPerTask", m_nDefaultMaxThreadNumPerTask);
        m_pFiCxt->SetDefault(m_strConfigPath + "/MaxTaskEntryNum", m_nDefaultMaxTaskEntryNum);

        m_nThreadNum = m_pFiCxt->GetNum(m_strConfigPath + "/MaxThreadNum");

        m_bRun = true;
        for (int i = 0; i < m_nThreadNum; i++) {
                pthread_t      m_hThread;
                int nret = fi_create_thread(&m_hThread, ThreadFunc, this);
                assert(!nret);
                m_hThreadSet.insert(m_hThread);
        }

        m_lock.Unlock();

        StartReceiveTask();
}

void FiThreads::OnTaskProc(u64 nTaskId, void *pPara) {
        printf("FiThreads::OnTaskProc: nTaskId=%lld, pPara=%p\n", nTaskId, pPara);
}

void FiThreads::SetMaxTaskNum(u64 nMaxTaskNum, u64 nMaxTaskEntryNum) {
        m_nDefaultMaxTaskNum = nMaxTaskNum;
        m_nDefaultMaxTaskEntryNum = nMaxTaskEntryNum;
}

void FiThreads::SetMaxThreadNum(u32 nMaxThreadNum) {
        m_nThreadNum = nMaxThreadNum;

}

void FiThreads::SetMaxThreadNumPerTask(u32 nMaxThreadNumPerTask) {
        m_nDefaultMaxThreadNumPerTask = nMaxThreadNumPerTask;
}

void  FiThreads::SetMaxThreadNumForSpecificTask(u64 nTaskId, u32 nMaxThreadNumPerTask) {
        m_nSpecTaskIdToMaxThreadNumMap[nTaskId] = nMaxThreadNumPerTask;
}

void FiThreads::SetConfigPath(string strConfigPath) {
        m_strConfigPath = strConfigPath;
}

void FiThreads::SetDefaultCallBack(FiThreadPfn pfnCallBack) {
        m_pfnDefaultCallBack = pfnCallBack;
}


void* FiThreads::ThreadFunc(void *pPara) {
        FiThreads *pThread = (FiThreads *)pPara;
        pThread->TaskProc();
}

FiWorkerQueue* FiWorkerQueue::GetInstance() {
        if (NULL == s_pInstance) {
                s_lock.Lock();
                if (NULL == s_pInstance) {
                        s_pInstance = new FiWorkerQueue();
                }
                s_lock.Unlock();
        }

        return s_pInstance;
}

FiErr FiWorkerQueue::PushTask(u64 nTaskId, FiThreadPfn pfnCallBack, void *pContext, FiEvent *pEvent) {
        return m_threadPool.PushTask(nTaskId, pContext, pfnCallBack, pEvent);
}

FiErr FiWorkerQueue::PushMergedTask(u64 nTaskId, FiThreadPfn pfnCallBack, void *pContext, bool bDelContextAfterMerge) {
        return m_threadPool.PushMergedTask(nTaskId, pContext, pfnCallBack, bDelContextAfterMerge);
}

void FiWorkerQueue::ClearTask(u64 nTaskId, bool bWait) {
        return m_threadPool.ClearTask(nTaskId, bWait);
}

FiWorkerQueue::FiWorkerQueue() {
        m_threadPool.SetConfigPath(gFiLibConfigPath);
        m_threadPool.Run();
}

FiWorkerQueue::~FiWorkerQueue() {
}
