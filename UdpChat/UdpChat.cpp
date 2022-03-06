#pragma comment ( lib, "ws2_32.lib" )
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ws2tcpip.h>
#include "Client.h"
//#include <thread>


int main(int argc, char** argv)
{
        WORD wSrcPort, wDstPort;

        if (argc < 4)
        {
            /*std::cout << "Usage : udpchart [filename] [srcport] [dstport] optional:[OtherUser]\n";
            return -1;*/
            argc = 5;
            argv = new char* [5];
            char s1[] = "test";
            char s2[] = "8000";
            char s3[] = "9000";
            char s4[] = "tests";
            argv[1] = s1;
            argv[2] = s2;
            argv[3] = s3;
            argv[4] = s4;
        }
        std::string name = argv[1];
        wSrcPort = (WORD)atoi(argv[2]);
        wDstPort = (WORD)atoi(argv[3]);

        std::string otherName;
        if (argc == 5)
            otherName = argv[4];
        if (argc > 5)
            std::cout << "Usage : udpchart [filename] [srcport] [dstport] optional:[OtherUser]\n";

        Client c(name, otherName, std::cin, std::cout, std::cerr);

        c.Connect(wSrcPort, wDstPort);


        printf("something gonna wrong!");
        return 0;
}