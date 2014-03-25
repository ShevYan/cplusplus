#include  "perf.h"

Perf *volatile Perf::s_pInstance = NULL;
CCritSec Perf::s_lock;
u64 PfDataRecordUnit::m_nPeriod = 0;
const string PfIdDB::strDelimit = "#";
const string PfIdDB::strwildcard = "*";
const u64  PerfCalc::s_nNoValue = -1LL;


vector<string>& FiSplitString(const std::string& s, char delim, std::vector<std::string>& elems) {
        std::stringstream ss(s);
        std::string item;
        elems.clear();
        while (std::getline(ss, item, delim)) {
                elems.push_back(item);
        }
        return elems;
}


std::vector<std::string> FiSplitString(const std::string& s, char delim) {
        std::vector<std::string> elems;
        return FiSplitString(s, delim, elems);
}

Perf* Perf::GetInstance() {
        if (NULL == s_pInstance) {
                Perf::s_lock.Lock();
                if (NULL == s_pInstance) {
                        s_pInstance = new Perf();
                }
                Perf::s_lock.Unlock();
        }
        return s_pInstance;
}

void Perf::SetFileNamePrefix(string strFileName) {
        m_cycleFile.SetWriterFileNamePrefix(strFileName);
}

Perf::Perf() {
        m_cycleFile.InitWriterConfig("/FPerf/Config/cycleFile");

        m_pFiCxt = CFiContext::GetInstance();
        m_bReset = true;
        m_pNowRecUnit = new PfDataRecordUnit();
        ResetIf();
}

void Perf::Probe(u64 nDid, PfValue v1) {
        vector<u64> arPfid;
        bool br = false;

        Perf::s_lock.Lock();
        if (!m_bMon) {
                Perf::s_lock.Unlock();
                return;
        }

        br =  m_pfIdDB.GenPfid(nDid, v1.first, arPfid);
        if (!br) {
                Perf::s_lock.Unlock();
                return;
        }

        for (vector<u64>::iterator it = arPfid.begin(); it < arPfid.end(); it++) {
                m_pNowRecUnit->Probe(*it, v1.second);
        }
        Perf::s_lock.Unlock();
}

void Perf::Probe(string& strDid, PfValue v1) {
        vector<u64> arPfid;
        bool br = false;

        Perf::s_lock.Lock();
        if (!m_bMon) {
                Perf::s_lock.Unlock();
                return;
        }

        br =  m_pfIdDB.GenPfid(strDid, v1.first, arPfid);
        if (!br) {
                Perf::s_lock.Unlock();
                return;
        }

        for (vector<u64>::iterator it = arPfid.begin(); it < arPfid.end(); it++) {
                m_pNowRecUnit->Probe(*it, v1.second);
        }
        Perf::s_lock.Unlock();
}

void Perf::Probe(u64 nDid, PfValue v1, PfValue v2) {
        Probe(nDid, v1);
        Probe(nDid, v2);
}


void Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
}

void  Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
}

void  Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
}

void  Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
}

void  Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
        Probe(nDid, v7);
}

void  Perf::Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7, PfValue v8) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
        Probe(nDid, v7);
        Probe(nDid, v8);
}



void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
}

void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
}

void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
}

void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
}

void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
        Probe(nDid, v7);
}

void  Perf::Probe(string& nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7, PfValue v8) {
        Probe(nDid, v1);
        Probe(nDid, v2);
        Probe(nDid, v3);
        Probe(nDid, v4);
        Probe(nDid, v5);
        Probe(nDid, v6);
        Probe(nDid, v7);
        Probe(nDid, v8);
}


u64 Perf::GetPerfValueLatest(string strPfPath, time_t *pTime) {
        Perf::s_lock.Lock();
        if (m_pPfDataRecList.size() == 0) {
                *pTime = 0;
                Perf::s_lock.Unlock();
                return 0;
        }

        PfDataRecordUnit *pPfValueLatest = m_pPfDataRecList.back();

        u64 nId = m_pfIdDB.GetId(strPfPath);
        u64 nPfvalue = pPfValueLatest->GetPfvalue(nId);
        if (NULL != pTime) {
                *pTime = pPfValueLatest->GetCreateTime();
        }

        Perf::s_lock.Unlock();

        return nPfvalue;
}

void Perf::GetAllDidpathInClass(string classpath, vector<string>& arDidpaths) {
        m_pfIdDB.GetAllDidPathInClass(classpath, arDidpaths);
}

void Perf::GetAllSidpathInClass(string classpath, vector<string>& arDidpaths) {
        m_pfIdDB.GetAllSidPathInClass(classpath, arDidpaths);
}

void Perf::OnTimer(int id) {
        if (id == m_nCheckPerfTimer) {
                PerfTimerThread();
        }
}

