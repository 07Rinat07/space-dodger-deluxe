# Android wrapper

Здесь лежит стартовый Android Gradle/NDK-проект для порта поверх raylib.

## Содержимое

- `settings.gradle` и `build.gradle` — Gradle-конфигурация.
- `app/build.gradle` — Android application target.
- `app/src/main/AndroidManifest.xml` — `NativeActivity`.
- `app/src/main/cpp/CMakeLists.txt` — подключение общего C++ кода.
- `app/src/main/assets/` — место для копии папки `assets/`.

## Сборка

Нужны Android Studio, Android SDK, Android NDK и raylib checkout, собранный/настроенный для Android.

Пример:

```bash
cp -R ../../assets app/src/main/assets/
./gradlew assembleDebug -PRAYLIB_ANDROID_SOURCE=/path/to/raylib
```

В `app/src/main/cpp/CMakeLists.txt` переменная `RAYLIB_ANDROID_SOURCE` должна указывать на локальный raylib checkout.
