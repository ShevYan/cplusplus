#pragma once
#include "libcfg.h"  //????????????c++?????
#include "difstypes.h"
#include "OSUtility.h"
#include "OSThread.h"
#include "FiContext.h"
#include "md5.h"
#include "fitimer.h"
#include "FiDict.h"

using namespace std;

typedef pair<string, u64>               PfValue;

template<typename K, typename V>
V FiGetMapValueWithDef(const  std::map<K, V>& m, const K& key, const V  defval) {
        typename std::map<K, V>::const_iterator it = m.find(key);
        if (it == m.end()) {
                return defval;
        } else {
                return it->second;
        }
}



template<class Derived>
class FiSingleton {
public:
        static Derived* GetInstance() {
                static Derived instance;
                return &instance;
        }

protected:
        FiSingleton() { }

        ~FiSingleton() { }
};

template<class T> class FiObserver;

template<class T>
class FiSubject {
public:
        FiSubject() { }
        ~FiSubject() { }

        virtual void Attach(FiObserver<T> *pOb);
        virtual void Detach(FiObserver<T> *pOb);

        virtual void Notify();

private:
        set<FiObserver<T> *>    m_obSet;
};

template<class T>
void FiSubject<T>::Attach(FiObserver<T> *pOb) {
        m_obSet.insert(pOb);
}

template<class T>
void FiSubject<T>::Detach(FiObserver<T> *pOb) {
        m_obSet.erase(pOb);
}

template<class T>
void FiSubject<T>::Notify() {
        for (typename set<FiObserver<T> *>::iterator it = m_obSet.begin(); it != m_obSet.end(); it++) {
                (*it)->Update(static_cast<T *>(this));
        }
}


template<class T>
class FiObserver {
public:
        FiObserver() { }
        ~FiObserver() { }

        virtual void Update(T *pSubect) { }
};

#pragma pack(push,1)

#define PF_MAX_FILE_PATH_LEN 512
#define PF_MAX_PF_PATH 40
#define PF_CYCLE_FILE_VER  1
#define PF_CYCLE_FILE_MAGIC 0xcdab
#define PF_DATA_UNIT_HEADER 0xcdcd


struct CycleParam {
        u32     nSplitSize;
        u32     nMaxFileSize;
        u32     nHeadSize;

        bool Equal(CycleParam cyclePara2) {
                return (nSplitSize == cyclePara2.nSplitSize && nMaxFileSize == cyclePara2.nMaxFileSize && nHeadSize == cyclePara2.nHeadSize);
        }
};

struct CycleFileHeadCheckSum {
        u32             n_magic;
        u32             n_ver;
        u32             n_len;
        CycleParam      cyclePara;

        unsigned char   pDesgist[16];
};

struct PfDataRecordUnitHeader {
        u32     n_pfHeaderMagic;
        u64     n_time;
        u32     n_len;

        PfDataRecordUnitHeader() {
                n_pfHeaderMagic = PF_DATA_UNIT_HEADER;
        }
};

struct PfpathToIdEntry {
        char    strPfpath[PF_MAX_PF_PATH];
        u64     nPfid;
};


#pragma pack(pop)

struct PerfCalcTaskContext {
        u64 nCalcId;
        u64 nLastOutValue;
        u64 nFixValue;

        PerfCalcTaskContext() {
                nCalcId = 0;
                nLastOutValue = 0;
                nFixValue = 0;
        }

};

class PerfCalc {
public:
        PerfCalc();
        ~PerfCalc() { }

        const char*     GetCalcMethodStr(u64 nCalcMethod);
        vector<u64>     GetCalcMethod(string strCalcMethod);
        u64             Calc(u64 nOrgValue, u64 nNewValue, u64 nCalcMethod, u64 nTaskId);
        bool            FixCalcNextWhenEnd(u64& nOutId, u64& nOutValue);

public:
        static const u64                s_nNoValue;
        static const char               cDelimit = '#';

private:
        u64                             m_nSum, m_nAvg, m_nMax, m_nMin;
        map<u64, string>                m_calcIdToCalcStrMap;
        map<string, u64>                m_calcStrToCalcId;
        map<u64, PerfCalcTaskContext>   m_taskIdToTaskCxtMap;

};

class CycleFile {
public:
        CycleFile();
        ~CycleFile();

        void         ResetWriter();
        void         InitWriterConfig(string strConfigPath);
        CycleParam   GetCycleParam();
        void         SetWriterFileNamePrefix(string strFileNamePrefix);
        void         WriteHeadFromBegin(u8 *pBuf, u32 nLen);
        void         WriteBodySequence(u8 *pBuf, u32 nLen, bool bUnitEnd = false);