void Perf::PerfTimerThread() {
        u8 *pBuf = NULL,*pBuf2 = NULL;
        u32 nLen = 0, nLen2 = 0;
        PfDataRecordUnit *pRecUnitItem = NULL,*pOldRecUnit = NULL;
        bool bNewRecUnitCreate = false;

        //??m_pNowRecUnit??????????
        Perf::s_lock.Lock();
        if (m_pNowRecUnit->IsTimeOut()) {
                pOldRecUnit = m_pNowRecUnit;
                m_pNowRecUnit = new PfDataRecordUnit();
                pOldRecUnit->SetDeactive();
                m_pPfDataRecList.push_back(pOldRecUnit);

                bNewRecUnitCreate = true;
        }
        Perf::s_lock.Unlock();

        if (bNewRecUnitCreate) {
                Notify();
        }
        //?pdidDB???????
        m_pfIdDB.GetBinBuffer(pBuf, nLen);
        m_cycleFile.WriteHeadFromBegin(pBuf, nLen);

        //????????????
        while (true) {
                Perf::s_lock.Lock();
                if (m_pPfDataRecList.size() <= 1) {
                        Perf::s_lock.Unlock();
                        break;
                }

                pRecUnitItem = m_pPfDataRecList.front();
                Perf::s_lock.Unlock();

                pRecUnitItem->GetBinBuffer(pBuf, nLen);
                pRecUnitItem->GetBinBufferForHead(pBuf2, nLen2);

                if (0 != nLen) {
                        m_cycleFile.WriteBodySequence(pBuf2, nLen2);
                        m_cycleFile.WriteBodySequence(pBuf, nLen, true);
                }

                Perf::s_lock.Lock();
                m_pPfDataRecList.pop_front();
                delete pRecUnitItem;
                Perf::s_lock.Unlock();
        }

        //????reset????reset
        ResetIf();

}

void Perf::ResetIf() {
        u8 *pBuf = NULL;
        u32 nLen = 0, nLenRet = 0;

        if (!m_bReset) {
                return;
        }

        m_cycleFile.ResetWriter();

        nLen = (m_cycleFile.GetCycleParam()).nHeadSize;
        pBuf = (u8 *)new char[nLen];
        m_cycleFile.ReadHead(pBuf, nLen, nLenRet, true);

        Perf::s_lock.Lock();

        m_pfIdDB.Reset(pBuf, nLenRet);
        m_pNowRecUnit->Reset();
        delete[] pBuf;

        while (m_pPfDataRecList.size() > 0) {
                PfDataRecordUnit *pRecUnitItem = m_pPfDataRecList.front();
                m_pPfDataRecList.pop_front();
                delete pRecUnitItem;
        }

        m_bReset = false;

        m_bMon = (bool)m_pFiCxt->GetNum("/FPerf/Config/bmon");

        m_nPeriod = m_pFiCxt->GetNum("/FPerf/Config/period");
        m_nCheckPerfTimer = SetTimer(m_nPeriod);
        Perf::s_lock.Unlock();
}

void Perf::MarkReset() {
        Perf::s_lock.Lock();
        m_bReset = true;
        m_bMon = false;
        Perf::s_lock.Unlock();
}

PfDataRecordUnit::PfDataRecordUnit() {
        Reset();
}


PfDataRecordUnit::~PfDataRecordUnit() {

}


void PfDataRecordUnit::Reset() {
        m_lock.Lock();

        m_pfidSumIndexMap.clear();
        m_classDidSum.clear();
        m_classSidSum.clear();

        m_arPfidMem.resize(0);
        m_arPfidMem.reserve(512 * 1024);

        m_bActive = true;
        m_nCreateTime = time(0);
        m_header.n_time = time(0);
        m_header.n_len = 0;

        m_lock.Unlock();
}

void PfDataRecordUnit::Probe(u64 nPfid, u64 nValue) {
        m_lock.Lock();
        if (!m_bActive) {
                m_lock.Unlock();
                return;
        }


        u64 nClassId = PfIdDB::PF_CLASS(nPfid);
        u64 nClassSid = PfIdDB::PF_CLASS_SID(nPfid);
        u64 nClassDid = PfIdDB::PF_CLASS_DID(nPfid);
        u64 nCalcMethod = PfIdDB::PF_CALC_METHOD(nPfid);
        u64 uIndexmapSize = m_pfidSumIndexMap.size();
        u64 nValueTmp = 0;

        bool bAddNewPfidMemIndex = false;
        if (m_pfidSumIndexMap.find(nPfid) == m_pfidSumIndexMap.end()) {
                m_pfidSumIndexMap[nPfid] = uIndexmapSize;
                m_arPfidMem.resize(m_arPfidMem.size() + 2); //??????????key?????value
                bAddNewPfidMemIndex = true;
        }

        u64 index = m_pfidSumIndexMap[nPfid] * 2;
        m_arPfidMem[index] = nPfid;
        m_arPfidMem[index + 1] = m_perfCalc.Calc(bAddNewPfidMemIndex ? PerfCalc::s_nNoValue : m_arPfidMem[index + 1], nValue, nCalcMethod, nPfid);


        nValueTmp = m_perfCalc.Calc(FiGetMapValueWithDef(m_classDidSum, nClassDid, PerfCalc::s_nNoValue), nValue, nCalcMethod, nClassDid);
        m_classDidSum[nClassDid] = nValueTmp;
        nValueTmp = m_perfCalc.Calc(FiGetMapValueWithDef(m_classSidSum, nClassSid, PerfCalc::s_nNoValue), nValue, nCalcMethod, nClassSid);
        m_classSidSum[nClassSid] = nValueTmp;

        m_lock.Unlock();

}

void PfDataRecordUnit::FixProbeWhenTimeout() {
        u64 nOutId, nOutValue;
        m_lock.Lock();
        while (true) {
                if (!m_perfCalc.FixCalcNextWhenEnd(nOutId, nOutValue)) {
                        break;
                }

                if (m_pfidSumIndexMap.find(nOutId) != m_pfidSumIndexMap.end()) {
                        u64 nIndex = m_pfidSumIndexMap[nOutId] * 2;
                        m_arPfidMem[nIndex + 1] = nOutValue;
                        continue;
                }

                if (m_classDidSum.find(nOutId) != m_classDidSum.end()) {
                        m_classDidSum[nOutId] = nOutValue;
                        continue;
                }

                if (m_classSidSum.find(nOutId) != m_classSidSum.end()) {
                        m_classSidSum[nOutId] = nOutValue;
                        continue;
                }
        }
        m_lock.Unlock();
}



