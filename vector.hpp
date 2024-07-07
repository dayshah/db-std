#ifndef DBSTD_VECTOR
#define DBSTD_VECTOR

#include <memory>

namespace dbstd {

template <typename T, class Allocator=std::allocator<T>>
class Vector {

private:
    Allocator alloc;
    size_t capacity;
    size_t size;
    T* backing;

    T* createBacking(size_t capacity) {
        return alloc.allocate(capacity);
    }

    void freeBacking(T* toFree, size_t size) {
        for (T* location = toFree; location < (toFree+size); ++location)
            location->~T();
        alloc.deallocate(toFree, capacity);
    }

    void copyBacking(T* inputStart, T* inputEnd, T* outputStart) {
        for (; inputStart < inputEnd; ++inputStart, ++outputStart)
            new (outputStart) T(*inputStart);
    }

public:
    Vector(const Allocator& alloc = Allocator())
    : alloc(alloc)
    , capacity(0)
    , size(0)
    , backing(createBacking(capacity))
    {}

    Vector(size_t capacity, const Allocator& alloc = Allocator())
    : alloc(alloc)
    , capacity(capacity)
    , size(0)
    , backing(createBacking(capacity))
    {}

    Vector(const Vector& other, const Allocator& alloc = Allocator())
    : alloc(alloc)
    , capacity(other.capacity)
    , size(other.size)
    , backing(createBacking(capacity))
    {
        copyBacking(other.backing, other.backing+size, backing); 
    }

    Vector(Vector&& other) noexcept
    : alloc(other.alloc)
    , capacity(other.capacity)
    , size(other.size)
    , backing(other.backing)
    {
        other.backing = nullptr;
    }

    // Copy swap example
    // friend void swap(Vector first, Vector second) {
    //     std::swap(first.capacity, second.capacity);
    //     std::swap(first.size, second.size);
    //     std::swap(first.backing, second.backing);
    // }
    // Vector& operator=(Vector other) noexcept {
    //     swap(*this, other);
    //     return *this;
    // }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            freeBacking(backing, size);
            capacity = other.capacity;
            size = other.size;
            backing = createBacking(capacity);
            copyBacking(other.backing, other.backing + other.size, backing);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            freeBacking(backing, size);
            alloc = other.alloc;
            capacity = other.capacity;
            size = other.size;
            backing = other.backing;
            other.backing = nullptr;
        }
        return *this;
    }

    ~Vector() {
        if (backing != nullptr) freeBacking(backing, size);
    }

    void reserve(size_t newCapacity) {
        capacity = newCapacity;
        T* newBacking = createBacking(capacity);
        copyBacking(backing, backing+size, newBacking);
        freeBacking(backing, size);
        backing = newBacking;
    }

    template <typename U>
    void push_back(U&& item) {
        if (size == capacity) {
            capacity == 0
                ? reserve(1)
                : reserve(capacity*2);
        }
        new (backing+size) T(std::forward<U>(item));
        ++size;
    }

    template <typename ...Args>
    void emplace_back(Args&&... args) {
        if (size == capacity) {
            capacity == 0
                ? reserve(1)
                : reserve(capacity*2);
        }
        new (backing+size) T(std::forward<Args>(args)...);
        ++size;
    }

    T& operator[](size_t index) {
        return *(backing + index);
    }
};

}

#endif
