#include <iostream>
#include "tcpio.h"
using namespace std;

HANDLE gsock = NULL;
CTcpIo net;
char buf[512*1024];

bool _createcb(CTCPSocket *sokt, DWORD strip, void *userData)
{
}

void _exitcb(CTCPSocket *sokt, void *userData)
{
    cout << "socket exit " << sokt->GetSocketHandle() << endl;
}

void _recvcb(CMemItem *soktData, CTCPSocket *psokt, void *userData)
{
    //cout << "recv msg size " << soktData->GetDataRealSize() << endl;
}

bool _modrecvcb(CMemItem* sheetData,CMemItem *soktData, CTCPSocket *psokt, void *userData)
{
    //cout << "svr recv mod 3 msg size " << soktData->GetDataRealSize() << endl;

    ASSERT(soktData->GetBufferPtr()[soktData->GetDataRealSize()-1] == 0x1);

   

    net.Reply(sheetData,buf, 512, NULL, 0);
}

int main (int argc, char *argv[])
{
    CStdString ip("10.200.103.89");
    BOOL ret = 0;
    if (argc < 3 )
    {
    cout << "usage:" << endl;
    cout << "   testtcpio <s> <port>" << endl;
    cout << "   testtcpio <c> <ip> <port>" << endl;

	exit( 0);
    }
    memset(buf,0x2, 512*1024);


    net.RegisterModule(3, (SOCKETCREATE_FUNC)NULL, _modrecvcb, (SOCKETEXIT_FUNC)NULL);

    if (argv[1][0] == 'c')
    {
        ip = argv[2];
		net.StartClient(atoi(argv[3]));
        Sleep(1000);
    }
    else
    {
		net.StartServer(atoi(argv[2]));
    }

    CMemItem buff;
    char tblk[512*1024];
    memset(tblk, 0x1,512*1024);
    buff.AddData((byte*)tblk,512*1024);

    if (argv[1][0] == 'c')
    {
	do
	{
	    
	    call_cxt cxt;
	    cxt.in_modid = 3;
	    cxt.in_buf1 = (char*)buff.GetBufferPtr();
	    cxt.in_buf1len = buff.GetDataRealSize();
	    cxt.inway.vecSrc.push_back(ConvertIPtoDWORD(ip));
	    cxt.inway.vecDst.push_back(ConvertIPtoDWORD(ip));
	    cxt.inway.time_out = 6000;

	    if(0 == net.Call(cxt))
	    {
		//cout << "send buff size success " << buff.GetDataRealSize()  << endl;
		ASSERT(cxt.out_buf->GetBufferPtr()[0] == 0x2);
	    }
	    else
	    {
		//cout << "send buff size failed " << buff.GetDataRealSize()  << endl;
		Sleep(1000);
	    }
	    
	}
	while (1); 
    }
    int i = 0;
    cin >> i;

    return(0);
}