u64 PfDataRecordUnit::GetCreateTime() {
        return m_nCreateTime;
}

u64  PfDataRecordUnit::GetPfvalue(u64 nId) {
        u64 nValue = 0;

        m_lock.Lock();
        __try_c{
                if (m_pfidSumIndexMap.find(nId) != m_pfidSumIndexMap.end()) {
                        u64 nIndex = m_pfidSumIndexMap[nId];
                        assert(nIndex * 2 + 1 <= m_arPfidMem.size());
                        nValue = m_arPfidMem[nIndex * 2 + 1];
                        __leave;
                }

                if (m_classSidSum.find(nId) != m_classSidSum.end()) {
                        nValue = m_classSidSum[nId];
                }

                if (m_classDidSum.find(nId) != m_classDidSum.end()) {
                        nValue  = m_classDidSum[nId];
                        __leave;
                }

        } __finally{

        }
        m_lock.Unlock();

        return nValue;
}


void PfDataRecordUnit::GetBinBufferForHead(u8 *& pBuf, u32& nLen) {
        pBuf = (u8 *)&m_header;
        nLen = sizeof(PfDataRecordUnitHeader);
}

void PfDataRecordUnit::GetBinBuffer(u8 *& pBuf, u32& nLen) {
        pBuf = (u8 *)&m_arPfidMem[0];
        nLen = m_arPfidMem.size() * sizeof(u64);
}

bool PfDataRecordUnit::SetDeactive() {
        m_lock.Lock();
        m_bActive = false;

        m_header.n_len =  m_arPfidMem.size() * sizeof(u64);

        FixProbeWhenTimeout();
        m_lock.Unlock();
}

bool PfDataRecordUnit::IsTimeOut() {
        bool bRet = false;

        m_lock.Lock();
        if (time(0) - m_nCreateTime >= PfDataRecordUnit::m_nPeriod) {
                bRet = true;
        }
        m_lock.Unlock();

        return bRet;
}


PerfCalc::PerfCalc() {
        m_nSum = 1;
        m_nAvg = 2;
        m_nMax = 3;
        m_nMin = 4;

        m_calcIdToCalcStrMap[m_nSum] = "sum";
        m_calcIdToCalcStrMap[m_nAvg] = "avg";
        m_calcIdToCalcStrMap[m_nMax] = "max";
        m_calcIdToCalcStrMap[m_nMin] = "min";

        m_calcStrToCalcId["sum"] = m_nSum;
        m_calcStrToCalcId["avg"] = m_nAvg;
        m_calcStrToCalcId["max"] = m_nMax;
        m_calcStrToCalcId["min"] = m_nMin;

}

bool PerfCalc::FixCalcNextWhenEnd(u64& nOutId, u64& nOutValue) {
        if (0 == m_taskIdToTaskCxtMap.size()) {
                return false;
        } else {
                map<u64, PerfCalcTaskContext>::iterator it = m_taskIdToTaskCxtMap.begin();
                nOutId = it->first;

                if (it->second.nCalcId == m_nAvg) {
                        nOutValue = it->second.nLastOutValue / it->second.nFixValue;
                } else {
                        nOutValue = it->second.nLastOutValue;
                }

                m_taskIdToTaskCxtMap.erase(it);
                return true;
        }
}


u64 PerfCalc::Calc(u64 nOrgValue, u64 nNewValue, u64 nCalcMethod, u64 nTaskId) {
        u64 nRet = 0;
        if (nCalcMethod == m_nSum) {
                nOrgValue = (nOrgValue == s_nNoValue) ? 0 : nOrgValue;
                nRet = nOrgValue + nNewValue;
        } else if (nCalcMethod ==  m_nMax) {
                nOrgValue = (nOrgValue == s_nNoValue) ? nNewValue : nOrgValue;
                nRet = nOrgValue < nNewValue ? nNewValue : nOrgValue;
        } else if (nCalcMethod ==   m_nMin) {
                nOrgValue = (nOrgValue == s_nNoValue) ? nNewValue : nOrgValue;
                nRet = nOrgValue > nNewValue ? nNewValue : nOrgValue;
        } else if (nCalcMethod == m_nAvg) {
                nOrgValue = (nOrgValue == s_nNoValue) ? 0 : nOrgValue;
                nRet = nOrgValue + nNewValue;

                m_taskIdToTaskCxtMap[nTaskId].nCalcId = nCalcMethod;
                m_taskIdToTaskCxtMap[nTaskId].nLastOutValue = nRet;
                m_taskIdToTaskCxtMap[nTaskId].nFixValue++;
        }

        return nRet;
}

const char* PerfCalc::GetCalcMethodStr(u64 nCalcMethod) {
        if (m_calcIdToCalcStrMap.find(nCalcMethod) == m_calcIdToCalcStrMap.end()) {
                return NULL;
        }
        return m_calcIdToCalcStrMap[nCalcMethod].c_str();
}

