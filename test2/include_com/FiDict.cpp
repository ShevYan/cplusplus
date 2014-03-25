#include "FiDict.h"
#include <iostream>
#include <fstream>

CCritSec FiDict::s_lock;
FiDict *volatile FiDict::s_pInstance = NULL;
string FiDict::s_strDictPath("/FiDict");

FiDict::FiDict() {
    m_pCxt = CFiContext::GetInstance();
    string strFileName = GetExeDir(); 
    strFileName += string("/dict/") + m_pCxt->GetString(string(gFiLibConfigPath) + "/lang");
    ifstream  srcFile(strFileName.c_str());
    ofstream  dstFile((GetExeDir() + "/lang.xml").c_str());

    dstFile << srcFile.rdbuf();

    srcFile.close();
    dstFile.close();
}

FiDict* FiDict::GetInstance() {
    if (NULL == s_pInstance) {
        s_lock.Lock();
        if (NULL == s_pInstance) {
            s_pInstance = new FiDict();
        }
        s_lock.Unlock();

    }
    return s_pInstance;
}

string FiDict::GetText(const char *strId) {
    m_pCxt->SetDefault(s_strDictPath + strId, strId);
    return m_pCxt->GetString(s_strDictPath + strId);
}
