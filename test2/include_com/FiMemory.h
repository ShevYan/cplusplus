#ifndef __FIMEMORY_H__
#define __FIMEMORY_H__

#include "dblinklist.h"
#include <vector>
#include "OSThread.h"

class CMemItem
{
public:
    ~CMemItem();
    CMemItem();
public:
    void Reset();
    BOOL Clone(CMemItem *pSrcBufferListItem);

    BOOL MoveUnusedDataToBufTop();
    

    BOOL SetDataRealSize(DWORD dwDataRealSize);
    BOOL SetBufferSize(DWORD dwBufSize);
    BOOL SetCurPos(DWORD dwCurPos);
    void SetBufData(PBYTE pData, DWORD dwSize);

    DWORD GetCurPosition();
    PBYTE GetUnusedBufPtr();
    PBYTE GetCurDataPtr();
    DWORD GetUnusedDataSize();
    PBYTE GetBufferPtr();
    DWORD GetBufferSize();
    int GetUnusedBufSize();
    DWORD GetDataRealSize();


    PBYTE DetachBuffer();
    void AttachBuffer(PBYTE pData, DWORD dwSize, CMemItem *pExtAttachedBufferItem = NULL);

    BOOL ChangeBuffer(PBYTE& pBuffer, DWORD dwBufSize);
    BOOL AddDataVirtual(PBYTE pDataBuf, DWORD dwSize);
    void AttachBufferEx(PBYTE pData, DWORD dwSize, CMemItem *pExtAttachedBufferItem = NULL);
    DWORD AddExtAttach();

    BOOL AddData(BYTE *pDataBuf, DWORD dwSize);

    BOOL IsExtAttachedBuf();

public:

    DWORD m_dwBufSize;
    int m_dwDataRealSize;
    DWORD m_dwCurPos;
    PBYTE m_ptrData; //data address
    PBYTE m_ptrBackData; //back data address
    DWORD m_dwBackBufSize;
    BOOL m_bBufferIsAttached;

    CMemItem *m_pAttachedBufferItem;
    DWORD m_dwExtAttachNum;
}; 


#endif 
