#pragma once
#include <winsock2.h>
#include <string>
#include <vector>
#include <iostream>
#include "RSA.h"
#include "Utils.h"

class Client
{
    RSA rsa;
    RSA otherRSA;

    const std::string Folder = "Client";

    inline static std::string separate = ":";

    inline static int KEY_LEN = 5;
    inline static int NAME_MAX_LEN = 10;
    inline static int SEPARATE = separate.size();

public:
    Client(std::string name, std::string otherClient, std::istream& in, std::ostream& out, std::ostream& err);

    void Connect(WORD wSrcPort, WORD wDstPort);
    
protected:

    std::string Connect(SOCKET sock, WORD wDstPort);

    void RunClient(SOCKET sock, WORD wDstPort);

    SOCKET MakeSocket(WORD wPort);

    void SetOtherUserName(std::string name);

private:

    class MessageText {
    private:

        struct Base {
            friend MessageText;
        protected:
            std::vector<int>& _value;

            void Change(std::vector<int>& v) {
                _value = v;
            }
            
        public:
            Base(std::vector<int>& value) : _value(value) { }
        };

        struct K : public Base {
            K(std::vector<int>& value) : Base(value) {}

            std::vector<int> KKey() {
                size_t e = KEY_LEN;
                return std::vector<int>(_value.begin(), _value.begin() + e);
            }

            std::vector<int> NKey() {
                size_t s = (KEY_LEN + SEPARATE);
                size_t e = (KEY_LEN * 2 + SEPARATE);
                return std::vector<int>(_value.begin() + s, _value.begin() + e);
            }

            std::vector<int> Msg() {
                size_t s = KEY_LEN * 2 + SEPARATE * 2;
                return std::vector<int>(_value.begin() + s, _value.end());
            }
        };

        struct KName : public K {
            KName(std::vector<int>& value) : K(value) {}

            std::vector<int> Name() {
                size_t s = KEY_LEN * 2 + SEPARATE * 2;
                size_t e = KEY_LEN * 2 + SEPARATE * 2 + NAME_MAX_LEN;
                return std::vector<int>(_value.begin() + s, _value.begin() + e);
            }

            std::vector<int> Msg() {
                size_t s = KEY_LEN * 2 + SEPARATE * 3 + NAME_MAX_LEN;
                return std::vector<int>(_value.begin() + s, _value.end());
            }
        };

        struct M : public Base {
            M(std::vector<int>& value) : Base(value) {}

            std::vector<int> Msg() {
                return std::vector<int>(_value.begin(), _value.end());
            }
        };


        struct N : public Base {
            N(std::vector<int>& value) : Base(value) {}

            std::vector<int> Name() {
                size_t e = NAME_MAX_LEN;
                return std::vector<int>(_value.begin(), _value.begin() + e);
            }

            std::vector<int> Msg() {
                size_t s = NAME_MAX_LEN + SEPARATE;
                return std::vector<int>(_value.begin() + s, _value.end());
            }
        };

    protected:

        void Change() {
            k.Change(_value);
            m.Change(_value);
            kName.Change(_value);
            n.Change(_value);
        }

    public:

        std::vector<int> _value;
        K k = K(_value);
        KName kName = KName(_value);
        M m = M(_value);
        N n = N(_value);

        MessageText() : _value(std::vector<int>()) { }
        MessageText(std::vector<int> value) : _value(value) { }
        MessageText(MessageText& mesText) {
            _value = mesText._value;
            Change();
        }
        MessageText(MessageText&& mesText) noexcept {
            _value = std::move(mesText._value);
            Change();
        }

        MessageText& operator=(const MessageText& mesText) {
            _value = mesText._value;
            Change();
            return *this;
        }

    };
    


    struct Message {
        std::string ip;
        u_short port;
        MessageText msg;
    };

    std::string _name;
    std::istream& _in;
    std::ostream& _out;
    std::ostream& _err;
    bool bEnd = false;

    std::string _otherUser;

    BOOL TrySendData(SOCKET sock, WORD wDstPort);

    void RecvThread(SOCKET sock);

    void RestoreMessage();
    std::vector<std::string> Restore();

    void Save(std::string message);

    void CreateOrReplaceFolder();

    std::string GetCurrentFile();

    bool CheckOtherUser(std::string name);

    Message RecvMessage(SOCKET sock);
    void SendTo(SOCKET sock, WORD wDstPort, std::vector<int> str);

    auto Async(SOCKET sock);

};

