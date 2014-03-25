
#include "netmsg.h"
#include "OSUtility.h"


void Tsheet::MarkSheet(char* buf, int buflen)
{
    ASSERT(buf);
    ASSERT(buflen < total_size);

    reserve_data = *(DWORD*)((char*)buf + buflen - sizeof(DWORD));
    *(DWORD*)((char*)buf + buflen - sizeof(DWORD)) = SHEET_END;
}

void Tsheet::RestoreSheet(char* buf, int buflen)
{
    ASSERT(buf);
    *(u32*)(buf + buflen - sizeof(u32)) = reserve_data;
}