        bool         OpenCycleFile(string strFileName, CycleParam& cycleParam);
        bool         ReadHead(u8 *pBuf, u32 nLen, u32& nLenRet, bool bSeekBegin = false);
        bool         ReadBody(u8 *pBuf, u32 nLen, u32& nLenRet, bool bSeekBegin = false);

private:
        void         SwitchWriterFileNo(u32 srcNo, u32 uDstNo);

public:
        static const u64 HEAD_WITH_CHECK = 1;
        static const u64 BODY = 2;

private:
        string                                  m_strConfigPath;
        CycleParam                              m_cycleParaConf;

        CycleFileHeadCheckSum   m_cyclHeadCheck;;
        MD5_CTX                                 m_md5Cxt;

        string                                  m_strFileName;
        string                                  m_strDir;

        fstream                                 m_hFile;
        u64                                             m_nFileSize;

        u32                                     m_nActiveSecNo;
        u32                                     m_nMaxSecNum;

        CFiContext *m_pFiCxt;
        CCritSec                                m_lock;

};





/**
 * pfid????
 *   64     49   48      41  40       25  24         1
 *  /----------/-----------/------------/------------/
 *  |          |  calcId   |   sidno    |            |
 *  |          |           |            |            |
 * @---------- \-----------\------------\------------\
 *  | classid  |           Sid          |  Did       |
 * @---------- /-----------/------------/------------/
 * @author like
 */
class PfIdDB {
public:
        PfIdDB();
        ~PfIdDB();

        void                    GetAllDidPathInClass(string classpath, vector<string>& arDidpaths);
        void                    GetAllSidPathInClass(string classpath, vector<string>& arSidpaths);
        bool                    GenPfid(u64 nDid, string strUserPath, vector<u64>& arPfid);
        bool                    GenPfid(string strDid, string strUserPath, vector<u64>& arPfid);
        u64                     GetId(const string& strUserPfpath);
        void                    GetBinBuffer(u8 *& pBuf, u32& nLen);

        void                    Reset(u8 *pBuf, u32 nLen);

        static u64 PF_CLASS(u64 nPfid) {
                return nPfid & 0xffff000000000000;
        }

        static u64 PF_CLASS_SID(u64 nPfid) {
                return nPfid & 0xffffffffff000000 | 0x0000000000ffffff;
        }

        static u64 PF_CLASS_DID(u64 nPfid) {
                return nPfid & 0xffffff0000ffffff | 0x000000ffff000000;
        }

        static u64 PF_CALC_METHOD(u64 nPfid) {
                return (nPfid & 0x0000ff0000000000) >> 40;
        }


        static u64 PF_MK_PFID(u64 nClassSid, u64 nMethod, u64 nPfidIndex) {
                return (nClassSid & 0xffff00ffff000000) | nMethod << 40 | nPfidIndex;
        }

        static u64 PF_MK_CLASS(u64 nClassIndex) {
                return nClassIndex << 48;
        }

        static u64 PF_MK_CLASS_SID(u64 nClassId, u64 nMethod, u64 nSidIndex) {
                return nClassId | nMethod << 40 | (nSidIndex) << 24 | 0x0000000000ffffff;
        }

        static u64 PF_MK_CLASS_DID(u64 nClassId, u64 nMethod, u64 nDid) {
                return nClassId | nMethod << 40 | nDid | 0x000000ffff000000;
        }

        static const string strDelimit;
        static const char   cDelimit = '#';
        static const string strwildcard;

private:
        bool       AddPfpath(string strPfPath, u64 nPfid);

private:
        map<string, u64>                        m_pfidMap;
        map<string, u64>                        m_classSidPathToId;
        map<string, u64>                        m_classDidPathToId;
        map<string, u64>                        m_classPathToId;
        map<string, vector<u64> >               m_userPathToCalcMap;


        map<string, vector<string> >            m_classpathToDidArr;
        map<string, vector<string> >            m_classpathToSidArr;

        vector<PfpathToIdEntry>                 m_pfpathToIdMem;

        CCritSec                        m_lock;
        CFiContext *m_pFiCxt;

        PerfCalc                        m_perfCalc;;

};


class PfDataRecordUnit {
public:
        PfDataRecordUnit();
        ~PfDataRecordUnit();

