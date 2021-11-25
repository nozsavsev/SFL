#include "hooks.h"

void ms_hook_ll::hook_main()
{
    HHOOK hook_handle = SetWindowsHookExW(WH_MOUSE_LL, ms_hook_ll::LowLevelMouseProc, NULL, NULL);
    MSG msg;

    printf("ms -> OK!\n");

    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK ms_hook_ll::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    bool block_input = false;

    if (nCode == HC_ACTION)
    {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;
        ist->callbacks_mutex.lock();
        ist->callbacks.Foreach([&](auto c) -> void { block_input |= c(ms, wParam); });
        ist->callbacks_mutex.unlock();
    }

    if (block_input) return 1;
    else             return CallNextHookEx(NULL, nCode, wParam, lParam);
}

ms_hook_ll::~ms_hook_ll()
{
    if (hook_main_thread)
    {
        delete hook_main_thread;
        hook_main_thread = nullptr;
    }      
}

ms_hook_ll* ms_hook_ll::getIST()
{
    return ist;
}

ms_hook_ll* ms_hook_ll::init()
{
    if (!hook_main_thread)
        hook_main_thread = new std::thread(&hook_main);
    return ist;
}

size_t ms_hook_ll::Add_Callback(std::function <bool(MSLLHOOKSTRUCT*, WPARAM)> uc)
{
    callbacks_mutex.lock();
    callbacks.push_back(uc);
    auto id = callbacks.size() - 1;
    callbacks_mutex.unlock();
    return id;
}

void ms_hook_ll::Remove_Callback(size_t id)
{
    callbacks_mutex.lock();
    callbacks.erase(callbacks.begin() + id);
    callbacks_mutex.unlock();

}

ms_hook_ll* ms_hook_ll::ist = new ms_hook_ll();






void kb_hook_ll::hook_main()
{

    HHOOK hook_handle = SetWindowsHookExW(WH_KEYBOARD_LL, kb_hook_ll::LowLevelKeyboardProc, NULL, NULL);
    MSG msg;

    printf("kb -> OK!\n");

    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK kb_hook_ll::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    bool block_input = false;

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        ist->callbacks_mutex.lock();
        ist->callbacks.Foreach([&](auto c) -> void { block_input |= c(kb, wParam); });
        ist->callbacks_mutex.unlock();
    }

    if (block_input) return 1;
    else             return CallNextHookEx(NULL, nCode, wParam, lParam);
}

kb_hook_ll* kb_hook_ll::getIST()
{
    return ist;
}

kb_hook_ll::~kb_hook_ll()
{


    if (hook_main_thread)
    {
        delete hook_main_thread;
        hook_main_thread = nullptr;
    }
}

kb_hook_ll* kb_hook_ll::init()
{
    if (!hook_main_thread)
        hook_main_thread = new std::thread(&hook_main);
    return ist;

}

size_t kb_hook_ll::Add_Callback(std::function <bool(KBDLLHOOKSTRUCT*, WPARAM)> uc)
{
    callbacks_mutex.lock();
    callbacks.push_back(uc);
    auto id = callbacks.size() - 1;
    callbacks_mutex.unlock();
    return id;
}

void kb_hook_ll::Remove_Callback(size_t id)
{
    callbacks_mutex.lock();
    callbacks.erase(callbacks.begin() + id);
    callbacks_mutex.unlock();
}

kb_hook_ll* kb_hook_ll::ist = new kb_hook_ll();