vector<u64> PerfCalc::GetCalcMethod(string strCalcMethod) {
        vector<u64> vCalcMethod;
        vector<string> vStrTmp = FiSplitString(strCalcMethod, cDelimit);
        for (vector<string>::iterator it = vStrTmp.begin(); it != vStrTmp.end(); it++) {
                if (m_calcStrToCalcId.find(*it) != m_calcStrToCalcId.end()) {
                        vCalcMethod.push_back(m_calcStrToCalcId[*it]);
                } else {
                        vCalcMethod.clear();
                        break;
                }
        }
        return vCalcMethod;
}

void PfIdDB::Reset(u8 *pBuf, u32 nLen) {
        m_lock.Lock();
        m_pfidMap.clear();
        m_classSidPathToId.clear();
        m_classPathToId.clear();
        m_userPathToCalcMap.clear();

        m_classpathToDidArr.clear();
        m_classpathToSidArr.clear();

        m_pfpathToIdMem.reserve(m_pFiCxt->GetNum("/FPerf/Config/maxPerfEntry") * sizeof(PfpathToIdEntry));
        m_pfpathToIdMem.resize(0);

        vector<string> arAllClass = m_pFiCxt->GetTagArr("/FPerf/PerfClass");
        vector<string> arAllPerfItem = m_pFiCxt->GetStringArr("/FPerf/Config/perfItem");

        vector<string> arTmp;
        for (vector<string>::iterator userPerfPathIt = arAllPerfItem.begin(); userPerfPathIt < arAllPerfItem.end(); userPerfPathIt++) {
                FiSplitString(*userPerfPathIt, cDelimit, arTmp);
                for (vector<string>::iterator classIt = arAllClass.begin(); classIt < arAllClass.end(); classIt++) {
                        if (0 == arTmp[0].compare(*classIt)) {
                                string strPath = "/FPerf/PerfClass/" + *classIt + "/op";
                                string strOp = m_pFiCxt->GetString(strPath);
                                vector<u64> vCalcMethod = m_perfCalc.GetCalcMethod(strOp);

                                for (vector<u64>::iterator calcIt = vCalcMethod.begin(); calcIt != vCalcMethod.end(); calcIt++) {
                                        m_userPathToCalcMap[*userPerfPathIt].push_back(*calcIt);

                                        string strClassSidPath = *userPerfPathIt + strDelimit + strwildcard + strDelimit + m_perfCalc.GetCalcMethodStr(*calcIt);
                                        m_classSidPathToId[strClassSidPath] = PF_MK_CLASS_SID(m_classPathToId[*classIt],  *calcIt,  (u64)m_classSidPathToId.size());
                                        m_classpathToSidArr[*classIt].push_back(strClassSidPath);
                                }
                        }
                }
        }


        if (0 != nLen) {
                assert(nLen % sizeof(PfpathToIdEntry) == 0);
                PfpathToIdEntry *pEntry = NULL;
                for (int i = 0; i < nLen / sizeof(PfpathToIdEntry); i++) {
                        pEntry = (PfpathToIdEntry *)pBuf + i;
                        AddPfpath(pEntry->strPfpath, pEntry->nPfid);
                }
        }
        m_lock.Unlock();
}

PfIdDB::PfIdDB() {
        m_pFiCxt = CFiContext::GetInstance();
        Reset(NULL, 0);
}

PfIdDB::~PfIdDB() {

}

u64 PfIdDB::GetId(const string& strUserPfpath) {
        u64 nId = 0;
        m_lock.Lock();
        string strPfpath = strUserPfpath;

        __try_c{
                vector<string> vStrTmp = FiSplitString(strPfpath, cDelimit);
                if (!(3 == vStrTmp.size() || 4 == vStrTmp.size())) {
                        __leave;
                }

                if (3 == vStrTmp.size()) {
                        //???????????????????
                        string strUserClassSid = vStrTmp[0] + strDelimit + vStrTmp[1];
                        if (m_userPathToCalcMap[strUserClassSid].size() != 1) {
                                __leave;
                        }
                        strPfpath = strUserPfpath + strDelimit + m_perfCalc.GetCalcMethodStr(m_userPathToCalcMap[strUserClassSid][0]);
                }


                map<string, u64>::iterator it = m_pfidMap.find(strPfpath);
                if (m_pfidMap.find(strPfpath) != m_pfidMap.end()) {
                        assert(m_pfpathToIdMem.size() >= m_pfidMap[strPfpath]);
                        nId = m_pfpathToIdMem[m_pfidMap[strPfpath]].nPfid;
                        __leave;
                }

                if (m_classSidPathToId.find(strPfpath) != m_classSidPathToId.end()) {
                        nId = m_classSidPathToId[strPfpath];
                        __leave;
                }

                if (m_classDidPathToId.find(strPfpath) != m_classDidPathToId.end()) {
                        nId = m_classDidPathToId[strPfpath];
                        __leave;
                }

        }
        __finally{
        }

        m_lock.Unlock();
        return nId;

}

bool PfIdDB::GenPfid(u64 nDid, string strUserPath, vector<u64>& arPfid) {
        char    strDid[32] = { 0 };

        sprintf(strDid, "%llx", nDid);

        return GenPfid(string(strDid), strUserPath, arPfid);

}


