# osu!kps

KPS (keys per second) indicator for osu! in C++.

This project is under occasional maintenance.

## Motivation

It's been a long time since I wrote a complete project. osu!kps could be a best chance for me to develop a new framework in C++.

## Environment

Target platform: Windows® 10.

IDE: Visual Studio® 2019.

Language: C++20.

Graphics: Direct2D and DirectWrite.

## How to get the executable

If you are a developer, it's strongly recommended that you deploy this project on your own. Just follow the steps below.

1. Make sure your PC runs Windows® 10, which should not be too old.
2. Install Visual Studio® 2019, with toolkits of Windows desktop applications in C++.
3. Clone this repository.
4. Open the solution file with VS.
5. Select "Release" and "x64", and build the project.
6. Runs the executable.

If you are not familiar with programming, you can go to the [release page](https://github.com/UnnamedOrange/osu-kps/releases), and download the latest version of it (osu-kps-x64.exe) and run. Before this, you should also make sure your PC runs Windows® 10 that is not too old.

## How to translate for this project

Supporting languages right now:

- English (by UnnamedOrange)
- 简体中文 / Simplified Chinese (by UnnamedOrange)

If you want to translate, follow the steps below.

1. Fork this repository and clone the forked.
2. Make a copy of [en-us.json](jsons/language/en-us.json), and translate based on it. Ignore "id", "locale" in the json file. If you think some things should remain English, just delete those items in the json file; they will be presented in English automatically.
3. Make a pull request.
4. The remaining work can be done by UnnamedOrange.

## Roadmap

The remaining targets won't be updated recently.

- [x] first beta release.

- [ ] add more keys in keyboard_char.

- [ ] add keyboard shortcut.

- [ ] do the documents.

  After this step the project can be released.

- [ ] implement functions specific to osu!.

## Details

### The "hard" method to calculate KPS

It just saves all key strokes, and based on it, calculates KPS. The KPS equals to the total times you stroke in recent 1 second.

### The "sensitive" method to calculate KPS

This method allows the situation that KPS is higher than the number of keys you stroke in recent 1 second. Details will be written in the documents (not ready yet).

## License

Copyright (c) UnnamedOrange. Licensed under the MIT License.

See the [LICENSE](./LICENSE) file in the repository root for full license text.

## Special thanks

Thanks to the following guys that tried this project out. This helped me a lot. The list is in no particular order.

- (osu ID) InabaMeguruSama
- (osu ID) Induction
- (osu ID) frankeskybr

## Credits

### Exo 2

Licensed under the [SIL Open Font License](https://www.fontsquirrel.com/license/exo-2).

```
Copyright (c) 2013, Natanael Gama (www.ndiscovered.com . info(at)ndiscovered.com), with Reserved Font Name Exo.

This Font Software is licensed under the SIL Open Font License, Version 1.1.
```

### JsonCpp

Licensed under the [MIT License](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE).

```
Copyright (c) 2007-2010 Baptiste Lepilleur and The JsonCpp Authors

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### osu-memory

Licensed under the [MIT License](https://github.com/UnnamedOrange/osu-memory/blob/main/LICENSE).

```
MIT License

Copyright (c) 2021 UnnamedOrange

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
