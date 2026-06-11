# iOS wrapper

Здесь лежит стартовый iOS CMake/Xcode-проект для порта поверх raylib.

## Содержимое

- `CMakeLists.txt` — iOS bundle target с общим C++ кодом.
- `build_ios.sh` — генерация Xcode build directory и сборка Release.

## Сборка

Нужны macOS, Xcode, CMake и raylib, доступный через `find_package(raylib)`.

```bash
./build_ios.sh
```

Скрипт копирует `assets/` в bundle resources после сборки.
