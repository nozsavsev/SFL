#include "hooks.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>

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

    string Event()
    {
        switch (event)
        {
        case   WM_LBUTTONDOWN: return "LBUTTONDOWN"; break;
        case   WM_LBUTTONUP:   return "LBUTTONUP";   break;
        case   WM_MOUSEMOVE:   return "MOUSEMOVE";   break;
        case   WM_MOUSEWHEEL:  return "MOUSEWHEEL";  break;
        case   WM_MOUSEHWHEEL: return "MOUSEHWHEEL"; break;
        case   WM_RBUTTONDOWN: return "RBUTTONDOWN"; break;
        case   WM_RBUTTONUP:   return "RBUTTONUP";   break;
        default:               return  "";           break;
        }
    }

    void Event(string str)
    {
        if (str == "LBUTTONDOWN") event = WM_LBUTTONDOWN;
        if (str == "LBUTTONUP") event = WM_LBUTTONUP;
        if (str == "MOUSEMOVE") event = WM_MOUSEMOVE;
        if (str == "MOUSEWHEEL") event = WM_MOUSEWHEEL;
        if (str == "MOUSEHWHEEL") event = WM_MOUSEHWHEEL;
        if (str == "RBUTTONDOWN") event = WM_RBUTTONDOWN;
        if (str == "RBUTTONUP") event = WM_RBUTTONUP;
    }

    string toString()
    {
        ostringstream stringStream;
        stringStream << x << ":" << y << " " << Event();

        return stringStream.str();
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
        kb.

    }

    int keyCode = 0;

    string Event()
    {


    }

    void Event(string str)
    {


    }

    string toString()
    {


    }



    high_resolution_clock::time_point time = high_resolution_clock::now();
    size_t relative_delay = 0;
};
void to_json(nlohmann::json& j, const keybd_record& p)
{


}

void from_json(const nlohmann::json& j, keybd_record& p)
{


}











int main()
{

    VectorEx<mouse_record> msrec;
    VectorEx<keybd_record> kbrec;


    std::atomic<bool> iHaveToWork = true;

    auto keyboard_hook = kb_hook_ll::getIST()->init();
    auto mouse_hook = ms_hook_ll::getIST()->init();

    auto  kbhookID = keyboard_hook->Add_Callback([&](KBDLLHOOKSTRUCT* kb, WPARAM evt) -> bool {

        if (kb->vkCode == VK_INSERT)
            return !(iHaveToWork = false);




        return !iHaveToWork;
        });

    auto  mshookID = mouse_hook->Add_Callback([&](MSLLHOOKSTRUCT* ms, WPARAM evt) -> bool {
        msrec.push_back({ ms, evt });
        return false;
        });


    while (iHaveToWork);

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
    }

    nlohmann::json out_json;

    out_json["ms_actions"] = msrec;
    out_json["kb_actions"] = kbrec;


    //std::this_thread::sleep_for(microseconds(s.relative_delay));


    Sleep(100);
    return 0;
}