        void    Probe(u64 nPfid, u64 nValue);
        u64     GetPfvalue(u64 nId);
        u64     GetCreateTime();
        void    GetBinBuffer(u8 *& pBuf, u32& nLen);
        void    GetBinBufferForHead(u8 *& pBuf, u32& nLen);
        bool    SetDeactive();
        bool    IsTimeOut();
        void    Reset();


private:
        void    FixProbeWhenTimeout();

private:
        map<u64, u64>   m_pfidSumIndexMap;
        map<u64, u64>   m_classDidSum;
        map<u64, u64>   m_classSidSum;
        map<u64, u64>   m_avgpfidToCountMap;

        u64                     m_nCreateTime;

        bool                    m_bActive;
        CCritSec                m_lock;


        //???????
        static u64                              m_nPeriod;

        //????????
        PfDataRecordUnitHeader  m_header;
        vector<u64>                             m_arPfidMem;

        PerfCalc                                m_perfCalc;

};

/**
 * ?????<p>
 * ????????
 * 
 * 1. ???????
 * ???? = "class.sid.did"  ???????40????
 * class: ????(xml???)
 * sid:   ?????(xml???)
 * did:   ??id?????????xml?????
 * ???diskband.write.disk1????????????????????disk1<p>
 * ?????
 * FPerf?????????????u64?id???pfid??????????????
 * pfid????
 *   64     49   48      41  40       25  24         1
 *  /----------/-----------/------------/------------/
 *  |          |  calcId   |   sidno    |            |
 *  |          |           |            |            |
 * 
 * @---------- \-----------\------------\------------\
 *  | classid  |           Sid          |  Did       |
 * @---------- /-----------/------------/------------/
 * ??calcId????????????sum, avg
 * sum:???? iops, ???????????
 * avg:????? ????????????
 * 
 * 2. ??:
 * 1. ???????????????????????????????
 * 2. ???????????web?????????????????
 * @author like
 */
class Perf : public FiTimer, public FiSubject<Perf> {
public:
        /**
         * ????
         * 
         * @return 
         */
        static Perf*    GetInstance();

        /**
         * ??????
         * 
         * @param nDid   ??id???ip??????????????????default??????????probe
         * @param v1     ????:???pair <string,u64> ?string?"class.sid"??????"FiBand.disk"?u64??????value
         */
        void                    Probe(u64 nDid, PfValue v1);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7);
        void                    Probe(u64 nDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7, PfValue v8);

        void                    Probe(string& strDid, PfValue v1);
        void                    Probe(string& strDid, PfValue v1, PfValue v2);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7);
        void                    Probe(string& strDid, PfValue v1, PfValue v2, PfValue v3, PfValue v4, PfValue v5, PfValue v6, PfValue v7, PfValue v8);


        /**
         * ????????????????value
         * 
         * @param strPfPath Perf?????:"FiBand.read.disk1"
         * 
         * @return ?????value?,????0?????????????
         *         ?????0
         */
        u64                     GetPerfValueLatest(string strPfPath, time_t *nTime = NULL);


        /**
         * ?????????????????Did????????FiIops??????ip
         * 
         * @param classpath  ??????,??"FiIops"
         * @param arDidpaths ???Did????
         */
        void                    GetAllDidpathInClass(string classpath, vector<string>& arDidpaths);


        /**
         * ???????????????id
         * 
         * @param classpath  ???class??
         * @param arSidpaths ???sid????
         */
        void                    GetAllSidpathInClass(string classpath, vector<string>& arSidpaths);

        /**
         * ???????????????????????xml??circlefile????
         * 
         * @param strFilename
         *               ???
         */
        void                    SetFileNamePrefix(string strFilename);

        ~Perf() {
                KillTimer(m_nPeriod);
        }

private:
        Perf();

        void                    PerfTimerThread();
        void                    MarkReset();
        void                    ResetIf();

        void                    OnTimer(int id);


private:
        //config
        static Perf *volatile  s_pInstance;
        static CCritSec         s_lock;
        bool                            m_bMon;
        bool                            m_bReset;
        u64                             m_nPeriod;
        u64                             m_nTime;
        u64                             m_nMaxMemSize;


        CFiContext *m_pFiCxt;

        //other
        u32                             m_nCheckPerfTimer;
        list<PfDataRecordUnit *>         m_pPfDataRecList;
        PfDataRecordUnit *m_pNowRecUnit;
        CycleFile                       m_cycleFile;
        PfIdDB                          m_pfIdDB;

};

class PerfFileParser {
public:
        PerfFileParser();
        ~PerfFileParser();

        /**
         * ??perf????
         * 
         * @param strSrcFile perf?????
         */
        void ParseFile(string strSrcFile);

