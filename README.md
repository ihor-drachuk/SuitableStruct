<p align="center">
  <h1 align="center">SuitableStruct</h1>
  <p align="center">
    <strong>C++ Struct Serialization Library</strong>
  </p>
  <p align="center">
    Binary and JSON serialization with data integrity validation, versioning, and auto-generated operators
  </p>
</p>

<p align="center">
  <a href="https://github.com/ihor-drachuk/SuitableStruct/actions/workflows/ci.yml"><img src="https://github.com/ihor-drachuk/SuitableStruct/actions/workflows/ci.yml/badge.svg?branch=master" alt="Build & Test"></a>
  <a href="https://github.com/ihor-drachuk/SuitableStruct/blob/master/License.txt"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Platform-Windows%20|%20Linux%20|%20macOS-blueviolet.svg" alt="Platform">
</p>

---

## Features

- **Binary Serialization** — Fast, compact binary format with hash-based integrity validation
- **JSON Serialization** — Human-readable format with Qt integration (optional)
- **Versioning System** — Seamless struct evolution with automatic migration between versions
- **Comparison Operators** — Auto-generated `==`, `!=`, `<`, `<=`, `>`, `>=` operators
- **Hash Functions** — `std::hash` and `qHash` support for use in containers
- **Extensive Type Support** — STL containers, smart pointers, `std::optional`, Qt types
- **Custom Handlers** — Extend serialization for any type
- **Cross-Platform** — Linux, Windows, macOS with GCC, Clang, MSVC

---

## Quick Start

### 1. Define Your Struct

```cpp
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Hashes.h>

struct Person {
    std::string name;
    int age;
    std::vector<std::string> hobbies;

    // Define which members to serialize
    auto ssTuple() const { return std::tie(name, age, hobbies); }
};

// Generate comparison operators and hash function
SS_COMPARISONS(Person)
SS_HASHES(Person)
```

### 2. Serialize and Deserialize

```cpp
using namespace SuitableStruct;

// Create and serialize
Person alice{"Alice", 30, {"reading", "hiking"}};
Buffer buffer = ssSave(alice);

// Deserialize
Person loaded = ssLoadRet<Person>(buffer);

// They're equal!
assert(alice == loaded);
```

---

## Installation

### CMake Integration

#### Option 1: Add as Subdirectory

```cmake
add_subdirectory(path/to/SuitableStruct)
target_link_libraries(your_target PRIVATE SuitableStruct)
```

#### Option 2: FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    SuitableStruct
    GIT_REPOSITORY https://github.com/ihor-drachuk/SuitableStruct.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(SuitableStruct)
target_link_libraries(your_target PRIVATE SuitableStruct)
```

---

## Usage Examples

### Binary Serialization with Integrity Check

```cpp
#include <SuitableStruct/Serializer.h>

struct Config {
    std::string host;
    int port;
    bool secure;
    std::optional<std::string> apiKey;

    auto ssTuple() const { return std::tie(host, port, secure, apiKey); }
};

// Serialize (with integrity protection enabled by default)
Config cfg{"localhost", 8080, true, "secret-key"};
Buffer data = ssSave(cfg);

// Deserialize (automatically validates integrity)
Config restored = ssLoadRet<Config>(data);

// Corrupted data throws an exception
data.data()[10] ^= 0xFF;  // Corrupt a byte
try {
    ssLoadRet<Config>(data);  // Throws!
} catch (...) {
    // Data integrity violation detected
}
```

### Struct Versioning

Evolve your data structures while maintaining backward compatibility:

```cpp
// Version 0: Original structure
struct Settings_v0 {
    std::string username;

    auto ssTuple() const { return std::tie(username); }
};

// Version 1: Added email field
struct Settings_v1 {
    std::string username;
    std::string email;

    auto ssTuple() const { return std::tie(username, email); }

    void ssConvertFrom(const Settings_v0& old) {
        username = old.username;
        email = "";  // Default value for new field
    }
};

// Version 2: Current version with theme
struct Settings {
    std::string username;
    std::string email;
    std::string theme;

    // Declare version chain
    using ssVersions = std::tuple<Settings_v0, Settings_v1, Settings>;

    auto ssTuple() const { return std::tie(username, email, theme); }

    void ssConvertFrom(const Settings_v1& old) {
        username = old.username;
        email = old.email;
        theme = "light";  // Default theme
    }
};

