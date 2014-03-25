#pragma once
#include "libcfg.h"
#include "difstypes.h"
#include "OSThread.h"
#include "FiContext.h"
#include "md5.h"
#include "FiLog.h"
#include "FiDict.h"

template<typename T>
class fi_make_vector {
public:
        typedef fi_make_vector<T> my_type;
        my_type& operator<<(const T& val) {
                data_.push_back(val);
                return *this;
        }
        operator std::vector<T>() const {
                return data_;
        }
private:
        std::vector<T> data_;
};

struct FiFileNameTableUnit {
        const char *strFileName;
        u16 nFileId;
};


class FiErrBaseMgr {

private:
        map<string, u16> m_filenameToIdMap;
        map<u16, string> m_fileIdToFileNameMap;
        map<u32, string> m_baseErrCodeToStr;
        map<u32, u32>    m_baseErrCodeToUserCode;
        static CCritSec s_lock;
        static FiErrBaseMgr *volatile s_pInstance;
        static const char *s_nullFileName;
        static const u16   s_nullFileId = -1;

public:
        static FiErrBaseMgr* GetInstance();

        void AddBaseCodeInfo(u32 nBaseCode, u32 nUserCode, const char *strBaseErr);

        void AddFileNameInfo(const char *strFileName, u16 nFileId);

        const char* GetFileName(u16 nFileId);

        u16 GetFileId(const char *strFileName);

        u32 GetUserErrCode(u32 nBaseErr);

        const char* GetBaseErrText(u32 nBaseErr);

        ~FiErrBaseMgr() { }

        static u32 MakeBaseCode_Erro(u8 nClasses, u16 nCode) {
                return (u32)(((unsigned long)(s_erro) << 24) | ((unsigned long)(nClasses) << 16) | ((unsigned long)(nCode)));
        }

        static u32 MakeBaseCode_Suc(u8 nClasses, u16 nCode) {
                return (u32)(((unsigned long)(s_succ) << 24) | ((unsigned long)(nClasses) << 16) | ((unsigned long)(nCode)));
        }

        static bool IsSuccess(u32 nBaseErr) {
                return (s_succ == (nBaseErr >> 24));
        }

        static bool IsErr(u32 nBaseErr) {
                return (s_erro == (nBaseErr >> 24));
        }

        static const u8 s_succ = 0;
        static const u8 s_erro = 1;

private:
        FiErrBaseMgr() { }
};

#define FSucCode FiErr::MakeErr(FiErrBaseMgr::s_succ, FiErr::GetFileId(__FILE__), __LINE__)

//???????
#define FSuc(nAppSuc) FiErr::MakeErr(nAppSuc, FiErr::GetFileId(__FILE__), __LINE__)

//???????
#define FErr(nAppErr) FiErr::MakeErr(nAppErr, FiErr::GetFileId(__FILE__), __LINE__)

//??????????????????
#define FErrEx(nAppErr, nSysErr) FiErr::MakeErr(nAppErr, FiErr::GetFileId(__FILE__), __LINE__, nSysErr)

/**
 * ?????
 * 1.  windows, linux ,mac????????????????????????????????????????
 * 2.  fics????????????????????????????????????????
 * 3.  ?????unsuccessful???????????????????
 * 4.  ????????????????????
 * 5.  ???????????????????????????
 * 6.  ????????????????????????????????????????
 *     ???????????????/error????baseerrCode?tag? ?????value
 * 7.  ?????????????????????????
 * 8.  ??????????????
 * 9.  fics????????????????????????
 * 10. ??basecode??????usercode,?????
 * 11. ?????????????????
 * ??????????
 * errcode = main code + extend code
 * 1.main code???
 *   64     57   56      49  48       33  32      17 16          0
 *  /----------/-----------/-----------/------------/------------/
 *  |  type    |  module   |  errcode  | filename   | lineNo
 *  |(err,suc) |           |           |            |
 * 
 * @---------- \-----------\-----------\------------\------------\
 *  |          BaseErrCode              |           |
 * @---------- /-----------/-----------/------------/------------/
 * 
 * 2. extend code
 *   64          33 32        0
 *  /-------------/-----------/
 *  |  reserve    |  syserro  |
 * @---------- ---\-----------\
 */
