# osu!kps

KPS (keys per second) indicator for osu! in C++.

This project is under regard. \_(:3」∠)\_

## Motivation

It's been a long time since I wrote a complete project. osu!kps could be a best chance for me to develop a new framework in C++.

## Environment

Target platform: Windows®.

IDE: Visual Studio® 2019.

Language: C++20.

## Roadmap

- [x] tool header for code page conversion, especially on Windows®.

- [x] tool header for window class on Windows®.

- [x] class of KPS calculator, independent of operating system.

  Key methods:

  ```cpp
  void clear();
  void notify_key_down(int key, time_point time); // key should be defined in this header, and should be compatible with those in Windows. Whether this method should be PostMessage-like is under consideration.
  int calc_kps_now(int key) const;
  int calc_kps_now(const std::vector<int>& keys) const; // Sum up the kps. This should be quick.
  ```
  
  The constructor and `operator=` should also be considered.
  
- [x] class of key monitor, on Windows®.

  There are two options:

  1. Use `GetAsyncKeyState`.
  2. Use `SetWindowsHookEx`.

  It's RAII style.

- [x] class of wrapped KPS calculator, using key monitor to call notify_key_down. It's a descendant class of KPS calculator.

  It's RAII style. By the mean time, it should implement a method that notifies an external function when a key is pressed.

- [x] menu to choose the number of keys.

  However, it's after implementing the config system that the keys can be modified.

- [ ] use Direct2D to draw a UI.

  After this step the project can be pre-released.

- [ ] tool header for config system.

  I may use json.

- [ ] add more statistic information.

- [ ] tool header for multi-language support.

  I may use json.

  After this step the project can be released.

- [ ] implement functions specific to osu!.

- [ ] tool header for DirectUI.

## Details

### The method to calculate KPS

I just save all the pressed keys, and based on it, calculate KPS. The KPS equals to the total times you press in recent 1 second.  

## License

Copyright (c) UnnamedOrange. Licensed under the MIT Licence.

See the LICENCE file in the repository root for full licence text.

## Credits