// Old data automatically migrates to current version
Buffer oldData = ssSave(Settings_v0{"john"});
Settings current = ssLoadRet<Settings>(oldData);
// current.username == "john", current.email == "", current.theme == "light"
```

### JSON Serialization (with Qt)

```cpp
#include <SuitableStruct/SerializerJson.h>

struct ApiResponse {
    QString status;
    int code;
    QStringList messages;
    std::optional<QJsonObject> metadata;

    auto ssTuple() const { return std::tie(status, code, messages, metadata); }

    // Define JSON field names
    auto ssNamesTuple() const {
        return std::tie("status", "code", "messages", "metadata");
    }
};

// Serialize to JSON
ApiResponse response{"success", 200, {"OK"}, std::nullopt};
QJsonValue json = ssJsonSave(response);

// Deserialize from JSON
ApiResponse restored = ssJsonLoadRet<ApiResponse>(json);
```

### Custom Type Handlers

Add serialization support for external types:

```cpp
#include <SuitableStruct/Handlers.h>

// Example: Support for a third-party Point3D class
namespace SuitableStruct {

template<>
struct Handlers<Point3D> : public std::true_type {
    static Buffer ssSaveImpl(const Point3D& p) {
        Buffer buf;
        buf.write(p.x);
        buf.write(p.y);
        buf.write(p.z);
        return buf;
    }

    static void ssLoadImpl(BufferReader& reader, Point3D& p) {
        reader.read(p.x);
        reader.read(p.y);
        reader.read(p.z);
    }
};

} // namespace SuitableStruct
```

---

## Supported Types

### Fundamental Types
`bool`, `char`, `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float`, `double`

### Standard Library
- **Containers**: `std::vector`, `std::list`, `std::deque`, `std::array`, `std::set`, `std::multiset`, `std::unordered_set`, `std::unordered_multiset`, `std::forward_list`
- **Strings**: `std::string`
- **Smart Pointers**: `std::shared_ptr`, `std::unique_ptr`, `std::weak_ptr`
- **Utilities**: `std::optional`, `std::pair`, `std::tuple`
- **Enums**: All enum types

### Qt Types (when available)
`QString`, `QByteArray`, `QPoint`, `QPointF`, `QSize`, `QSizeF`, `QRect`, `QRectF`, `QColor`, `QVector`, `QList`, `QStringList`, `QSet`, `QMap`, `QJsonValue`, `QJsonObject`, `QJsonArray`

---

## Macros Reference

| Macro | Description |
|-------|-------------|
| `SS_COMPARISONS(Type)` | Generates all comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`) |
| `SS_COMPARISONS_ONLY_EQ(Type)` | Generates only equality operators (`==`, `!=`) |
| `SS_COMPARISONS_MEMBER()` | Comparison operators as member functions |
| `SS_HASHES(Type)` | Generates `std::hash<Type>` specialization and `qHash()` |
| `SS_SWAP(Type)` | Generates efficient swap function |

---

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `SUITABLE_STRUCT_ENABLE_TESTS` | `OFF` | Build test suite |
| `SUITABLE_STRUCT_ENABLE_BENCHMARK` | `OFF` | Build benchmarks |
| `SUITABLE_STRUCT_QT_SEARCH_MODE` | `Auto` | Qt detection: `Auto`, `Force`, `Skip` |
| `SUITABLE_STRUCT_GTEST_SEARCH_MODE` | `Auto` | GTest detection: `Auto`, `Force`, `Skip` |

---

## Requirements

- **Compiler**: C++17 or later (GCC, Clang, MSVC)
- **Build System**: CMake 3.16+
- **Platforms**: Windows, Linux, macOS
- **Optional**: Qt 5.15+ or Qt 6.x for JSON serialization and Qt type support

---

## Running Tests

```bash
mkdir build && cd build
cmake -DSUITABLE_STRUCT_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd tests && ctest --output-on-failure
```

---

## License

MIT License — see [License.txt](License.txt) for details.

Copyright (c) 2020-2026 Ihor Drachuk

---

## Author

**Ihor Drachuk** — [ihor-drachuk-libs@pm.me](mailto:ihor-drachuk-libs@pm.me)

[GitHub](https://github.com/ihor-drachuk/SuitableStruct)
