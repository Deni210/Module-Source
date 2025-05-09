#pragma once

#include <string_view>
#include <cstdint>
#include <string>

/*
              SET                                 GET
    VMValue0: Data = Value                     || Value = Data
    VMValue1: Data = (Value + (Data + Offset)) || Value = (Data - (Value + Offset))
    VMValue2: Data = ((Data + Offset) - Value) || Value = ((v + Offset) - Data)
    VMValue3: Data = (Value ^ (Data + Offset)) || Value = ((v + Offset) ^ Data)
    VMValue4: Data = (Value - (Data + Offset)) || Value = ((v + Offset) + Data)
*/

template<typename T>
struct VMValue0
{
private:
    T storage;

public:
    operator const T() const
    {
        return storage;
    }

    void operator=(const T& value)
    {
        storage = value;
    }

    const T operator->() const
    {
        return operator const T();
    }

    T get()
    {
        return operator const T();
    }

    void set(const T& value)
    {
        operator=(value);
    }
};

template <typename T> struct VMValue1 {
private:
    T Storage;
public:
    operator const T() const {
        return (T)((uintptr_t)this->Storage - (uintptr_t)this);
    }

    void operator=(const T& Value) {
        this->Storage = (T)((uintptr_t)Value + (uintptr_t)this);
    }

    const T operator->() const {
        return operator const T();
    }

    T Get() {
        return operator const T();
    }

    void Set(const T& Value) {
        operator=(Value);
    }
};

template <typename T> struct VMValue2 {
private:
    T Storage;
public:
    operator const T() const {
        return (T)((uintptr_t)this - (uintptr_t)this->Storage);
    }

    void operator=(const T& Value) {
        this->Storage = (T)((uintptr_t)this - (uintptr_t)Value);
    }

    const T operator->() const {
        return operator const T();
    }

    T Get() {
        return operator const T();
    }

    void Set(const T& Value) {
        operator=(Value);
    }
};

template <typename T> struct VMValue3 {
private:
    T Storage;
public:
    operator const T() const {
        return (T)((uintptr_t)this ^ (uintptr_t)this->Storage);
    }

    void operator=(const T& Value) {
        this->Storage = (T)((uintptr_t)Value ^ (uintptr_t)this);
    }

    const T operator->() const {
        return operator const T();
    }

    T Get() {
        return operator const T();
    }

    void Set(const T& Value) {
        operator=(Value);
    }
};

template <typename T> struct VMValue4 {
private:
    T Storage;
public:
    operator const T() const {
        return (T)((uintptr_t)this + (uintptr_t)this->Storage);
    }

    void operator=(const T& Value) {
        this->Storage = (T)((uintptr_t)Value - (uintptr_t)this);
    }

    const T operator->() const {
        return operator const T();
    }

    T Get() {
        return operator const T();
    }

    void Set(const T& Value) {
        operator=(Value);
    }
};