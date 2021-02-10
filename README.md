# osu!kps

KPS (keys per second) indicator for osu! in C++.

This project is under development. \_(:3」∠)\_

## Motivation

It's been a long time since I wrote a complete project. osu!kps could be a best chance for me to develop a new framework in C++.

## Environment

Target platform: Windows® 10.

IDE: Visual Studio® 2019.

Language: C++20.

Graphics: Direct2D.

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

- [x] use Direct2D to draw a UI.

  And more:

  - [x] support different implementation of KPS calculator.
  - [x] draw the key correctly.
  - [x] draw the graph.

- [ ] details.

  - [x] revise the code in drawing the graph --- use `size()` instead of `history_count`.
  - [x] add `&` for button count menu.
  - [ ] correct the size when the buttons are hidden.
  - [ ] draw more aux. info. in the graph.
  - [ ] perfect the behavior when resizing.

- [ ] tool header for config system.

  I may use json.

  After this step the project can be pre-released.

- [ ] add more keys in keyboard_char.

- [ ] add keyboard shortcut.

- [ ] do the documents.

- [ ] implement "soft" KPS method.

- [ ] tool header for multi-language support.

  I may use json.

  After this step the project can be released.

- [ ] implement functions specific to osu!.

## Details

### The "hard" method to calculate KPS

I just save all the pressed keys, and based on it, calculate KPS. The KPS equals to the total times you press in recent 1 second.  

## License

Copyright (c) UnnamedOrange. Licensed under the MIT Licence.

See the [LICENSE](./LICENSE) file in the repository root for full licence text.

## Credits

### Exo 2

Licensed under the [SIL Open Font License](https://www.fontsquirrel.com/license/exo-2).

```
Copyright (c) 2013, Natanael Gama (www.ndiscovered.com . info(at)ndiscovered.com), with Reserved Font Name Exo.

This Font Software is licensed under the SIL Open Font License, Version 1.1.
```