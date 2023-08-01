# osu!kps

KPS (keys per second) indicator for osu! in C++.

## Motivation

It's been a long time since I wrote a complete project. osu!kps could be a best chance for me to develop a new framework in C++.

## Environment

Target platform: Windows® 10

Target compiler: Visual Studio 17 2022

Language: C++20.

Graphics: Direct2D and DirectWrite.

## How to translate for this project

Supporting languages right now:

- English (by UnnamedOrange)
- 简体中文 / Simplified Chinese (by UnnamedOrange)

If you want to translate, follow the steps below.

1. Fork this repository and clone the forked.
2. Make a copy of [en-us.json](jsons/language/en-us.json), and translate based on it. Ignore "id", "locale" in the json file. If you think some things should remain English, just delete those items in the json file; they will be presented in English automatically.
3. Make a pull request.
4. The remaining work can be done by UnnamedOrange.

## License

Copyright (c) UnnamedOrange. Licensed under the MIT License.

See the [LICENSE](./LICENSE) file in the repository root for full license text.

## Special thanks

Thanks to the following guys that tried this project out. This helped me a lot. The list is in no particular order.

- (osu ID) InabaMeguruSama
- (osu ID) Induction
- (osu ID) frankeskybr
- (osu ID) 13145200

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
