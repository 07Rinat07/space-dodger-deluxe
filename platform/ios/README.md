# iOS wrapper

Здесь лежит стартовый iOS CMake/Xcode-проект для порта поверх raylib.

## Содержимое

- `CMakeLists.txt` — iOS bundle target с общим C++ кодом.
- `build_ios.sh` — генерация Xcode build directory и сборка Release.

## Сборка

Нужны macOS, Xcode, CMake и raylib, доступный через `find_package(raylib)`.
На Linux этот проект можно только подготовить/проверить статически: реальная сборка и запуск требуют `xcodebuild` из Xcode.

```bash
./build_ios.sh
```

Скрипт копирует `assets/` в bundle resources после сборки.
