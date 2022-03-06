#include "Client.h"
#pragma comment ( lib, "ws2_32.lib" )
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ws2tcpip.h>
#include <thread>
#include <fstream>
#include <format>
#include <string_view>
#include <filesystem>
#include <stdexcept>
#include <random>
#include <utility>
#include <future>
#include <chrono>

namespace {

    namespace fs = std::filesystem;

#define __T(x)      L ## x

#define _T(x)       __T(x)

#define IP_TARGET "127.0.0.1"

    std::string gen_random(const int len) {

        std::random_device rd;
        std::mt19937 mersenne(rd());

        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string tmp_s;
        tmp_s.reserve(len);

        for (int i = 0; i < len; ++i) {
            tmp_s += alphanum[mersenne() % (sizeof(alphanum) - 1)];
        }

        return tmp_s;
    }

    class StopTread {

        template<typename Tfunc, typename... Args>
        void Runnable(Tfunc func, Args&&... args) {
            while (!isStop)
            {
                func(std::forward<Args>(args)...);
            }
        }

        bool isStop = false;

    public:
        StopTread() { isStop = false; }
        ~StopTread() { Stop(); }

        void Stop() {
            isStop = true;
        }

        template<typename Tfunc, typename... Args>
        void Run(Tfunc func, Args&&... args) {
            std::thread writethread = std::thread([&]() { this->Runnable(func, std::forward<Args>(args)...); });
            writethread.detach();
        }
    };
}

Client::Client(std::string name, std::string otherUser, std::istream& in, std::ostream& out, std::ostream& err) : _in(in), _out(out), _err(err) {
    _name = name;
    _name.resize(NAME_MAX_LEN, ' ');
    _otherUser = otherUser;
}

Client::Message Client::RecvMessage(SOCKET sock)
{
    SOCKADDR_IN recvAddr = { 0 };
    int iRet, iRecvSize;

    iRecvSize = sizeof(recvAddr);
    char buf[1024];
    iRet = recvfrom(sock, buf, 1024, 0, (SOCKADDR*)&recvAddr, &iRecvSize);


    if (iRet == SOCKET_ERROR)
        return Message{ "", 0, MessageText(std::vector<int>()) };

    buf[iRet] = '\0';

    char str[INET_ADDRSTRLEN + 1] = { 0 };
    inet_ntop(AF_INET, &recvAddr.sin_addr, str, INET_ADDRSTRLEN);
    std::string name(str, INET_ADDRSTRLEN);

    return Message{ name, htons(recvAddr.sin_port), MessageText(std::vector<int>((int*)buf,  (int*)buf + iRet / sizeof(int))) };
}

void Client::SendTo(SOCKET sock, WORD wDstPort, std::vector<int> str) {
    SOCKADDR_IN sendAddr = { 0 };

    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(wDstPort);
    InetPton(AF_INET, _T(IP_TARGET), &sendAddr.sin_addr.s_addr);

    sendto(sock, (char*)str.data(), str.size() * sizeof(int), 0, (SOCKADDR*)&sendAddr, sizeof(sendAddr));
    
}