bool PfIdDB::GenPfid(string strDid, string strUserPath, vector<u64>& arPfid) {
        string              strPfPath;
        u64                 nClassSid = 0;
        PfpathToIdEntry     *pEntry = NULL;
        u32                 nOldSize = 0;
        bool                bRet = false;
        vector<u64>         vCalc;

        __try_c{
                m_lock.Lock();
                if (m_userPathToCalcMap.find(strUserPath) == m_userPathToCalcMap.end()) {
                        //???????
                        bRet = false;
                        __leave;
                }

                vCalc = m_userPathToCalcMap[strUserPath];

                for (vector<u64>::iterator calcIt = vCalc.begin(); calcIt != vCalc.end(); calcIt++) {
                        nClassSid = m_classSidPathToId[strUserPath + strDelimit + strwildcard + strDelimit + m_perfCalc.GetCalcMethodStr(*calcIt)];
                        strPfPath = strUserPath + strDelimit + strDid + strDelimit + m_perfCalc.GetCalcMethodStr(*calcIt);
                        if (m_pfidMap.find(strPfPath) == m_pfidMap.end()) {
                                bRet = AddPfpath(strPfPath, PF_MK_PFID(nClassSid, (u64) * calcIt, (u64)m_pfidMap.size()));
                                if (!bRet) __leave;
                        }

                        arPfid.push_back(m_pfpathToIdMem[m_pfidMap[strPfPath]].nPfid);
                }

                bRet = true;
        }
        __finally{
                m_lock.Unlock();
        }

        return true;
}

inline void PfIdDB::GetBinBuffer(u8 *& pBuf, u32& nLen) {
        pBuf = (u8 *)&m_pfpathToIdMem[0];
        nLen = m_pfpathToIdMem.size() * sizeof(PfpathToIdEntry);
}

inline bool PfIdDB::AddPfpath(string strPfPath, u64 nPfid) {
        ASSERT(m_pfpathToIdMem.size() <= m_pfpathToIdMem.capacity());
        if (m_pfpathToIdMem.size() == m_pfpathToIdMem.capacity() || strPfPath.length() >= PF_MAX_PF_PATH) {
                return false;
        }
        u64 nSize = m_pfpathToIdMem.size();
        m_pfidMap[strPfPath] = nSize;
        m_pfpathToIdMem.resize(nSize + 1);
        m_pfpathToIdMem[nSize].nPfid = nPfid;
        strcpy(&m_pfpathToIdMem[nSize].strPfpath[0], strPfPath.c_str());

        vector<string> arStrTmp;
        FiSplitString(strPfPath, cDelimit, arStrTmp);
        assert(arStrTmp.size() == 4);
        string strDidPath = arStrTmp[0] + strDelimit + strwildcard + strDelimit + arStrTmp[2] + strDelimit + arStrTmp[3];

        m_classDidPathToId[strDidPath] = PF_CLASS_DID(nPfid);
        m_classpathToDidArr[arStrTmp[0]].push_back(arStrTmp[2]);

        return true;
}

void PfIdDB::GetAllDidPathInClass(string classpath, vector<string>& arDidpaths) {
        m_lock.Lock();
        if (m_classpathToDidArr.find(classpath) == m_classpathToDidArr.end()) {
                m_lock.Unlock();
                return;
        }

        arDidpaths = m_classpathToDidArr[classpath];
        m_lock.Unlock();
}

void PfIdDB::GetAllSidPathInClass(string classpath, vector<string>& arSidpaths) {
        m_lock.Lock();
        if (m_classpathToSidArr.find(classpath) == m_classpathToSidArr.end()) {
                return;
        }

        arSidpaths = m_classpathToSidArr[classpath];
        m_lock.Unlock();
}


CycleFile::CycleFile() {

}

CycleFile::~CycleFile() {

}

void CycleFile::SetWriterFileNamePrefix(string strFileNamePrefix) {
        m_strFileName = strFileNamePrefix;
}

void CycleFile::InitWriterConfig(string strConfigPath) {
        m_pFiCxt = CFiContext::GetInstance();
        m_strDir = GetExeDir() + "PerfLog";
        mkdir(m_strDir.c_str(), 0777);

        m_strConfigPath = strConfigPath;
        m_strFileName = m_pFiCxt->GetString("/FPerf/Config/cycleFile/filename");
}

CycleParam CycleFile::GetCycleParam() {
        return m_cyclHeadCheck.cyclePara;
}

void CycleFile::ResetWriter() {
        m_lock.Lock();
        m_cycleParaConf.nMaxFileSize = m_pFiCxt->GetNum(m_strConfigPath + "/filesize") * FSIZE_1M;
        m_cycleParaConf.nSplitSize = m_pFiCxt->GetNum(m_strConfigPath + "/splitsize") * FSIZE_1M;
        m_cycleParaConf.nHeadSize = m_pFiCxt->GetNum(m_strConfigPath + "/headsize") * FSIZE_1M;

        assert(m_cycleParaConf.nHeadSize < m_cycleParaConf.nSplitSize);
        assert(m_cycleParaConf.nSplitSize < m_cycleParaConf.nMaxFileSize);

        m_cycleParaConf.nMaxFileSize =  (0 != m_cycleParaConf.nMaxFileSize % m_cycleParaConf.nSplitSize)
           ? (m_cycleParaConf.nMaxFileSize / m_cycleParaConf.nSplitSize + 1) * m_cycleParaConf.nSplitSize
           : m_cycleParaConf.nMaxFileSize;
        m_nMaxSecNum = m_cycleParaConf.nMaxFileSize / m_cycleParaConf.nSplitSize;

        if (m_hFile.is_open()) {
                m_hFile.close();
        }

        char pFilename[PF_MAX_FILE_PATH_LEN] = { 0 };

        m_nActiveSecNo = -1;
        for (int i = 0; i < m_nMaxSecNum; i++) {
                sprintf(pFilename, "%s/%s_%d_ing", m_strDir.c_str(), m_strFileName.c_str(), i);

                ifstream infile((const char *)pFilename);
                if (infile.is_open()) {
                        m_nActiveSecNo = i;
                        infile.close();
                        break;
                }
        }


        bool bNeedCreateCycleFile = true;
        CycleParam cycleParaConfNow;
        if (-1 != m_nActiveSecNo && OpenCycleFile(pFilename, cycleParaConfNow) && cycleParaConfNow.Equal(m_cycleParaConf)) {
                bNeedCreateCycleFile = false;
        }


        if (-1 == m_nActiveSecNo) {
                m_nActiveSecNo = 0;
        }

        if (bNeedCreateCycleFile) {
                SwitchWriterFileNo(-1, m_nActiveSecNo);
        }

        m_lock.Unlock();
}

