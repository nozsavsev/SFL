#include "hooks.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <fstream> 

using namespace std;
using namespace std::chrono;

class mouse_record
{
public:
    mouse_record() = default;
    mouse_record(MSLLHOOKSTRUCT* ms, WPARAM evt)
    {
        x = ms->pt.x;
        y = ms->pt.y;
        event = evt;
        delta = ms->mouseData;
    }

    int x = 0;
    int y = 0;

    WPARAM event = 0;
    int delta = 0;

    high_resolution_clock::time_point time = high_resolution_clock::now();
    size_t relative_delay = 0;
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

    int keyCode = 0;
    WPARAM event = 0;

    high_resolution_clock::time_point time = high_resolution_clock::now();
    size_t relative_delay = 0;
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

int main()
{

    VectorEx<mouse_record> msrec;
    VectorEx<keybd_record> kbrec;


    std::atomic<bool> iHaveToWork = true;
                                                                          
    //setup hooks
    auto keybd_hook = kb_hook_ll::getIST()->init();
    auto mouse_hook = ms_hook_ll::getIST()->init();

    //add callbacks
    auto  kbhookID = keybd_hook->Add_Callback(
        [&](KBDLLHOOKSTRUCT* kb, WPARAM evt) -> bool
        {
            if (kb->vkCode == VK_INSERT)
                return !(iHaveToWork = false);

            kbrec.push_back({ kb, evt });

            return false;
        });


    //add callbacks
    auto  mshookID = mouse_hook->Add_Callback(
        [&](MSLLHOOKSTRUCT* ms, WPARAM evt) -> bool
        {
            msrec.push_back({ ms, evt });
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
        msrec.Foreach(
            [&](mouse_record& s) -> void
            {
                static auto last = msrec[0].time;
                s.relative_delay = duration_cast<microseconds>(s.time - last).count();
                last = s.time;
            }
        );

        kbrec.Foreach(
            [&](keybd_record& s) -> void
            {
                static auto last = msrec[0].time;
                s.relative_delay = duration_cast<microseconds>(s.time - last).count();
                last = s.time;
            }
        );
    }

    //serialize and dump to file
    {
        nlohmann::json out_json;
        out_json["ms_actions"] = msrec;
        out_json["kb_actions"] = kbrec;
        fstream out_file;
        out_file.open("rec.sfl", fstream::out);
        auto fstr = out_json.dump();
        out_file.write(fstr.c_str(), fstr.size());
        out_file.close();
    }

    Sleep(100);
    return 0;
}