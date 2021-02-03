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

- [ ] tool header for code page conversion, especially on Windows®.

- [ ] tool header for window class on Windows®.

- [ ] class of KPS calculator, independent of operating system.

  Should consider the cases that the monitor program runs on the main thread and that on the child thread.

  Should consider the max cache size of history key presses.

  Key methods:

  ```cpp
  void notify_key_down(int key, std::chrono::time_point time); // key should be defined in this header, and should be compatible with those in Windows. Whether this method should be PostMessage-like is under consideration.
  double calc_kps_now(int key);
  double calc_kps_now(const std::vector<int>& keys); // Sum up the kps. This should be quick.
  std::vector<double> calc_kps_all(int key);
  std::vector<double> calc_kps_all(const std::vector<int>& keys); // This is allowed to use much CPU time.
  void clear();
  ```

  The constructor and `operator=` should also be considered.

- [ ] class of key monitor, on Windows®.

  There are two options:

  1. Use `GetAsyncKeyState`.
  2. Use `SetWindowsHookEx`.

- [ ] class of wrapped KPS calculator, using key monitor to call notify_key_down. It's a descendant class of KPS calculator.

  If option 1 is chosen, a thread lives the same long as this class. If option 2 is chosen, a hook lives the same long as this class. Namely it's RAII style.

- [ ] use Direct2D to draw a UI.

  After this step the project can be pre-released.

- [ ] tool header for config system.

  I may use json.

- [ ] tool header for multi-language support.

  I may use json.

  After this step the project can be released.

- [ ] implement functions specific to osu!.

- [ ] tool header for DirectUI.

## License

Copyright (c) UnnamedOrange. Licensed under the MIT Licence.

See the LICENCE file in the repository root for full licence text.

## Credits