bool CycleFile::OpenCycleFile(string strFileName, CycleParam& cyclePara) {
        bool bOpen = false;
        __try_c{
                if (m_hFile.is_open()) {
                        m_hFile.close();
                }

                m_hFile.open(strFileName.c_str());
                if (m_hFile.fail()) {
                        __leave;
                }

                m_hFile.seekp(0, ios_base::end);
                m_nFileSize = m_hFile.tellp();
                if (m_nFileSize < sizeof(CycleFileHeadCheckSum)) {
                        __leave;
                }

                m_hFile.seekp(0);
                m_hFile.read((char *)&m_cyclHeadCheck, sizeof(CycleFileHeadCheckSum));
                assert(!m_hFile.fail());
                if (m_nFileSize <= m_cyclHeadCheck.cyclePara.nHeadSize || m_cyclHeadCheck.n_len > m_cyclHeadCheck.cyclePara.nHeadSize || m_cyclHeadCheck.n_magic != PF_CYCLE_FILE_MAGIC ||  m_cyclHeadCheck.n_ver != PF_CYCLE_FILE_VER) {
                        __leave;
                }

                char *pBuf = new char[m_cyclHeadCheck.n_len];
                unsigned char pDesgist[16];
                m_hFile.read(pBuf, m_cyclHeadCheck.n_len);
                assert(!m_hFile.fail());
                MD5_Init(&m_md5Cxt);
                MD5_Update(&m_md5Cxt, (unsigned char *)pBuf, m_cyclHeadCheck.n_len);
                MD5_Final(pDesgist, &m_md5Cxt);
                delete[] pBuf;
                if (0 != memcmp(pDesgist, m_cyclHeadCheck.pDesgist, 16)) {
                        __leave;
                }

                cyclePara = m_cyclHeadCheck.cyclePara;
                m_hFile.seekp(sizeof(CycleFileHeadCheckSum));
                bOpen = true;

        } __finally{
                if (!bOpen && m_hFile.is_open()) {
                        m_hFile.close();
                }
        }
        return bOpen;
}

void CycleFile::WriteHeadFromBegin(u8 *pBuf, u32 nLen) {
        if (0 == nLen) {
                return;
        }

        m_lock.Lock();

        if (nLen + sizeof(CycleFileHeadCheckSum) >= m_cycleParaConf.nHeadSize) {
                m_lock.Unlock();
                return;
        }

        m_hFile.seekp(0);

        m_cyclHeadCheck.n_len = nLen;
        MD5_Init(&m_md5Cxt);
        MD5_Update(&m_md5Cxt, pBuf, m_cyclHeadCheck.n_len);
        MD5_Final(m_cyclHeadCheck.pDesgist, &m_md5Cxt);
        m_hFile.write((char *)&m_cyclHeadCheck, sizeof(CycleFileHeadCheckSum));
        m_hFile.write((char *)pBuf, nLen);
        m_hFile.flush();

        m_lock.Unlock();
}


void CycleFile::WriteBodySequence(u8 *pBuf, u32 nLen, bool bUnitEnd) {
        char pFilenameIng[PF_MAX_FILE_PATH_LEN] = { 0 }, pFilenameOld[PF_MAX_FILE_PATH_LEN] = { 0 };

        if (0 == nLen) {
                return;
        }

        m_lock.Lock();
        m_hFile.seekp(m_nFileSize);
        m_hFile.write((char *)pBuf, nLen);
        m_hFile.flush();

        m_nFileSize += nLen;
        if (bUnitEnd && m_nFileSize + nLen > m_cyclHeadCheck.cyclePara.nMaxFileSize) {
                SwitchWriterFileNo(m_nActiveSecNo, m_nActiveSecNo == m_nMaxSecNum ? 0 : m_nActiveSecNo + 1);
        }


        m_lock.Unlock();
}

void CycleFile::SwitchWriterFileNo(u32 srcNo, u32 uDstNo) {
        char pFilenameIng[PF_MAX_FILE_PATH_LEN] = { 0 }, pFilenameOld[PF_MAX_FILE_PATH_LEN] = { 0 };

        if (m_hFile.is_open()) {
                m_hFile.close();
        }

        if (-1 != srcNo) {
                sprintf(pFilenameIng, "%s/%s_%d_ing", m_strDir.c_str(), m_strFileName.c_str(), srcNo);
                sprintf(pFilenameOld, "%s/%s_%d", m_strDir.c_str(), m_strFileName.c_str(), srcNo);

                rename(pFilenameIng, pFilenameOld);
        }

        assert(uDstNo != -1);

        sprintf(pFilenameIng, "%s/%s_%d_ing", m_strDir.c_str(), m_strFileName.c_str(), uDstNo);
        sprintf(pFilenameOld, "%s/%s_%d", m_strDir.c_str(), m_strFileName.c_str(), uDstNo);
        unlink(pFilenameOld);

        m_hFile.open(pFilenameIng, fstream::trunc | fstream::out);
        assert(m_hFile.is_open());

        m_nFileSize = m_cycleParaConf.nHeadSize;
        m_cyclHeadCheck.n_magic = PF_CYCLE_FILE_MAGIC;
        m_cyclHeadCheck.n_ver = PF_CYCLE_FILE_VER;
        m_cyclHeadCheck.n_len = 0;
        m_cyclHeadCheck.cyclePara = m_cycleParaConf;


}