SOCKET Client::MakeSocket(WORD wPort) {
    SOCKET sock = (SOCKET)NULL;
    SOCKADDR_IN addr = { 0 };

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        return (SOCKET)NULL;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(wPort);
    InetPton(AF_INET, _T(IP_TARGET), &addr.sin_addr.s_addr);

    if (bind(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return (SOCKET)NULL;
    }

    return sock;
}

void Client::SetOtherUserName(std::string name)
{
    _otherUser = name;
}

BOOL Client::TrySendData(SOCKET sock, WORD wDstPort) {
    RestoreMessage();
    std::string text;
    std::getline(_in, text);

    if (text[0] == 'q')
        return FALSE;

    SendTo(sock, wDstPort, otherRSA.RSA_Encrypt(Convert(_name + separate + text)));

    text = "you: " + text;

    Save(text + "\n");

    return TRUE;
}

void Client::RecvThread(SOCKET sock)
{
    while (!bEnd)
    {
        Message m = RecvMessage(sock);

        if (m.ip.empty())
            continue;

        std::string name = strip(Convert(rsa.RSA_Decrypt(m.msg.n.Name())));

        if (name.compare(_otherUser) != 0)
            continue;

        std::string temp = "[" + name + "] : " + Convert(rsa.RSA_Decrypt(m.msg.n.Msg())) + "\n";
        Save(temp);
        RestoreMessage();
    }

    _err << "Recv Thread End\n";
}

auto Client::Async(SOCKET sock) {
    return std::async(std::launch::async, [sock, this]() {
        Message m;
        do {
            m = RecvMessage(sock);
        } while (m.ip.empty());
        return m;
        });
};


std::string Client::Connect(SOCKET sock, WORD wDstPort) {
    using namespace std::chrono_literals;
    std::string resultName;
    std::string key = gen_random(10);
    while (true)
    {
        std::future<Message> ret = Async(sock);

        auto openKey = rsa.GetOpenKey();
        auto k = std::to_string(openKey.k);
        k.resize(KEY_LEN, ' ');
        auto n = std::to_string(openKey.n);
        n.resize(KEY_LEN, ' ');

        std::string idStr = k + separate + n;

        std::string payload = gen_random(10);

        std::string connectStr = idStr + separate + payload;

        SendTo(sock, wDstPort, Convert(connectStr));

        if (std::future_status::ready != ret.wait_for(1s))
            continue;

        Message m = ret.get();
        if (!m.ip.empty())
        {

            SendTo(sock, wDstPort, Convert(connectStr));
            Message nm;

            RSA otherRsa;
        
            otherRsa.RSAInitializeByOpenKey({ stoi(strip(Convert(m.msg.k.KKey()))), stoi(strip(Convert(m.msg.k.NKey()))) });
            std::string sendMsg = idStr + separate + _name + separate + Convert(m.msg.k.Msg());
            std::vector<int> sendMsgList = otherRsa.RSA_Encrypt(Convert(sendMsg));

            SendTo(sock, wDstPort, sendMsgList);
            while (true) {
                ret = Async(sock);
                while (std::future_status::ready != ret.wait_for(1s))
                {
                }
                nm = ret.get();


                if (Convert(m.msg.m.Msg()).compare(Convert(nm.msg.m.Msg())) == 0)
                    continue;

                std::string message = Convert(rsa.RSA_Decrypt(nm.msg.kName.Msg()));

                if (message.compare(payload) == 0)
                    break;
            }

            otherRSA = otherRsa;
            return strip(Convert(rsa.RSA_Decrypt(nm.msg.kName.Name())));

        }
    }
    return "";
}

void Client::Connect(WORD wSrcPort, WORD wDstPort)
{
    WSADATA wsaData = { 0 };
    SOCKET sock;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = MakeSocket(wSrcPort);

    if (sock) {
        std::string name;
        Message m;

        bool isFirst = true;

        do {
            do {
                if(!isFirst)
                    m = RecvMessage(sock);
                rsa.RSAInitializeByPrimary();
                name = Connect(sock, wDstPort);
                isFirst = false;
            } while (!CheckOtherUser(name));

            SendTo(sock, wDstPort, otherRSA.RSA_Encrypt(Convert( _name + separate + "hi")));
            m = RecvMessage(sock);
        } while (Convert(rsa.RSA_Decrypt(m.msg.n.Msg())).compare("hi") != 0);


        _otherUser = name;
        RestoreMessage();


        CreateOrReplaceFolder();
        
        RunClient(sock, wDstPort);
    }
    WSACleanup();
}

void Client::RunClient(SOCKET sock, WORD wDstPort) 
{
    RestoreMessage();

    std::thread readthread = std::thread(&Client::RecvThread, this, sock);

    readthread.detach();

    while (1)
    {
        if (!TrySendData(sock, wDstPort))
            break;
    }
    bEnd = TRUE;
    closesocket(sock);
}

void Client::RestoreMessage() {
    system("cls");
    _out << "                                               Chat with: " << _otherUser << "\n";
    auto res = Restore();
    for (auto& i : res) {
        _out << i << "\n";
    }
    _out << "Enter Message : ";
}

void Client::CreateOrReplaceFolder() {
    if (fs::exists(Folder)) {
        return;
    }
    fs::create_directory(Folder);
}

bool Client::CheckOtherUser(std::string name)
{
    if (_otherUser.empty())
        return true;
    else
        return name.compare(_otherUser) == 0;
}

std::string Client::GetCurrentFile() {
    return Folder + "\\" + _otherUser + ".txt";
}


std::vector<std::string> Client::Restore() {
    std::vector<std::string> res;
    std::string line;
    std::ifstream myfile(GetCurrentFile());
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
           res.push_back(line);
        }
        myfile.close();
    }
    return res;
}

void Client::Save(std::string message) {
    std::ofstream myfile;
    myfile.open(GetCurrentFile(), std::ios::app);
    myfile << message;
    myfile.close();
}