        /**
         * ??perf??head???
         * 
         * @param vPerfHeader
         *               ?????????perfid?
         */
        virtual void OnParseHead(vector<string>& vPerfHeader) { }
        /**
         * 
         * @param nTime  ??perf?????
         * @param vValue ???????????????OnParseHead??????
         */
        virtual void OnParseTimeUnit(time_t nTime, vector<u64>& vValue) { }
        /**
         * ????
         */
        virtual void OnParseEnd() { }

private:
        void FormatDataUnit(u8 *pDataBuf, u32 nDataLen, u32 nOffParsed);

private:
        CycleFile               m_cycleFile;
        u32                     m_nPfpathNum;
        map<u64, u64>   m_idToValueMap;
        PerfCalc                m_perfCalc;
};


class PerfParsePrintFile : public PerfFileParser {
public:
        PerfParsePrintFile(string strDstFile);
        ~PerfParsePrintFile();

        void OnParseHead(vector<string>& vPerfHeader);
        void OnParseTimeUnit(time_t nTime, vector<u64>& vValue);
        void OnParseEnd();

private:
        void OpenDstFile(string strFileName);
        void OutputUnitToFile(string pStr);
        void OutputReturnToFile();
        void OutputFile(const char *pBuf, u32 nLen);
        void CloseDstFile();

private:
        fstream                         m_hDstFile;
        static const u32    m_nCacheFileBufLen = FSIZE_1M;
        char                            m_pCacheFileBuf[m_nCacheFileBufLen];
        u32                             m_nCacheFileBufValidLen;
};


/**
 * ????????????????????????
 */
class FPerfTimeCalc {
public:
        /**
         * 
         * @param bStartAuto ????????
         * @param nTimeResultFormat
         *                   s_second? GetTimeResult??????
         *                   s_mill? GetTimeResult???????
         *                   s_micro: GetTimeResult???????
         */
        FPerfTimeCalc(bool bStartAuto = true, int nTimeResultFormat = s_micro) {
                SetTimeResultFormat(nTimeResultFormat);
                if (bStartAuto) {
                        Start();
                } else {
                        m_nStartMicro = s_nInvalidTime;
                        m_nStopMicro = s_nInvalidTime;

                        bRun = false;
                }
        }

        ~FPerfTimeCalc() { }

        /**
         * ??????????????????????????
         * 
         * @return 
         */
        void Start() {
                m_nStartMicro = GetMicroSecTime();
                m_nStopMicro = s_nInvalidTime;

                bRun = true;
        }

        /**
         * ?????????????????????stop???????????????
         * 
         * @return ??start?stop???????
         */
        u64 Stop() {
                if (bRun) {
                        bRun = false;
                        m_nStopMicro = GetMicroSecTime();
                }

                return GetTimeResult();
        }

        /**
         * ??????
         * 
         * @return 
         */
        u64 Restart() {
                u64 nRet = Stop();
                Start();
                return nRet;
        }

        /**
         * ???????start?stop?????????????????start?stop?????0
         * 
         * @return ??????
         */
        u64 GetTimeResult() {
                u64 nRet = 0;
                if (s_nInvalidTime == m_nStartMicro || s_nInvalidTime == m_nStopMicro) {
                        nRet = 0;

                } else if (m_nStartMicro > m_nStopMicro) {
                        nRet = 0xffffffffffffffff - m_nStartMicro;
                        nRet += m_nStopMicro;
                } else {
                        nRet = m_nStopMicro - m_nStartMicro;
                }

                if (s_micro == m_nTimeFormat) {
                        nRet = nRet;
                } else if (s_mill == m_nTimeFormat) {
                        nRet /= 1000;
                } else if (s_second == m_nTimeFormat) {
                        nRet /= 1000000;
                }


                return nRet;
        }

        /**
         * ????????
         * 
         * @param nTimeResultFormat
         *               s_second? GetTimeResult??????
         *               s_mill? GetTimeResult???????
         *               s_micro: GetTimeResult???????
         * 
         * @return 
         */
        void SetTimeResultFormat(const int nTimeResultFormat) {
                assert(s_micro == nTimeResultFormat || s_mill == nTimeResultFormat || s_second == nTimeResultFormat);
                m_nTimeFormat = nTimeResultFormat;
        }

        static const int s_second = 1;
        static const int s_mill = 2;
        static const int s_micro = 3;

private:
        u64 m_nStartMicro;
        u64 m_nStopMicro;
        bool bRun;
        int m_nTimeFormat;

        static const u64 s_nInvalidTime = 0;
};