bool  CycleFile::ReadHead(u8 *pBuf, u32 nLen, u32& nLenRet,  bool bSeekBegin) {
        u32 nOff = 0;
        if (bSeekBegin) {
                nOff = sizeof(m_cyclHeadCheck);
        } else {
                nOff = m_hFile.tellp();
        }

        if (nOff >=   m_cyclHeadCheck.n_len + sizeof(m_cyclHeadCheck) || nOff < sizeof(m_cyclHeadCheck)) {
                nLenRet = 0;
                return false;
        } else {
                m_hFile.seekp(nOff);
        }

        nLenRet = m_cyclHeadCheck.n_len + sizeof(m_cyclHeadCheck) - nOff;
        m_hFile.read((char *)pBuf, nLenRet);
        return true;
}

bool  CycleFile::ReadBody(u8 *pBuf, u32 nLen, u32& nLenRet, bool bSeekBegin) {
        u32 nOff = 0;
        if (bSeekBegin) {
                nOff = m_cyclHeadCheck.cyclePara.nHeadSize;
        } else {
                nOff =  m_hFile.tellp();
        }

        if (nOff >= m_nFileSize) {
                return false;
        } else {
                m_hFile.seekp(nOff);
        }

        nLenRet = nOff + nLen > m_nFileSize ? m_nFileSize - nOff : nLen;

        m_hFile.read((char *)pBuf, nLenRet);
        return true;
}


PerfFileParser::PerfFileParser() {
}

PerfFileParser::~PerfFileParser() {
}

void PerfFileParser::ParseFile(string strSrcFile) {
        CycleParam cyclePara;

        if (!m_cycleFile.OpenCycleFile(strSrcFile, cyclePara)) {
                return;
        }

        u32 nLen = cyclePara.nHeadSize, nLenRet = 0, nDataLen = FSIZE_1M * 5, nDataLenRet = 0, nOffset = 0;
        u8 *pBuf = (u8 *)new char[nLen], *pDataBuf = (u8 *)new char[nDataLen];
        map<u64, string> idToPfPathMap;

        m_cycleFile.ReadHead(pBuf, nLen, nLenRet, true);

        assert(nLenRet % sizeof(PfpathToIdEntry) == 0);
        PfpathToIdEntry *pEntry = NULL;
        m_nPfpathNum = nLenRet / sizeof(PfpathToIdEntry);
        for (int i = 0; i < m_nPfpathNum; i++) {
                pEntry = (PfpathToIdEntry *)pBuf + i;

                u64 nPfid = pEntry->nPfid;
                if (m_idToValueMap.find(nPfid) == m_idToValueMap.end()) {
                        m_idToValueMap[nPfid] = 0;
                        m_idToValueMap[PfIdDB::PF_CLASS_SID(nPfid)] = 0;
                        m_idToValueMap[PfIdDB::PF_CLASS_DID(nPfid)] = 0;

                        vector<string> vTmp = FiSplitString(pEntry->strPfpath, PfIdDB::cDelimit);
                        assert(vTmp.size() == 4);
                        idToPfPathMap[nPfid] = pEntry->strPfpath;
                        idToPfPathMap[PfIdDB::PF_CLASS_SID(nPfid)] = vTmp[0] + PfIdDB::strDelimit + vTmp[1] + PfIdDB::strDelimit + PfIdDB::strwildcard + PfIdDB::strDelimit + vTmp[3];
                        idToPfPathMap[PfIdDB::PF_CLASS_DID(nPfid)] = vTmp[0] + PfIdDB::strDelimit + PfIdDB::strwildcard + PfIdDB::strDelimit + vTmp[2] + PfIdDB::strDelimit + vTmp[3];;
                }

        }

        int i = 0;
        vector<string> vPerfHeader;
        for (map<u64, u64>::iterator it = m_idToValueMap.begin(); it != m_idToValueMap.end(); it++) {
                it->second = i++;
                vPerfHeader.push_back(idToPfPathMap[it->first]);
        }

        OnParseHead(vPerfHeader);

        u32 nOffParsed = 0;
        bool bSeekBeg = true;
        while (true) {
                if (!m_cycleFile.ReadBody(pDataBuf, nDataLen, nDataLenRet, bSeekBeg)) {
                        break;
                }

                bSeekBeg = false;
                FormatDataUnit(pDataBuf, nDataLenRet, nOffParsed);
                if (nDataLen > nDataLenRet) {
                        break;
                }

                if (nDataLenRet > nOffParsed) {
                        memcpy(pDataBuf, pDataBuf + nOffParsed, nDataLenRet - nOffParsed);
                }
                nOffParsed = 0;
        }

        OnParseEnd();

}

