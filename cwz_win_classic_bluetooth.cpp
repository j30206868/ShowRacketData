#include "cwz_win_classic_bluetooth.h"

cwz_c_blue::cwz_c_blue(){
    //00001101-0000-1000-8000-00805f9b34fb
    guid.Data1 = 0x00001101;
    //00001101-
    guid.Data2 = 0x0000;
    //0000-
    guid.Data3 = 0x1000;
    //1000-
    guid.Data4[0] = 0x80;
    guid.Data4[1] = 0x00;
    //8000-
    guid.Data4[2] = 0x00;
    guid.Data4[3] = 0x80;
    guid.Data4[4] = 0x5f;
    guid.Data4[5] = 0x9b;
    guid.Data4[6] = 0x34;
    guid.Data4[7] = 0xfb;
    //00805f9b34fb

    //addr = 0X201511303545;//HC-05
    addr = 0x001209257365;//HC-06
}

int cwz_c_blue::connect(){
    std::cout << "cwz_c_blue: start to connect bluetooth device ";
    printf("0x%x...", addr);
    std::cout << std::endl;

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        std::cout << "cwz_c_blue: Unable to load Winsock! Error code: " << WSAGetLastError() << std::endl;
        return 0;
    }
    //else
    //    printf("WSAStartup() works, Winsock lib loaded!\n");

    skt = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (skt == INVALID_SOCKET)
    {
        std::cout << "cwz_c_blue: Failed to create socket, error code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }
    //else
    //    printf ("cwz_c_blue: Bluetooth socket created.\n");

    memset (&sab, 0, sizeof(sab));
    sab.addressFamily  = AF_BTH;
    sab.btAddr = addr;
    sab.serviceClassId = guid;
    sab.port = 1;

    while(::connect (skt, (SOCKADDR *)&sab, sizeof(sab)) == SOCKET_ERROR){
        std::cout << "cwz_c_blue: bluetooth connection failed, try to connect again after 1 second. error code: " << WSAGetLastError() << std::endl;
        Sleep(1000);
    }
    std::cout << "cwz_c_blue: bluetooth socket is successfully connected." << std::endl;
    return 1;
}

void cwz_c_blue::disconnect(){
    if(closesocket(skt) == 0 && WSACleanup () == 0)
         std::cout << "cwz_c_blue: socket closed!" << std::endl;
}
