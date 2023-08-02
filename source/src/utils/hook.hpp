// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

// This header is designed to be used in Windows®.

#pragma once

#include <chrono>
#include <functional>
#include <semaphore>
#include <thread>
#include <unordered_map>

#include <Windows.h>
#undef min
#undef max

class _hook_base {
private:
    std::binary_semaphore bs{0};
    std::binary_semaphore bs_descendent{0};
    std::thread hook_thread;
    void thread_proc() {
        MSG msg;
        // Ask the system to create a message queue.
        PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE);
        bs.release();
        bs_descendent.acquire();
        _do_before_message_loop();
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

protected:
    /// <summary>
    /// Get id of the thread the hook runs on. The thread is guaranteed to be created after the base class is
    /// constructed.
    /// </summary>
    /// <returns></returns>
    DWORD get_thread_id() const {
        auto id = hook_thread.get_id();
        return *reinterpret_cast<const DWORD*>(&id);
    }

private:
    /// <summary>
    /// This function will run on an individual thread.
    /// </summary>
    virtual void _do_before_message_loop() = 0;

protected:
    /// <summary>
    /// You must call this function after critical variables of the descendent class are constructed.
    /// After calling it, _do_before_message_loop will be called on an individual thread.
    /// </summary>
    void _constructed() {
        bs_descendent.release();
    }

public:
    _hook_base() : hook_thread(&_hook_base::thread_proc, this) {
        bs.acquire(); // Wait until the message queue is created.
    }
    ~_hook_base() {
        PostThreadMessageW(get_thread_id(), WM_QUIT, 0, 0);
        hook_thread.join();
    }
    _hook_base(const _hook_base&) = delete;
    _hook_base(_hook_base&&) = delete;
    _hook_base& operator=(const _hook_base&) = delete;
    _hook_base& operator=(_hook_base&&) = delete;
};

/// <summary>
/// The keyboard hook. Call set_callback to register a callback (see keyboard_hook::callback_t). The callback function
/// will run on an individual thread.
/// </summary>
class keyboard_hook final : public _hook_base {
private:
    inline static std::binary_semaphore bs_map{1};
    inline static std::unordered_map<DWORD, keyboard_hook*> id2instance;
    static LRESULT CALLBACK virtual_proc(int nCode, WPARAM wParam, LPARAM lParam) {
        auto now = std::chrono::steady_clock::now();

        static thread_local keyboard_hook* instance;
        if (!instance) {
            bs_map.acquire();
            instance = id2instance[GetCurrentThreadId()];
            bs_map.release();
        }

        if (nCode < 0)
            return CallNextHookEx(instance->hKeybd, nCode, wParam, lParam);
        if (nCode == HC_ACTION) {
            auto ks = PKBDLLHOOKSTRUCT(lParam);
            bool skip = false;
            instance->bs_callback.acquire();
            if (instance->callback) {
                skip = instance->callback(wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN, ks->vkCode, now);
            }
            instance->bs_callback.release();

            if (skip)
                return 1;
        }
        return CallNextHookEx(instance->hKeybd, nCode, wParam, lParam);
    }

private:
    virtual void _do_before_message_loop() override {
        hKeybd = SetWindowsHookExW(WH_KEYBOARD_LL, virtual_proc, nullptr, NULL);
    }

private:
    HHOOK hKeybd{};

public:
    keyboard_hook() {
        bs_map.acquire();
        id2instance[get_thread_id()] = this;
        bs_map.release();
        _constructed();
    }
    ~keyboard_hook() {
        UnhookWindowsHookEx(hKeybd);
        bs_map.acquire();
        id2instance.erase(get_thread_id());
        bs_map.release();
    }

public:
    /// <summary>
    /// Returning true means to skip the key message.
    /// </summary>
    using callback_t = std::function<bool(bool down, int vk, std::chrono::steady_clock::time_point time)>;

private:
    std::binary_semaphore bs_callback{1};
    callback_t callback;

public:
    void set_callback(callback_t func) {
        bs_callback.acquire();
        callback = func;
        bs_callback.release();
    }
    void reset_callback() {
        bs_callback.acquire();
        callback = callback_t();
        bs_callback.release();
    }
};

/// <summary>
/// The mouse hook. Call set_callback to register a callback (see mouse_hook::callback_t). The callback function will
/// run on an individual thread.
/// </summary>
class mouse_hook final : public _hook_base {
private:
    inline static std::binary_semaphore bs_map{1};
    inline static std::unordered_map<DWORD, mouse_hook*> id2instance;
    static LRESULT CALLBACK virtual_proc(int nCode, WPARAM wParam, LPARAM lParam) {
        auto now = std::chrono::steady_clock::now();

        static thread_local mouse_hook* instance;
        if (!instance) {
            bs_map.acquire();
            instance = id2instance[GetCurrentThreadId()];
            bs_map.release();
        }

        if (nCode < 0)
            return CallNextHookEx(instance->hMouse, nCode, wParam, lParam);
        if (nCode == HC_ACTION) {
            auto ms = PMSLLHOOKSTRUCT(lParam);
            bool skip = false;
            instance->bs_callback.acquire();
            if (instance->callback) {
                skip = instance->callback(wParam, ms, now);
            }
            instance->bs_callback.release();

            if (skip)
                return 1;
        }
        return CallNextHookEx(instance->hMouse, nCode, wParam, lParam);
    }

private:
    virtual void _do_before_message_loop() override {
        hMouse = SetWindowsHookExW(WH_MOUSE_LL, virtual_proc, nullptr, NULL);
    }

private:
    HHOOK hMouse{};

public:
    mouse_hook() {
        bs_map.acquire();
        id2instance[get_thread_id()] = this;
        bs_map.release();
        _constructed();
    }
    ~mouse_hook() {
        UnhookWindowsHookEx(hMouse);
        bs_map.acquire();
        id2instance.erase(get_thread_id());
        bs_map.release();
    }

public:
    /// <summary>
    /// Returning true means to skip the key message.
    /// </summary>
    using callback_t =
        std::function<bool(UINT message, PMSLLHOOKSTRUCT ms, std::chrono::steady_clock::time_point time)>;

private:
    std::binary_semaphore bs_callback{1};
    callback_t callback;

public:
    void set_callback(callback_t func) {
        bs_callback.acquire();
        callback = func;
        bs_callback.release();
    }
    void reset_callback() {
        bs_callback.acquire();
        callback = callback_t();
        bs_callback.release();
    }
};