void PerfFileParser::FormatDataUnit(u8 *pDataBuf, u32 nDataLen, u32 nOffParsed) {
        PfDataRecordUnitHeader *pDataUnitHeader = NULL;
        u64 *pDataUnitBuf = NULL;

        u64 nClassSid = 0, nClassDid = 0, nCalcId = 0;

        while (true) {
                for (map<u64, u64>::iterator it = m_idToValueMap.begin(); it != m_idToValueMap.end(); it++) {
                        it->second = PerfCalc::s_nNoValue;
                }

                pDataUnitHeader = (PfDataRecordUnitHeader *)(pDataBuf + nOffParsed);
                pDataUnitBuf = (u64 *)(pDataBuf + nOffParsed + sizeof(PfDataRecordUnitHeader));

                if (nDataLen - nOffParsed < sizeof(PfDataRecordUnitHeader) || nDataLen - nOffParsed <  pDataUnitHeader->n_len) {
                        break;
                }


                u64 nPfId = 0;
                for (int i = 0; i < pDataUnitHeader->n_len / sizeof(u64); i += 2) {
                        nPfId = pDataUnitBuf[i];
                        nClassSid = PfIdDB::PF_CLASS_SID(nPfId);
                        nClassDid = PfIdDB::PF_CLASS_DID(nPfId);
                        nCalcId = PfIdDB::PF_CALC_METHOD(nPfId);

                        m_idToValueMap[nPfId] = pDataUnitBuf[i + 1];
                        m_idToValueMap[nClassSid] = m_perfCalc.Calc(m_idToValueMap[nClassSid], pDataUnitBuf[i + 1],  nCalcId, nClassSid);
                        m_idToValueMap[nClassDid] = m_perfCalc.Calc(m_idToValueMap[nClassDid], pDataUnitBuf[i + 1],  nCalcId, nClassDid);

                }


                while (true) {
                        u64 nOutId = 0, nOutValue = 0;
                        if (!m_perfCalc.FixCalcNextWhenEnd(nOutId, nOutValue)) {
                                break;
                        }

                        if (m_idToValueMap.find(nOutId) != m_idToValueMap.end()) {
                                m_idToValueMap[nOutId] = nOutValue;
                        }

                }

                char arBufTmp[32];
                vector<u64> vValue;
                for (map<u64, u64>::iterator it = m_idToValueMap.begin(); it != m_idToValueMap.end(); it++) {
                        if (it->second == PerfCalc::s_nNoValue) {
                                it->second = 0;
                        }
                        vValue.push_back(it->second);

                }

                OnParseTimeUnit(pDataUnitHeader->n_time, vValue);

                nOffParsed += (sizeof(PfDataRecordUnitHeader) + pDataUnitHeader->n_len);
        }
}


PerfParsePrintFile::PerfParsePrintFile(string strDstFile) {
        OpenDstFile(strDstFile);
}

PerfParsePrintFile::~PerfParsePrintFile() {

}

void PerfParsePrintFile::OnParseHead(vector<string>& vPerfHeader) {
        OutputUnitToFile("time");

        for (vector<string>::iterator it = vPerfHeader.begin(); it != vPerfHeader.end(); it++) {
                OutputUnitToFile(*it);
        }

        OutputReturnToFile();
}

void PerfParsePrintFile::OnParseTimeUnit(time_t nTime, vector<u64>& vValue) {
        struct tm *tm = localtime(&nTime);
        char szTmp[50] = { 0 };
        strftime(szTmp, 50, "%y:%m:%d %H:%M:%S\t", tm);
        OutputUnitToFile(szTmp);

        char arBufTmp[32];
        for (vector<u64>::iterator it = vValue.begin(); it != vValue.end(); it++) {
                sprintf(arBufTmp, "%lld\t", *it);
                OutputUnitToFile(arBufTmp);
        }

        OutputReturnToFile();
}

void PerfParsePrintFile::OnParseEnd() {
        CloseDstFile();
}

void PerfParsePrintFile::OpenDstFile(string strFileName) {
        if (m_hDstFile.is_open()) {
                m_hDstFile.close();
        }
        m_hDstFile.open(strFileName.c_str(), fstream::trunc | fstream::out);
        assert(m_hDstFile.is_open());
        m_nCacheFileBufValidLen = 0;
}

void PerfParsePrintFile::OutputUnitToFile(string pStr) {
        OutputFile((char *)pStr.c_str(), pStr.length());
        OutputFile("\t", strlen("\t"));
}

void PerfParsePrintFile::OutputReturnToFile() {
        OutputFile("\n", strlen("\n"));
}

void PerfParsePrintFile::OutputFile(const char *pBuf, u32 nLen) {
        if (m_nCacheFileBufValidLen + nLen > m_nCacheFileBufLen) {
                m_hDstFile.write(m_pCacheFileBuf, m_nCacheFileBufValidLen);
                m_nCacheFileBufValidLen = 0;
        }

        if (nLen > m_nCacheFileBufLen) {
                m_hDstFile.write(pBuf, nLen);
        } else {
                memcpy(m_pCacheFileBuf + m_nCacheFileBufValidLen, pBuf, nLen);
                m_nCacheFileBufValidLen += nLen;
        }

}

void PerfParsePrintFile::CloseDstFile() {
        if (m_nCacheFileBufValidLen > 0) {
                m_hDstFile.write(m_pCacheFileBuf, m_nCacheFileBufValidLen);
        }

        if (m_hDstFile.is_open()) {
                m_hDstFile.close();
        }
}
