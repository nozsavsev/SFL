#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>

template <class T> class VectorEx : public std::vector <T>
{
public:
    using std::vector<T>::vector;

    bool Contains(T val);
    void Rem_All(T val);
    void Rem_If(std::function <bool(T)> fnc);
    void Foreach(std::function <void(T&)> fnc);
    void Sort(std::function <bool(T, T)> fnc);

    bool operator==(VectorEx<T>& rhs);
    bool operator!=(VectorEx<T>& rhs);
};

class ms_hook_ll
{
private:
    ms_hook_ll() = default;

    static  ms_hook_ll* ist;
    std::mutex callbacks_mutex;
    VectorEx<std::function <bool(MSLLHOOKSTRUCT*, WPARAM)> >   callbacks;
    std::thread* hook_main_thread = nullptr;
    static void hook_main();
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    ~ms_hook_ll();
    static ms_hook_ll* getIST();
    ms_hook_ll* init();
    size_t Add_Callback(std::function <bool(MSLLHOOKSTRUCT*, WPARAM)> uc);
    void Remove_Callback(size_t id);
};

class kb_hook_ll
{
private:
    kb_hook_ll() = default;

    static  kb_hook_ll* ist;
    std::mutex callbacks_mutex;
    VectorEx<std::function <bool(KBDLLHOOKSTRUCT*, WPARAM)> >   callbacks;
    std::thread* hook_main_thread = nullptr;
    static void hook_main();
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    ~kb_hook_ll();
    static kb_hook_ll* getIST();
    kb_hook_ll* init();
    size_t Add_Callback(std::function <bool(KBDLLHOOKSTRUCT*, WPARAM)> uc);
    void Remove_Callback(size_t id);
};






template <class T>
bool VectorEx<T>::Contains(T val)
{
    for (size_t i = 0; i < this->size(); i++)
        if ((*this)[i] == val)
            return true;

    return false;
}
template <class T>
void VectorEx <T>::Rem_All(T val)
{
    this->erase(
        std::remove_if(this->begin(), this->end(), [&](T& item) -> bool { return (item == val); })
        , this->end());
}
template <class T>
void VectorEx <T>::Rem_If(std::function <bool(T)> fnc)
{
    std::remove_if(this->begin(), this->end(), fnc);
}
template <class T>
void VectorEx <T>::Foreach(std::function <void(T&)> fnc)
{
    std::for_each(this->begin(), this->end(), fnc);
}
template <class T>
void VectorEx <T>::Sort(std::function <bool(T, T)> fnc)
{
    std::sort(this->begin(), this->end(), fnc);
}
template <class T>
bool VectorEx <T>::operator==(VectorEx<T>& rhs)
{
    if (this->size() == rhs.size())
        return std::equal(this->begin(), this->end(), rhs.begin());

    return false;
}
template <class T>
bool VectorEx <T>::operator!=(VectorEx<T>& rhs)
{
    if (this->size() == rhs.size())
        return !std::equal(this->begin(), this->end(), rhs.begin());

    return true;
}