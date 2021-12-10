#define _CRT_SECURE_NO_WARNINGS
#include "hooks.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <fstream> 
#include <future> 


using namespace std;
using namespace std::chrono;

class mouse_record
{
public:
    mouse_record() = default;
    mouse_record(MSLLHOOKSTRUCT* ms, WPARAM evt)
    {
        x = ms->pt.x + 1;
        y = ms->pt.y + 1;
        event = evt;


        delta = (short)HIWORD(ms->mouseData);
    }

    DWORD translateEvent()
    {
        if (event == WM_MOUSEMOVE) return MOUSEEVENTF_MOVE;
        else if (event == WM_LBUTTONDOWN) return MOUSEEVENTF_LEFTDOWN;
        else if (event == WM_LBUTTONUP) return MOUSEEVENTF_LEFTUP;
        else if (event == WM_RBUTTONDOWN) return MOUSEEVENTF_RIGHTDOWN;
        else if (event == WM_RBUTTONUP) return MOUSEEVENTF_RIGHTUP;
        else if (event == WM_MOUSEWHEEL) return MOUSEEVENTF_WHEEL;
        else if (event == WM_MOUSEHWHEEL) return MOUSEEVENTF_HWHEEL;
        return 0;
    }

    void replicate()
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();

        INPUT input;
        memset(&input, 0, sizeof(input));
        input.type = INPUT_MOUSE;
        input.mi.mouseData = delta;
        input.mi.dx = (65535 * (x + 0)) / GetSystemMetrics(SM_CXSCREEN);
        input.mi.dy = (65535 * (y + 0)) / GetSystemMetrics(SM_CYSCREEN);

        input.mi.dwFlags = translateEvent() | MOUSEEVENTF_ABSOLUTE;

        auto delay = microseconds(relative_delay) - (high_resolution_clock::now() - start);

        if (delay.count() > 0)
            std::this_thread::sleep_for(delay);

        SendInput(1, &input, sizeof(input));
    }

    int x = 0;
    int y = 0;

    WPARAM event = 0;
    int delta = 0;

    size_t relative_delay = 0;
    steady_clock::time_point time = high_resolution_clock::now();

};
void to_json(nlohmann::json& j, const mouse_record& p)
{
    j["relative_delay"] = p.relative_delay;
    j["delta"] = p.delta;
    j["event"] = p.event;
    j["x"] = p.x;
    j["y"] = p.y;
}
void from_json(const nlohmann::json& j, mouse_record& p)
{
    p.relative_delay = j["relative_delay"];
    p.delta = j["delta"];
    p.event = j["event"];
    p.x = j["x"];
    p.y = j["y"];
}

class keybd_record
{
public:
    keybd_record() = default;
    keybd_record(KBDLLHOOKSTRUCT* kb, WPARAM evt)
    {
        keyCode = kb->vkCode;
        event = evt;
    }

    void replicate()
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();

        INPUT ip;
        memset(&ip, 0, sizeof(ip));


        ip.type = INPUT_KEYBOARD;
        ip.ki.dwExtraInfo = 0;
        ip.ki.wVk = keyCode;
        ip.ki.dwFlags = 0;

        if (event == WM_KEYUP || event == WM_SYSKEYUP)
        {
            ip.ki.dwFlags = KEYEVENTF_KEYUP;
            printf("keyup\n\n");
        }
        else
            printf("key\n");

        auto delay = microseconds(relative_delay) - (high_resolution_clock::now() - start);
        if (delay.count() > 0)
            std::this_thread::sleep_for(delay);

        SendInput(1, &ip, sizeof(INPUT));

    }

    int keyCode = 0;
    WPARAM event = 0;

    size_t relative_delay = 0;
    steady_clock::time_point time = high_resolution_clock::now();
};
void to_json(nlohmann::json& j, const keybd_record& p)
{
    j["keyCode"] = p.keyCode;
    j["event"] = p.event;
    j["relative_delay"] = p.relative_delay;
}
void from_json(const nlohmann::json& j, keybd_record& p)
{
    p.keyCode = j["keyCode"];
    p.event = j["event"];
    p.relative_delay = j["relative_delay"];
}

class hid_record
{
public:
    enum class type_e { UNDEFINED = 0, KEYBD = 1, MOUSE = 2 };

    hid_record() = default;
    hid_record(mouse_record msi)
    {
        kb = keybd_record();
        ms = msi;
        type = type_e::MOUSE;
    }

    hid_record(keybd_record kbi)
    {
        ms = mouse_record();
        kb = kbi;
        type = type_e::KEYBD;
    }

    operator mouse_record() { return ms; }
    operator keybd_record() { return kb; }

    type_e type = type_e::UNDEFINED;


    mouse_record ms;
    keybd_record kb;

};
void to_json(nlohmann::json& j, const hid_record& p)
{
    if (p.type == hid_record::type_e::MOUSE)
        j = p.ms;
    else if (p.type == hid_record::type_e::KEYBD)
        j = p.kb;

    j["type"] = p.type;
}
void from_json(const nlohmann::json& j, hid_record& p)
{
    p.type = j["type"];

    if (p.type == hid_record::type_e::MOUSE)
        p.ms = j;
    else if (p.type == hid_record::type_e::KEYBD)
        p.kb = j;

}


size_t getFileSize(const char* fname)
{
    FILE* f;
    f = fopen(fname, "r");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}

