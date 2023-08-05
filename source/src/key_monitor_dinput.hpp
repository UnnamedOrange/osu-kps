/**
 * @file key_monitor_dinput.hpp
 * @author UnnamedOrange
 * @brief Use DirectInput to implement @ref key_monitor_base.
 * @version 1.0.0
 * @date 2023-08-05
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#pragma once

#include <chrono>
#include <thread>
using namespace std::literals;

#include <Windows.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "key_monitor_base.hpp"
#include "utils/d2d/SharedComPtr.hpp"

namespace kps {
    class key_monitor_dinput final : public key_monitor_base {
    private:
        using key_monitor_base::_on_llkey_down;
        using key_monitor_base::_on_llkey_up;

    private:
        void polling_thread_routine(std::stop_token st) {
            while (!st.stop_requested()) {
                auto now = std::chrono::steady_clock::now();

                // Poll keyboard.
                {
                    BYTE keyboard_state[256];
                    if (FAILED(keyboard_device->GetDeviceState(sizeof(keyboard_state), keyboard_state))) {
                        keyboard_device->Acquire();
                        continue;
                    }
                    for (int i = 0; i < 256; ++i) {
                        auto code = MapVirtualKeyW(i, MAPVK_VSC_TO_VK_EX);
                        if (!code) {
                            continue;
                        }
                        if (keyboard_state[i] & 0x80) {
                            _on_llkey_down(code, now);
                        } else {
                            _on_llkey_up(code, now);
                        }
                    }
                }

                // Poll mouse.
                {
                    DIMOUSESTATE mouse_state;
                    if (FAILED(mouse_device->GetDeviceState(sizeof(mouse_state), &mouse_state))) {
                        mouse_device->Acquire();
                        continue;
                    }
                    if (mouse_state.rgbButtons[0] & 0x80) {
                        _on_llkey_down(VK_LBUTTON, now);
                    } else {
                        _on_llkey_up(VK_LBUTTON, now);
                    }
                    if (mouse_state.rgbButtons[1] & 0x80) {
                        _on_llkey_down(VK_RBUTTON, now);
                    } else {
                        _on_llkey_up(VK_RBUTTON, now);
                    }
                    if (mouse_state.rgbButtons[2] & 0x80) {
                        _on_llkey_down(VK_MBUTTON, now);
                    } else {
                        _on_llkey_up(VK_MBUTTON, now);
                    }
                }

                std::this_thread::sleep_for(10ms);
            }
        }

    private:
        orange::SharedComPtr<IDirectInput8W> dinput;
        orange::SharedComPtr<IDirectInputDevice8W> keyboard_device, mouse_device;
        std::jthread polling_thread;

    public:
        key_monitor_dinput() {
            // Initialize DirectInput.
            if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8W,
                                          reinterpret_cast<void**>(dinput.reset_and_get_address()), nullptr))) {
                throw std::runtime_error("Failed to initialize DirectInput.");
            }

            // Initialize keyboard.
            if (FAILED(dinput->CreateDevice(GUID_SysKeyboard, keyboard_device.reset_and_get_address(), nullptr))) {
                throw std::runtime_error("Failed to initialize DirectInput keyboard.");
            }
            if (FAILED(keyboard_device->SetDataFormat(&c_dfDIKeyboard))) {
                throw std::runtime_error("Failed to set DirectInput keyboard data format.");
            }
            if (FAILED(keyboard_device->SetCooperativeLevel(nullptr, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                throw std::runtime_error("Failed to set DirectInput keyboard cooperative level.");
            }
            if (FAILED(keyboard_device->Acquire())) {
                throw std::runtime_error("Failed to acquire DirectInput keyboard.");
            }

            // Initialize mouse.
            if (FAILED(dinput->CreateDevice(GUID_SysMouse, mouse_device.reset_and_get_address(), nullptr))) {
                throw std::runtime_error("Failed to initialize DirectInput mouse.");
            }
            if (FAILED(mouse_device->SetDataFormat(&c_dfDIMouse))) {
                throw std::runtime_error("Failed to set DirectInput mouse data format.");
            }
            if (FAILED(mouse_device->SetCooperativeLevel(nullptr, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                throw std::runtime_error("Failed to set DirectInput mouse cooperative level.");
            }
            if (FAILED(mouse_device->Acquire())) {
                throw std::runtime_error("Failed to acquire DirectInput mouse.");
            }

            polling_thread = std::jthread(std::bind_front(&key_monitor_dinput::polling_thread_routine, this));
        }
        ~key_monitor_dinput() {
            if (polling_thread.joinable()) {
                polling_thread.request_stop();
                polling_thread.join();
            }

            if (mouse_device) {
                mouse_device->Unacquire();
            }
            if (keyboard_device) {
                keyboard_device->Unacquire();
            }
        }
    }; // namespace kps
} // namespace kps
