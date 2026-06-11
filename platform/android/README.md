# Android wrapper

Здесь описана минимальная обвязка для Android-порта поверх raylib.

## Подход

1. Собрать raylib для Android NDK.
2. Использовать текущие `src/` и `include/` как общий игровой модуль.
3. Добавить Android Activity, которая создаёт окно raylib и запускает `Game`.
4. Скопировать `assets/` в Android assets.

## Что уже готово в проекте

- Игра не зависит от абсолютных путей.
- Ассеты лежат в `assets/textures`, `assets/sounds`, `assets/music`.
- Логика игры отделена от тестов через `UNIT_TEST`.

## Следующий практический шаг

Создать Gradle-проект с CMake/NDK и подключить этот репозиторий как native source set.
