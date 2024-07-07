#ifndef DBSTD_RINGBUFFER
#define DBSTD_RINGBUFFER

#include <memory>
#include <optional>
#include <bit>

namespace dbstd {

template<typename T>
class RingBuffer {

private:
    size_t mCapacity;
    size_t mSize;
    T* backing;
    size_t headIdx;

    void freeBacking() {
        size_t end = headIdx + mSize;
        for (size_t idx = headIdx; idx < end; ++idx)
            (backing + (idx % mCapacity))->~T();
        ::operator delete(backing);
    }

public:
    RingBuffer(size_t minimumCapacity)
    : mCapacity(std::bit_ceil(minimumCapacity)) // closest larger or equal power of 2
    , mSize(0)
    , backing(static_cast<T*>(::operator new(sizeof(T) * mCapacity)))
    , headIdx(0) {}

    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer& operator=(const RingBuffer& other) = delete;
    
    RingBuffer(RingBuffer&& other) noexcept
    : mCapacity(other.mCapacity)
    , mSize(other.mSize)
    , backing(other.backing)
    , headIdx(other.headIdx) {
        other.backing = nullptr;
        other.head = nullptr;
    }

    RingBuffer& operator=(RingBuffer&& other) noexcept {
        if (this != &other) {
            freeBacking();
            mCapacity = other.mCapacity;
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
        if (mSize == mCapacity) {
            return false;
        }
        size_t idx = headIdx + mSize;
        new 
            (backing + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
        return true;
    }

    template <typename ...Args>
    void unchecked_enqueue(Args&&... args) {
        size_t idx = headIdx + mSize;
        new 
            (backing + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    void dequeue() {
        if (mSize > 0) {
            (backing + headIdx)->~T();
            headIdx = (headIdx + 1) & (mCapacity - 1);
            --mSize;
        }
    }

    void unchecked_dequeue() {
        (backing + headIdx)->~T();
        headIdx = (headIdx + 1) & (mCapacity - 1);
        --mSize;
    }

    std::optional<T> dequeue_and_get() {
        std::optional<T> result = std::nullopt;
        if (mSize > 0) {
            result.emplace(std::move(*(backing+headIdx)));
            (backing + headIdx)->~T();
            headIdx = (headIdx + 1) & (mCapacity - 1);
            --mSize;
        }
        return result;
    }

    T unchecked_dequeue_and_get() {
        T result = std::move(*(backing+headIdx));
        (backing + headIdx)->~T();
        headIdx = (headIdx + 1) & (mCapacity - 1);
        --mSize;
        return result;
    }

    T& front() { return *(backing + headIdx); }
    
    const T& front() const { return *(backing + headIdx); }

    bool empty() const { return mSize == 0; }

    size_t size() const { return mSize; }

    size_t capacity() const { return mCapacity; }
};

}

#endif
