#ifndef __NETMSG_H__
#define __NETMSG_H__

#include "BaseType.h"

#pragma pack(push,1)

#define SHEET_HEAD      0x46494353
#define SHEET_END       0x56055605
#define SHEET_MAX_SIZE  0x1000000  /// 16M
#define SHEET_VERION    0x1
#define SHEET_CALL      0x1
#define SHEET_REPLY     0x2
#define SHEET_NOTIFY    0x3
#define SHEET_ROUTE		0x4

#define SHEET_ENABLE_TUNNEL 0x1

#define TSHEET_MSG_OFFS(sheet) ((char*)sheet + sizeof(Tsheet) + sheet->src_ip_num*sizeof(DWORD) + sheet->dst_ip_num*sizeof(DWORD))

class Tsheet
{     
/// must use special alloc sheet function
private:
    Tsheet(){}
    ~Tsheet(){}
public:
    void MarkSheet(char* buf, int buflen);
    void RestoreSheet(char* buf, int buflen);
public:
    u32 head;
    u32 total_size;
    u32 sheet_size;
    u32 identity;
    u32 version;
    u32 pid;

    /// tunnel just as read user buf ,directly use tunnel when recv i
    /// reply must check old tunnelsize >= real data size then set real data size to tunnelsize
    u32 tunnelsize;
    /// only svr reply two buffer can set tunnel 1, otherwise tunnel must 0
    u8 tunnel;
    u8  direction;
    u8  oneoff; /// once
    /// user set
    u32 reg_module_type;/// when type msg return call type_call_back_func

    u32 out_time;
    u8  query; /// when client can not recv svr call back, then query svr sheet_num 
    u8 src_ip_num;
    u8 dst_ip_num;

    u64 trace_back; /// trace back recv msg socket

    u32 carry_msg_len; /// user msg header len + user msg data len

    /// insize set 
    u64 sheet_num;

    /// after check msg , restore _len _data to check buff position
    u32 reserve_data;
};

#pragma pack(pop)

#endif