int main()
{
    int replicate = 0;

    auto keybd_hook = kb_hook_ll::getIST()->init();
    auto mouse_hook = ms_hook_ll::getIST()->init();

    auto starter_id = keybd_hook->Add_Callback([&](KBDLLHOOKSTRUCT* kb, WPARAM wp) -> bool {

        if (kb->vkCode == 'S')
        {
            replicate = 2;
            return true;
        }
        else if (kb->vkCode == 'R')
        {
            replicate = 1;
            return true;
        }
        else if (kb->vkCode == 'N')
        {
            replicate = 228;
            return true;
        }

        return false;
        });

    printf("press S to start recording to file\npress R to replicate from file\nto stop recording or replication press Escape\nto enter N.I.K.I.T.A. mode press n\n");

    while (!replicate) Sleep(10);

    keybd_hook->Remove_Callback(starter_id);

    if (replicate == 228)
    {
        keybd_hook->Add_Callback(
            [&](KBDLLHOOKSTRUCT* kb, WPARAM evt) -> bool
            {
                if (kb->vkCode == VK_ESCAPE)
                    exit(0);

                if (((kb->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kb->flags & LLKHF_INJECTED) == LLKHF_INJECTED))
                    return false;

                return true;
            });


        while (1)
        {
            POINT p;
            GetCursorPos(&p);

            INPUT input;
            memset(&input, 0, sizeof(input));
            input.type = INPUT_MOUSE;
            input.mi.dx = (65535 * (p.x + 0)) / GetSystemMetrics(SM_CXSCREEN);
            input.mi.dy = (65535 * (p.y + 0)) / GetSystemMetrics(SM_CYSCREEN);

            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE;
            SendInput(1, &input, sizeof(input));
            std::this_thread::sleep_for(microseconds(100));
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE;
            SendInput(1, &input, sizeof(input));
        }

    }
    else if (replicate == 1)
    {
        VectorEx<hid_record> hrec;
        fstream in_file;

        auto fsize = getFileSize("rec.sfl");
        in_file.open("rec.sfl", fstream::in);
        char* _buffer = (char*)malloc(fsize);
        in_file.read(_buffer, fsize);
        auto j = nlohmann::json::parse(_buffer);
        hrec = j["actions"];
        free(_buffer);

        printf("replicationg\n");

        keybd_hook->Add_Callback(
            [&](KBDLLHOOKSTRUCT* kb, WPARAM evt) -> bool
            {
                if (kb->vkCode == VK_ESCAPE)
                    exit(0);

                if (((kb->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kb->flags & LLKHF_INJECTED) == LLKHF_INJECTED))
                    return false;

                return true;
            });

        mouse_hook->Add_Callback([&](MSLLHOOKSTRUCT* ms, WPARAM evt) -> bool
            {
                if (((ms->flags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED) || ((ms->flags & LLMHF_INJECTED) == LLMHF_INJECTED))
                    return false;

                return true;
            });

        hrec.Foreach([&](hid_record& s) -> void
            {
                if (s.type == hid_record::type_e::KEYBD)
                    s.kb.replicate();
                else if (s.type == hid_record::type_e::MOUSE)
                    s.ms.replicate();
            }
        );

    }
    else if (replicate == 2)
    {

        printf("recording\n");

        VectorEx<hid_record> hrec;
        std::mutex hrec_mtx;

        std::atomic<bool> iHaveToWork = true;

        //add callbacks
        auto  kbhookID = keybd_hook->Add_Callback(
            [&](KBDLLHOOKSTRUCT* kb, WPARAM evt) -> bool
            {
                if (kb->vkCode == VK_ESCAPE)
                    return !(iHaveToWork = false);

                hrec_mtx.lock();
                hrec.push_back(keybd_record(kb, evt));
                hrec_mtx.unlock();

                return false;
            });


        //add callbacks
        auto  mshookID = mouse_hook->Add_Callback(
            [&](MSLLHOOKSTRUCT* ms, WPARAM evt) -> bool
            {
                hrec_mtx.lock();
                hrec.push_back(mouse_record(ms, evt));
                hrec_mtx.unlock();
                return false;
            });

        while (iHaveToWork)
            Sleep(1);

        //clear callbacks();
        {
            keybd_hook->Remove_Callback(kbhookID);
            mouse_hook->Remove_Callback(mshookID);
        }

        //fill relative delays
        {

            steady_clock::time_point last = high_resolution_clock::now();

            printf("%llu\n", hrec.size());

            if (hrec[0].type == hid_record::type_e::KEYBD)
                last = ((keybd_record)(hrec[0])).time;
            else if (hrec[0].type == hid_record::type_e::MOUSE)
                last = ((mouse_record)(hrec[0])).time;

            hrec.Foreach(
                [&](hid_record& s) -> void
                {
                    if (s.type == hid_record::type_e::KEYBD)
                    {
                        (s.kb).relative_delay = duration_cast<microseconds>(s.kb.time - last).count();
                        last = s.kb.time;
                    }
                    else if (s.type == hid_record::type_e::MOUSE)
                    {
                        (s.ms).relative_delay = duration_cast<microseconds>(s.ms.time - last).count();
                        last = s.ms.time;
                    }
                }
            );
        }

        //serialize and dump to file

        {
            nlohmann::json out_json;

            out_json["actions"] = hrec;

            fstream out_file;
            out_file.open("rec.sfl", fstream::out);
            auto fstr = out_json.dump();
            out_file.write(fstr.c_str(), fstr.size());
            out_file.close();
        }

        Sleep(100);

    }

    return 0;
}