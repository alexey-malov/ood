# gfx_editor

CMake-проект на базе [SFML 3.x](https://www.sfml-dev.org/).

## Требования

- CMake 3.8+
- MSVC (Visual Studio 2019 или новее)
- Ninja
- SFML 3.x (prebuilt binaries для Windows)

## Настройка SFML

Проект использует `CMakeUserPresets.json` для хранения машинозависимых путей. Этот файл **не хранится в репозитории** — каждый разработчик создаёт его самостоятельно.

### 1. Скачайте SFML

Скачайте prebuilt-архивы SFML 3.x для MSVC с [sfml-dev.org](https://www.sfml-dev.org/download.php):

- `SFML-3.x.x-windows-vc17-32-bit.zip`
- `SFML-3.x.x-windows-vc17-64-bit.zip`

Распакуйте их, например:

```
C:\sdk\sfml-x86\SFML-3.1.0\
C:\sdk\sfml-x64\SFML-3.1.0\
```

### 2. Создайте CMakeUserPresets.json

Создайте файл `CMakeUserPresets.json` в корне проекта (рядом с `CMakePresets.json`), подставив ваши пути:

```json
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 8,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "x64-debug",
            "displayName": "x64 Debug",
            "inherits": "x64-debug-base",
            "cacheVariables": {
                "SFML_DIR": "C:/sdk/sfml-x64/SFML-3.1.0/lib/cmake/SFML"
            }
        },
        {
            "name": "x64-release",
            "displayName": "x64 Release",
            "inherits": "x64-release-base",
            "cacheVariables": {
                "SFML_DIR": "C:/sdk/sfml-x64/SFML-3.1.0/lib/cmake/SFML"
            }
        },
        {
            "name": "x86-debug",
            "displayName": "x86 Debug",
            "inherits": "x86-debug-base",
            "cacheVariables": {
                "SFML_DIR": "C:/sdk/sfml-x86/SFML-3.1.0/lib/cmake/SFML"
            }
        },
        {
            "name": "x86-release",
            "displayName": "x86 Release",
            "inherits": "x86-release-base",
            "cacheVariables": {
                "SFML_DIR": "C:/sdk/sfml-x86/SFML-3.1.0/lib/cmake/SFML"
            }
        }
    ]
}
```

### 3. Откройте проект

**Visual Studio:**
`File > Open > CMake...` — выберите корневой `CMakeLists.txt`. В выпадающем списке конфигураций появятся пресеты из вашего `CMakeUserPresets.json`.

**Командная строка:**
```bat
cmake --preset x64-debug
cmake --build --preset x64-debug
```

## Структура проекта

```
gfx_editor/
├── CMakeLists.txt           # корневой CMake-файл
├── CMakePresets.json        # общие пресеты (в репозитории)
├── CMakeUserPresets.json    # локальные пресеты с путями (не в репозитории)
└── 01_hello/
    ├── CMakeLists.txt
    └── main.cpp             # пример: окно SFML с прямоугольником
```

## Примечания

- SFML `.dll` файлы автоматически копируются рядом с `.exe` при сборке.
- `CMakeUserPresets.json` добавлен в `.gitignore` — не коммитьте его.
