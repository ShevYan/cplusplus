
#include "msgsite.h"

CMsgSite::~CMsgSite()
{
	map<HANDLE, MSG_CXT *>::iterator pPos;

	for (pPos = m_mapSockMsg.begin(); pPos != m_mapSockMsg.end(); pPos++)
	{
		_SafeDeletePtr(pPos->second->pMsgHead);
		_SafeDeletePtr(pPos->second);
	}

	m_mapSockMsg.clear();
}

BOOL CMsgSite::InsertSock(HANDLE hSock)
{
	BOOL bInsertSucc = TRUE;
	map<HANDLE, MSG_CXT *>::iterator it;

	m_lk.OwnerLock();
	it = m_mapSockMsg.find(hSock);

	if (it != m_mapSockMsg.end())
	{
		MASTRACE(_T("[FAIL]Socket<0x%08x> existed!"), (unsigned int)(long)hSock);

		bInsertSucc = FALSE;
	} else
	{
		MSG_CXT *pNewMsgCxt = new MSG_CXT;
		ASSERT(pNewMsgCxt);

		pNewMsgCxt->pMsgHead   = new CMemItem;
		pNewMsgCxt->pTunnelBuf = new CMemItem;
		pNewMsgCxt->pUserData  = new CMemItem;

		ASSERT(pNewMsgCxt->pMsgHead && pNewMsgCxt->pTunnelBuf);

		m_mapSockMsg[hSock] = pNewMsgCxt;
		MASTRACE(_T("[OK] insert hSock<0x%08x> ----pMsgCxt<0x%p>"), (unsigned int)(long)hSock, pNewMsgCxt);
	}

	m_lk.LeaveLock();

	return bInsertSucc;
}

void CMsgSite::RemoveSock(HANDLE hSock)
{
	map<HANDLE, MSG_CXT *>::iterator it;

	m_lk.OwnerLock();

	it = m_mapSockMsg.find(hSock);

	if (it == m_mapSockMsg.end())
	{
		MASTRACE(_T("[FAIL]Socket<0x%08x> not existed!"), (unsigned int)(long)hSock);
	} else
	{
		MSG_CXT *pMsgCxt = it->second;

		m_mapSockMsg.erase(it);

		MASTRACE(_T("[OK] remove hSock<0x%08x> ----pMsgCxt<0x%p>"), (unsigned int)(long)hSock, pMsgCxt);

		_SafeDeletePtr(pMsgCxt->pMsgHead);
		_SafeDeletePtr(pMsgCxt->pTunnelBuf);

		_SafeDeletePtr(pMsgCxt);
	}

	m_lk.LeaveLock();
}

