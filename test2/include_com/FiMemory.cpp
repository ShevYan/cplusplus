
#include "FiMemory.h"
#include "OSUtility.h"

BOOL CMemItem::AddData(byte *pDataBuf, DWORD dwSize)
{ 
    BOOL br = TRUE;

    if (m_dwDataRealSize + dwSize > m_dwBufSize)
    { //no enough size
		SetBufferSize(m_dwDataRealSize + dwSize);
    }
    memcpy(m_ptrData + m_dwDataRealSize, pDataBuf, dwSize);
    m_dwDataRealSize += dwSize;

    return br;
}


CMemItem::~CMemItem()
{
    if (m_bBufferIsAttached)
    {
		DetachBuffer();
    }

    _SafeDeletePtrArr(m_ptrData);
}

CMemItem::CMemItem()
{
    m_ptrData = NULL;

    m_dwBufSize = 0;
    m_dwDataRealSize = 0;
    m_dwCurPos = 0;

    m_bBufferIsAttached = FALSE;

    m_ptrBackData = NULL;
    m_dwBackBufSize = 0;

    m_pAttachedBufferItem = NULL;
    m_dwExtAttachNum = 0;
};


BOOL CMemItem::SetBufferSize(DWORD dwBufSize)
{
    BOOL br = TRUE;

    ASSERT(dwBufSize > 0);

    if (m_ptrData == NULL)
    {
		m_ptrData = new BYTE[dwBufSize];

		if (m_ptrData == NULL)
		{
			return FALSE;
		}

		m_dwBufSize = dwBufSize;
		m_dwDataRealSize = 0;
		m_dwCurPos = 0;
		br = TRUE;
    } else if (dwBufSize <= m_dwBufSize)
    {
		m_dwBufSize = dwBufSize;
		br = TRUE;
    } else
    {
		PBYTE ptr = new BYTE[dwBufSize];

		if (ptr == NULL)
		{
			return FALSE;
		}

		memcpy(ptr, m_ptrData, m_dwDataRealSize);
		_SafeDeletePtrArr(m_ptrData);

		m_ptrData = ptr;
		m_dwBufSize = dwBufSize;
		br = TRUE;
    }

    return br;
}

BOOL CMemItem::Clone(CMemItem *pSrcBufferListItem)
{
    BOOL br = TRUE;

    m_dwDataRealSize = 0;
    m_dwCurPos = 0;

    SetBufferSize(pSrcBufferListItem->m_dwBufSize);
    memcpy(m_ptrData, pSrcBufferListItem->m_ptrData, pSrcBufferListItem->m_dwBufSize);
    m_dwDataRealSize = pSrcBufferListItem->m_dwDataRealSize;

    return br;
}

void CMemItem::Reset()
{
    if (m_ptrBackData && m_dwBackBufSize > 0)
    {
		DetachBuffer();
    }

    m_dwDataRealSize = 0;
    m_dwCurPos = 0;

    if (m_dwBufSize > 0)
    {
		OSZeroMemory(m_ptrData, m_dwBufSize);
    }
}

void CMemItem::SetBufData(PBYTE pData, DWORD dwSize)
{
    if (NULL != m_ptrData)
    {
		delete m_ptrData;
		m_ptrData = NULL;
    }

    m_dwDataRealSize = dwSize;
    m_dwBufSize = dwSize;
    m_ptrData = pData;
}

void CMemItem::AttachBuffer(byte *pData, DWORD dwSize, CMemItem *pExtAttachedBufferItem)
{
    m_ptrBackData = m_ptrData;
    m_dwBackBufSize = m_dwBufSize;

    m_dwDataRealSize = dwSize;
    m_ptrData = pData;
    m_dwBufSize = dwSize;
    m_bBufferIsAttached = TRUE; 
}

byte* CMemItem::DetachBuffer()
{
    byte *pData = NULL;
    pData = m_ptrData;

    m_ptrData = m_ptrBackData;
    m_dwBufSize = m_dwBackBufSize;
    m_dwDataRealSize = 0;

    m_bBufferIsAttached = FALSE;

    return pData;
}

BOOL CMemItem::SetDataRealSize(DWORD dwDataRealSize)
{
    BOOL br = TRUE;

    ASSERT(dwDataRealSize <= m_dwBufSize);

    m_dwDataRealSize = dwDataRealSize;
    return br;
}; 

BOOL CMemItem::SetCurPos(DWORD dwCurPos)
{
    BOOL br = TRUE;

    if (dwCurPos <= m_dwDataRealSize)
    {
		m_dwCurPos = dwCurPos;
		br = TRUE;
    } else
    {
		br = FALSE;
    }

    return br;
}

DWORD CMemItem::GetDataRealSize()
{
    return m_dwDataRealSize;
}

int CMemItem::GetUnusedBufSize()
{
    return m_dwBufSize - m_dwDataRealSize;
}

DWORD CMemItem::GetBufferSize()
{
    return m_dwBufSize;
}


PBYTE CMemItem::GetBufferPtr()
{
    return m_ptrData;
}

DWORD CMemItem::GetUnusedDataSize()
{
    ASSERT(m_dwCurPos <= m_dwDataRealSize);

    return m_dwDataRealSize - m_dwCurPos;
}

PBYTE CMemItem::GetCurDataPtr()
{
    return (PBYTE)(m_ptrData + m_dwCurPos);
}


BOOL CMemItem::ChangeBuffer(PBYTE& pBuffer, DWORD dwBufSize)
{
    pBuffer = GetBufferPtr();

    ASSERT(dwBufSize == m_dwDataRealSize);

    m_ptrData = new BYTE[m_dwBufSize];

    m_dwDataRealSize = 0;

    return TRUE;
}

BOOL CMemItem::AddDataVirtual(PBYTE pDataBuf, DWORD dwSize)
{
    BOOL br = TRUE;

    if (0 == dwSize)
    {
		return TRUE;
    }

    if (m_dwDataRealSize + dwSize > m_dwBufSize)
    {
		SetBufferSize(m_dwDataRealSize + dwSize);
    }

    m_dwDataRealSize += dwSize;

    return br;
}

void CMemItem::AttachBufferEx(PBYTE pData, DWORD dwSize, CMemItem *pExtAttachedBufferItem)
{
    ASSERT(!m_bBufferIsAttached);

    m_ptrBackData = m_ptrData;
    m_dwBackBufSize = m_dwBufSize;

    m_dwDataRealSize = 0;
    m_ptrData = pData;
    m_dwBufSize = dwSize;
    m_bBufferIsAttached = TRUE; //use outside buffer

    m_pAttachedBufferItem = pExtAttachedBufferItem;

    if (m_pAttachedBufferItem)
    {
	m_pAttachedBufferItem->AddExtAttach();
    }
}

DWORD CMemItem::AddExtAttach()
{
    m_dwExtAttachNum++;

    return m_dwExtAttachNum;
}

byte* CMemItem::GetUnusedBufPtr()
{
    return (byte *)(m_ptrData + m_dwDataRealSize);
}

BOOL CMemItem::MoveUnusedDataToBufTop()
{
    BOOL br = TRUE;

    memmove(m_ptrData, m_ptrData + m_dwCurPos, m_dwDataRealSize - m_dwCurPos);

    m_dwDataRealSize = m_dwDataRealSize - m_dwCurPos;
    m_dwCurPos = 0;

    return br;
}

DWORD CMemItem::GetCurPosition()
{
    return m_dwCurPos;
}


BOOL CMemItem::IsExtAttachedBuf()
{
    return m_bBufferIsAttached;
}

