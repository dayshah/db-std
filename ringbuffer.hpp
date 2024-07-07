#ifndef DBSTD_RINGBUFFER
#define DBSTD_RINGBUFFER

#include <__format/format_functions.h>
#include <memory>
#include <optional>

namespace dbstd {

template<typename T>
class RingBuffer {
private:

    size_t capacity;
    size_t mSize;
    T* backing;
    size_t headIdx;

    void freeBacking() {
        size_t end = headIdx + mSize;
        for (size_t idx = headIdx; idx < end; ++idx)
            (backing + (idx % capacity))->~T();
        ::operator delete(backing);
    }

public:
    RingBuffer(size_t capacity)
    : capacity(capacity)
    , mSize(0)
    , backing(static_cast<T*>(::operator new(sizeof(T) * capacity)))
    , headIdx(0)
    {}
    
    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer& operator=(const RingBuffer& other) = delete;
    
    RingBuffer(RingBuffer&& other) noexcept
    : capacity(other.capacity)
    , mSize(other.mSize)
    , backing(other.backing)
    , headIdx(other.headIdx) {
        other.backing = nullptr;
        other.head = nullptr;
    }

    RingBuffer& operator=(RingBuffer&& other) noexcept {
        if (this != &other) {
            freeBacking();
            capacity = other.capacity;
            mSize = other.mSize;
            backing = other.backing;
            headIdx = other.headIdx;
            other.backing = nullptr;
            other.head = nullptr;
        }
        return *this;
    }

    ~RingBuffer() { freeBacking(); }

    template <typename ...Args>
    bool enqueue(Args&&... args) {
        if (mSize == capacity) {
            return false;
        }
        new (backing + ((headIdx + mSize) % capacity)) T(std::forward<Args>(args)...);
        ++mSize;
        return true;
    }

    // void uncheckedEnqueue(Args&&... args) {

    // }

    void dequeue() {
        if (mSize > 0) {
            (backing + headIdx)->~T();
            headIdx = (headIdx+1 == capacity)
                ? 0
                : headIdx + 1;
            --mSize;
        }
    }

    std::optional<T> dequeue_and_get() {
        std::optional<T> result = std::nullopt;
        if (mSize > 0) {
            result.emplace(std::move(*(backing+headIdx)));
            (backing + headIdx)->~T();
            headIdx = (headIdx+1 == capacity)
                ? 0
                : headIdx + 1;
            --mSize;
        }
        return result;
    }

    T& front() { return *(backing + headIdx); }
    
    const T& front() const { return *(backing + headIdx); }

    bool empty() const { return mSize == 0; }

    size_t size() const { return mSize; }
};

}

#endif