BOOL CMsgSite::GetMsg(u32& identify, CShareLock& modlock, map<DWORD, CModuleInfo*>& modmap, 
                      CMemItem *pMsgBuf, CTCPSocket *psokt, void *userData,    MSG_CXT* pMsgCxt)
{
	PBYTE pBegin		= NULL;
	PBYTE pEnd			= NULL;
	DWORD dwAddSize		= 0;
	BOOL bRet			= TRUE;
	CModuleInfo	*mod = NULL;
	DWORD dwSheetHeadSize = sizeof(Tsheet);

	ASSERT(pMsgCxt->pMsgHead && pMsgCxt->pUserData);

	pBegin = pMsgBuf->GetBufferPtr();
	pEnd = pBegin + pMsgBuf->GetDataRealSize();


	while (pBegin < pEnd || pMsgCxt->dwMsgSect == MSG_END)
	{
		switch (pMsgCxt->dwMsgSect)
		{
		case MSG_HEAD:
			if (pMsgCxt->pMsgHead->GetDataRealSize() < 3 * sizeof(DWORD))
			{
				dwAddSize = (pEnd - pBegin + pMsgCxt->pMsgHead->GetDataRealSize()) <= 3 * sizeof(DWORD) ?
				   pEnd - pBegin : 3 * sizeof(DWORD) - pMsgCxt->pMsgHead->GetDataRealSize();

				pMsgCxt->pMsgHead->AddData(pBegin, dwAddSize);
				pBegin += dwAddSize;

				if (pMsgCxt->pMsgHead->GetDataRealSize() != 3 * sizeof(DWORD))
				{
					continue;
				}
			}

			if (pMsgCxt->dwMsgSize == 0)
			{
				DWORD dwMsgHead = *(DWORD *)(pMsgCxt->pMsgHead->GetBufferPtr());
				/// fixme : header flag error, should return error and close msg

				ASSERT(dwMsgHead == SHEET_HEAD);
				if (dwMsgHead != SHEET_HEAD)
				{
					bRet = FALSE;
					goto _aleave;
				}

				pMsgCxt->dwMsgSize   = *(DWORD *)(pMsgCxt->pMsgHead->GetBufferPtr() + sizeof(DWORD));
				pMsgCxt->dwSheetSize = *(DWORD *)(pMsgCxt->pMsgHead->GetBufferPtr() + sizeof(DWORD) + sizeof(DWORD));

				/// set pMsgCxt->pMsgHead buffer size in range ,will greatly reduce copy times
				ASSERT(pMsgCxt->dwMsgSize < SHEET_MAX_SIZE);

			}

			/// construct sheet head
			if (pMsgCxt->pMsgHead->GetDataRealSize() < pMsgCxt->dwSheetSize)
			{
				dwAddSize = (pEnd - pBegin + pMsgCxt->pMsgHead->GetDataRealSize()) <= pMsgCxt->dwSheetSize  ?
				   pEnd - pBegin : pMsgCxt->dwSheetSize  - pMsgCxt->pMsgHead->GetDataRealSize();

				pMsgCxt->pMsgHead->AddData(pBegin, dwAddSize);
				pBegin += dwAddSize;

				if (pMsgCxt->pMsgHead->GetDataRealSize() != pMsgCxt->dwSheetSize)
				{
					continue;
				}
			}

			if (pMsgCxt->sh == NULL)
			{
				pMsgCxt->sh  = (Tsheet *)pMsgCxt->pMsgHead->GetBufferPtr();
			}

			ASSERT(pMsgCxt->dwSheetSize == (sizeof(Tsheet) + pMsgCxt->sh->src_ip_num * sizeof(DWORD) + pMsgCxt->sh->dst_ip_num * sizeof(DWORD)));
			ASSERT(pMsgCxt->pMsgHead->GetDataRealSize() == pMsgCxt->dwSheetSize);


			/// set user pre pUserData recv buffer size
			if (pMsgCxt->sh->tunnel == SHEET_ENABLE_TUNNEL)
			{
				ASSERT(pMsgCxt->sh->direction == SHEET_REPLY);
				pMsgCxt->dwUserDataSize = pMsgCxt->dwMsgSize - pMsgCxt->sh->tunnelsize - pMsgCxt->dwSheetSize;
			} else
			{
				pMsgCxt->dwUserDataSize = pMsgCxt->dwMsgSize -  pMsgCxt->dwSheetSize;
			}

			pMsgCxt->pUserData->SetBufferSize(pMsgCxt->dwUserDataSize);

			pMsgCxt->dwMsgSect = (pMsgCxt->dwMsgSize == pMsgCxt->pMsgHead->GetDataRealSize()) ? MSG_END : MSG_USER_BODY;
			break;

		case MSG_USER_BODY:

			/// construct sheet carry user buffer head
			if (pMsgCxt->pUserData->GetDataRealSize() < pMsgCxt->dwUserDataSize)
			{
				dwAddSize = (pEnd - pBegin + pMsgCxt->pUserData->GetDataRealSize()) <= pMsgCxt->dwUserDataSize ?
				   pEnd - pBegin : pMsgCxt->dwUserDataSize - pMsgCxt->pUserData->GetDataRealSize();

				pMsgCxt->pUserData->AddData(pBegin, dwAddSize);
				pBegin += dwAddSize;

				if (pMsgCxt->pUserData->GetDataRealSize() != pMsgCxt->dwUserDataSize)
				{
					continue;
				}
			}

			/// throw other msg
			//if (identify != pMsgCxt->sh->identity)
			//{
			//	pMsgCxt->bTunnelUnused = TRUE;
			//}

			pMsgCxt->dwMsgSect = (pMsgCxt->dwMsgSize == (pMsgCxt->dwSheetSize + pMsgCxt->dwUserDataSize)) ? MSG_END : MSG_TUNNEL_BODY;
			break;

		case MSG_TUNNEL_BODY:
			/// check if use tunnel buffer, then recv buf to tunnel buff
			if (pMsgCxt->sh->direction == SHEET_REPLY && pMsgCxt->sh->tunnel && pMsgCxt->sh->tunnelsize)
			{
				/// get mod
				if (NULL == pMsgCxt->mod)
				{
					map<DWORD, CModuleInfo *>::iterator it;
					modlock.ShareLock();

					if (modmap.end() != (it = modmap.find(pMsgCxt->sh->reg_module_type)))
					{
						pMsgCxt->mod = it->second;
					}

					modlock.LeaveLock();

					ASSERT(pMsgCxt->mod);

					if (((CModuleInfo *)pMsgCxt->mod)->ThrowReplyPkt(pMsgCxt->sh->sheet_num))
					{
						pMsgCxt->bTunnelUnused = TRUE;
					} else
					{
						ReqCxt *cxt = ((CModuleInfo *)pMsgCxt->mod)->GetReqParam(pMsgCxt->sh->sheet_num);
						ASSERT(cxt);

						ASSERT(cxt->tunnelbuflen >= pMsgCxt->sh->tunnelsize);
						cxt->tunnelbuflen = pMsgCxt->sh->tunnelsize;
						pMsgCxt->pTunnelBuf->AttachBufferEx((byte *)cxt->tunnelbuf, pMsgCxt->sh->tunnelsize);
					}
				}

				/// construct sheet carry user tunnel buffer
				if (pMsgCxt->pTunnelBuf->GetDataRealSize() < pMsgCxt->sh->tunnelsize)
				{
					dwAddSize = (pEnd - pBegin + pMsgCxt->pTunnelBuf->GetDataRealSize()) <= pMsgCxt->sh->tunnelsize ?
					   pEnd - pBegin : pMsgCxt->sh->tunnelsize - pMsgCxt->pMsgHead->GetDataRealSize();

					if (pMsgCxt->bTunnelUnused)
					{   /// just throw
						pMsgCxt->pTunnelBuf->AddDataVirtual(pBegin, dwAddSize);
					} else
					{
						pMsgCxt->pTunnelBuf->AddData(pBegin, dwAddSize);
					}

					pBegin += dwAddSize;

					if (pMsgCxt->pTunnelBuf->GetDataRealSize() != pMsgCxt->sh->tunnelsize)
					{
						continue;
					}
				}

				pMsgCxt->dwMsgSect = MSG_END;
			} else
			{
				ASSERT(0);
			}

			break;

		case MSG_END:

			if (pMsgCxt->bTunnelUnused)
			{
				_SafeDeletePtr(pMsgCxt->pMsgHead);
			} else
			{
				ASSERT(pMsgCxt->dwMsgSize == pMsgCxt->sh->total_size);

				if (pMsgCxt->sh->direction == SHEET_REPLY && pMsgCxt->sh->tunnel && pMsgCxt->sh->tunnelsize)
				{
					ASSERT(pMsgCxt->pTunnelBuf->GetDataRealSize() == pMsgCxt->sh->tunnelsize);
					ASSERT(*(u32 *)(pMsgCxt->pTunnelBuf->GetBufferPtr() + pMsgCxt->sh->tunnelsize - sizeof(u32)) == SHEET_END);

					if (*(u32 *)(pMsgCxt->pTunnelBuf->GetBufferPtr() + pMsgCxt->sh->tunnelsize - sizeof(u32)) != SHEET_END)
					{
						_SafeDeletePtr(pMsgCxt->pMsgHead);
						bRet = FALSE;

						goto _aleave;
					}

					/// restore check data
					pMsgCxt->sh->RestoreSheet((char *)pMsgCxt->pTunnelBuf->GetBufferPtr(), pMsgCxt->sh->tunnelsize);

					pMsgCxt->pTunnelBuf->DetachBuffer();

					((CModuleInfo *)pMsgCxt->mod)->SetTunnelBufferStatus(pMsgCxt->sh->sheet_num, FALSE);
				} else
				{
					ASSERT(*(u32 *)(pMsgCxt->pUserData->GetBufferPtr() + pMsgCxt->dwUserDataSize - sizeof(u32)) == SHEET_END);
					/*if (*(u32 *)(pMsgCxt->pTunnelBuf->GetBufferPtr() + pMsgCxt->sh->tunnelsize - sizeof(u32)) != SHEET_END)
					{
						_SafeDeletePtr(pMsgCxt->pMsgHead);
						bRet = FALSE;

						goto _aleave;
					}*/
					/// restore check data
					pMsgCxt->sh->RestoreSheet((char *)pMsgCxt->pUserData->GetBufferPtr(), pMsgCxt->dwUserDataSize);
				}

				pMsgCxt->sh->trace_back = (u64)psokt->GetSocketHandle();

				/// user call back must take away msg quickly
				/// callback masg
				m_callback(pMsgCxt->pMsgHead, pMsgCxt->pUserData, psokt, userData);
			}

			pMsgCxt->Reset();
			break;

		default:
			ASSERT(0);
		}
	}
_aleave:

	return bRet;
}

MSG_CXT *CMsgSite::Lock_sock(CTCPSocket *psokt)
{
        ASSERT(psokt);
        map<HANDLE, MSG_CXT *>::iterator it;
        MSG_CXT *pMsgCxt = NULL;

	m_lk.ShareLock();

	if((it = m_mapSockMsg.find(psokt->GetSocketHandle())) != m_mapSockMsg.end())
        {
            pMsgCxt = it->second;

            ASSERT(pMsgCxt->pMsgHead && pMsgCxt->pUserData);

            pMsgCxt->lk.Lock();
        }
        else
        {
            printf("CMsgSite::Lock_sock can not find pair sokcet msg_cxt return null !!!!!\n");
            pMsgCxt = NULL;
        }

        return pMsgCxt;
}

void CMsgSite::Unlock_sock(MSG_CXT *pMsgCxt)
{
    if (NULL != pMsgCxt)
    {
        pMsgCxt->lk.Unlock();
    }

    m_lk.LeaveLock();
}