struct FiErrData;
class FiErr : public exception {
        friend struct FiErrData;
private:
        u64 m_nMainCode;
        u64 m_nExtCode;

        static string        s_strDictPathPrefix;
        static FiErr         s_suc;

public:

        bool Is(u32 nAppErr){
                return nAppErr == (u32)(m_nMainCode>>32);
        }

        /**
         * ?????????????????
         * 
         * @return ??????
         */
        u32 GetUserErrCode();

        /**
         * ????????????????
         * ?????BaseErrText:SysErrCode, FileName:lineNo
         * 
         * @return ???????
         */
        string StrFiText();

        /**
         * ???StrFiText?????????wchar_t*???
         * 
         * @return 
         */
        wstring WstrFiText();

        /**
         * ????????????(?????)
         * 
         * @return ?????????
         */
        string StrUserText();


        /**
         * 
         * @return ????
         */
        bool IsSuccess() {
                FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
                return pErrBaseMgr->IsSuccess(BaseErr());
        }

        /**
         * 
         * @return ????
         */
        bool IsErr() {
                FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
                return pErrBaseMgr->IsErr(BaseErr());
        }

        /**
         * 
         * @return ??????(???????????)
         */
        static u32  MakeBaseCode_Erro(u8 nClasses, u16 nCode, u32 nUserCode, const char *strBaseErr);

        /**
         * ???????(????????????) 
         * 
         * @return 
         */
        static u32  MakeBaseCode_Suc(u8 nClasses, u16 nCode, u32 nUserCode, const char *strBaseSuc);

        static FiErr MakeErr(u32 nAppErr, u16 nFileId, u16 nLine, u32 nSysErr = 0) {
                FiErr err;
                err.MakeErrMySelf(nAppErr, nFileId, nLine, nSysErr);
                return err;
        }

        static u16 GetFileId(const char *strFileName);

        FiErr() {
                MakeErrMySelf(FiErrBaseMgr::s_succ, -1, 0, 0);
        }

        FiErr(const FiErr& err) {
                m_nMainCode = err.GetMainCode();
                m_nExtCode = err.GetExtCode();
        }

        FiErr& operator = (const FiErr& err) {
                m_nMainCode = err.GetMainCode();
                m_nExtCode = err.GetExtCode();
                return *this;
        }

        ~FiErr() throw() { }

private:
        u32 BaseErr();
        u16 FileId();
        u16 FileNo();
        u32 SysErr();
        const char* FileName();

        void MakeErrMySelf(const u32& nAppErr, const u16& nFileId, const u16& nLine, const u32& nSysErr) {
                m_nMainCode = (u64)nAppErr << 32 | (u64)nFileId << 16 | (u64)nLine;
                m_nExtCode = (u64)nSysErr;
        }

        const u64& GetMainCode() const {
                return m_nMainCode;
        }

        const u64& GetExtCode() const {
                return m_nExtCode;
        }
};


/**
 * ????????????id,???????, 
 * ??????:???????????,???????add??
 */
class FiErrFileNameRegitor {
public:
        FiErrFileNameRegitor();

        FiErrFileNameRegitor(vector<FiFileNameTableUnit> vFileNameTable);

        ~FiErrFileNameRegitor() { }

        void AddFileTable(vector<FiFileNameTableUnit> vFileNameTable);

private:
        FiErrBaseMgr *m_pErrBaseMgr;
};

struct FiErrData {
        u64 m_nMainCode;
        u64 m_nExtCode;

        FiErrData() {
        }

        FiErrData(const FiErr& err) {
                m_nMainCode = err.m_nMainCode;
                m_nExtCode = err.m_nExtCode;
        }

        FiErrData& operator = (const FiErr& err) {
                m_nMainCode = err.m_nMainCode;
                m_nExtCode = err.m_nExtCode;
                return *this;
        }

        operator FiErr() const {
                FiErr err;
                err.m_nMainCode = m_nMainCode;
                err.m_nExtCode = m_nExtCode;
                return err;
        }
};
