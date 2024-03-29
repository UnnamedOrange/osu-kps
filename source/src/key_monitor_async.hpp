// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <chrono>
#include <concepts>
#include <semaphore>
#include <thread>
#include <unordered_map>
using namespace std::literals;

#include <Windows.h>
#undef min
#undef max

#include "key_monitor_base.hpp"

namespace kps {
    /// <summary>
    /// 使用 GetAsyncKeyState 实现的按键监视器。
    /// </summary>
    class key_monitor_async final : public key_monitor_base {
    private:
        using key_monitor_base::_on_llkey_down;
        using key_monitor_base::_on_llkey_up;

    public:
        /// <summary>
        /// most significant bit.
        /// </summary>
        /// <param name="v">number.</param>
        /// <returns>the most significant bit of the number.</returns>
        template <std::integral int_t>
        static bool msb(int_t v) {
            return v & (1 << (sizeof(int_t) * 8 - 1));
        }

    private:
        std::binary_semaphore s_exit{0};
        std::thread t{&key_monitor_async::thread_proc, this};
        void thread_proc() {
            while (true) {
                if (s_exit.try_acquire_for(10ms))
                    break;
                for (int i = 1; i < 256; i++)
                    if (msb(GetAsyncKeyState(i)))
                        _on_llkey_down(i, clock::now());
                    else
                        _on_llkey_up(i, clock::now());
            }
        }

    public:
        ~key_monitor_async() {
            s_exit.release();
            t.join();
        }
    };
} // namespace kps
