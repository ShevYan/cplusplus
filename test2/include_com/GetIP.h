#ifndef __GETIP_H__
#define __GETIP_H__
#include <string>
#include <vector>
using namespace std;

class CGetIP
{
public:
	static int GetIPV4(vector <std::string> &IPList);
	static int GetIPV6(vector <std::string> &IPList);
private:
	CGetIP(){};
	~CGetIP(){};
};
#endif
