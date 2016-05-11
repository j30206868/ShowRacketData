#ifndef CWZ_WIN_CLASSIC_BLUETOOTH
#define CWZ_WIN_CLASSIC_BLUETOOTH

#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <stdio.h>

#include <string>

#include <iostream>

typedef ULONGLONG bt_addr, *pbt_addr, BT_ADDR, *PBT_ADDR;

class cwz_c_blue{
private:
    GUID guid;
    WSADATA wsd;
    SOCKET skt;
    SOCKADDR_BTH sab;
    BT_ADDR addr;
public:
    cwz_c_blue();
    int connect(bool &stopFlag);
    inline void send(std::string content);
    inline int  receive(char *buf, int buf_len);
    void disconnect();
};

inline void cwz_c_blue::send(std::string content){
    int result = ::send(skt, content.c_str(), content.length(), 0 );
    if (result == SOCKET_ERROR) {
        std::cout << "cwz_c_blue::send error, error code: " << WSAGetLastError() << std::endl;
    }
}

inline int cwz_c_blue::receive(char *buf, int buf_len)//return received data length
{
    return ::recv(skt, buf, buf_len, 0);
}

#endif // CWZ_WIN_CLASSIC_BLUETOOTH

