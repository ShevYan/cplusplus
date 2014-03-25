#include "FiErr.h"

CCritSec FiErrBaseMgr::s_lock;
FiErrBaseMgr *volatile FiErrBaseMgr::s_pInstance = NULL;
const char *FiErrBaseMgr::s_nullFileName = "nullfile";

string        FiErr::s_strDictPathPrefix("/error/");
FiErr         FiErr::s_suc = FSucCode;


FiErrBaseMgr* FiErrBaseMgr::GetInstance() {
        if (NULL == FiErrBaseMgr::s_pInstance) {
                s_lock.Lock();
                if (NULL == FiErrBaseMgr::s_pInstance) {
                        s_pInstance = new FiErrBaseMgr();
                }
                s_lock.Unlock();
        }

        return s_pInstance;
}


void FiErrBaseMgr::AddBaseCodeInfo(u32 nBaseCode, u32 nUserCode, const char *strBaseErr) {
        m_baseErrCodeToStr[nBaseCode] = strBaseErr;
        m_baseErrCodeToUserCode[nBaseCode] = nUserCode;
}

void FiErrBaseMgr::AddFileNameInfo(const char *strFileName, u16 nFileId) {
        m_fileIdToFileNameMap[nFileId] = strFileName;
        m_filenameToIdMap[strFileName] = nFileId;
}


const char* FiErrBaseMgr::GetFileName(u16 nFileId) {
        if (m_fileIdToFileNameMap.find(nFileId) != m_fileIdToFileNameMap.end()) {
                return m_fileIdToFileNameMap[nFileId].c_str();
        } else {
                return s_nullFileName;
        }
}

u16 FiErrBaseMgr::GetFileId(const char *strFileName) {
        if (m_filenameToIdMap.find(strFileName) != m_filenameToIdMap.end()) {
                return m_filenameToIdMap[strFileName];
        } else {
                return s_nullFileId;
        }
}

u32 FiErrBaseMgr::GetUserErrCode(u32 nBaseErr) {
        if (IsErr(nBaseErr)) {
                assert(m_baseErrCodeToUserCode.find(nBaseErr) != m_baseErrCodeToUserCode.end());
                return m_baseErrCodeToUserCode[nBaseErr];
        } else {
                if (m_baseErrCodeToUserCode.find(nBaseErr) != m_baseErrCodeToUserCode.end()) {
                        return m_baseErrCodeToUserCode[nBaseErr];
                }else {
                        return 0;
                }
        }
}

const char* FiErrBaseMgr::GetBaseErrText(u32 nBaseErr) {
        if (IsErr(nBaseErr)) {
                assert(m_baseErrCodeToStr.find(nBaseErr) != m_baseErrCodeToStr.end());
                return m_baseErrCodeToStr[nBaseErr].c_str();
        } else {
                if (m_baseErrCodeToStr.find(nBaseErr) != m_baseErrCodeToStr.end()) {
                        return m_baseErrCodeToStr[nBaseErr].c_str();
                }else {
                        return "FiSuccess";
                }
        }
}

u32 FiErr::GetUserErrCode() {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        return pErrBaseMgr->GetUserErrCode(BaseErr());
}

string FiErr::StrFiText() {
        char arText[512];
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        sprintf(arText, "%s:%d,%s:%d ", pErrBaseMgr->GetBaseErrText(BaseErr()), SysErr(), FileName(), FileNo());
        return string(arText);
}

wstring FiErr::WstrFiText() {
        wchar_t arText[512];
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        swprintf(arText, 512, L"%s:%d,%s:%d ", pErrBaseMgr->GetBaseErrText(BaseErr()), SysErr(), FileName(), FileNo());
        return wstring(arText);
}

string FiErr::StrUserText() {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        FiDict *pDict = FiDict::GetInstance();
        return pDict->GetText((s_strDictPathPrefix + pErrBaseMgr->GetBaseErrText(BaseErr())).c_str());
}

u32  FiErr::MakeBaseCode_Erro(u8 nClasses, u16 nCode, u32 nUserCode, const char *strBaseErr) {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        u32 nBaseCode = pErrBaseMgr->MakeBaseCode_Erro(nClasses, nCode);
        pErrBaseMgr->AddBaseCodeInfo(nBaseCode, nUserCode, strBaseErr);
        return nBaseCode;
}

u32  FiErr::MakeBaseCode_Suc(u8 nClasses, u16 nCode, u32 nUserCode, const char *strBaseSuc) {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        u32 nBaseCode = pErrBaseMgr->MakeBaseCode_Suc(nClasses, nCode);
        pErrBaseMgr->AddBaseCodeInfo(nBaseCode, nUserCode, strBaseSuc);
        return nBaseCode;
}

u16 FiErr::GetFileId(const char *strFileName) {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        return pErrBaseMgr->GetFileId(strFileName);
}



inline u32 FiErr::BaseErr() {
        return (u32)(m_nMainCode >> 32);
}

inline u16 FiErr::FileId() {
        return (u16)((m_nMainCode & 0x00000000ffff0000) >> 16);
}

inline u16 FiErr::FileNo() {
        return (u16)(m_nMainCode & 0x000000000000ffff);
}

inline u32 FiErr::SysErr() {
        return (u32)m_nExtCode;
}

inline const char* FiErr::FileName() {
        FiErrBaseMgr *pErrBaseMgr = FiErrBaseMgr::GetInstance();
        return pErrBaseMgr->GetFileName(FileId());
}

FiErrFileNameRegitor::FiErrFileNameRegitor() {
        m_pErrBaseMgr = FiErrBaseMgr::GetInstance();
}

FiErrFileNameRegitor::FiErrFileNameRegitor(vector<FiFileNameTableUnit> vFileNameTable) {
        m_pErrBaseMgr = FiErrBaseMgr::GetInstance();
        AddFileTable(vFileNameTable);
}


inline void FiErrFileNameRegitor::AddFileTable(vector<FiFileNameTableUnit> vFileNameTable) {
        for (vector<FiFileNameTableUnit>::iterator it = vFileNameTable.begin(); it != vFileNameTable.end(); it++) {
                m_pErrBaseMgr->AddFileNameInfo(it->strFileName, it->nFileId);
        }
}
