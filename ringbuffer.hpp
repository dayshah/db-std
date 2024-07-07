#ifndef DBSTD_RINGBUFFER
#define DBSTD_RINGBUFFER

#include <memory>
#include <optional>
#include <bit>

namespace dbstd {

template<typename T>
class DRingBuffer {
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
    DRingBuffer(size_t minimumCapacity)
    : capacity(std::bit_ceil(minimumCapacity)) // closest larger or equal power of 2
    , mSize(0)
    , backing(static_cast<T*>(::operator new(sizeof(T) * capacity)))
    , headIdx(0) {}

    DRingBuffer(const DRingBuffer& other) = delete;
    DRingBuffer& operator=(const DRingBuffer& other) = delete;
    
    DRingBuffer(DRingBuffer&& other) noexcept
    : capacity(other.capacity)
    , mSize(other.mSize)
    , backing(other.backing)
    , headIdx(other.headIdx) {
        other.backing = nullptr;
        other.head = nullptr;
    }

    DRingBuffer& operator=(DRingBuffer&& other) noexcept {
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

    ~DRingBuffer() { freeBacking(); }

    template <typename ...Args>
    bool enqueue(Args&&... args) {
        if (mSize == capacity) {
            return false;
        }
        size_t idx = headIdx + mSize;
        new 
            (backing + (idx & (capacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
        return true;
    }

    template <typename ...Args>
    void unchecked_enqueue(Args&&... args) {
        size_t idx = headIdx + mSize;
        new 
            (backing + (idx & (capacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    void dequeue() {
        if (mSize > 0) {
            (backing + headIdx)->~T();
            headIdx = (headIdx + 1) & (capacity - 1);
            --mSize;
        }
    }

    void unchecked_dequeue() {
        (backing + headIdx)->~T();
        headIdx = (headIdx + 1) & (capacity - 1);
        --mSize;
    }

    std::optional<T> dequeue_and_get() {
        std::optional<T> result = std::nullopt;
        if (mSize > 0) {
            result.emplace(std::move(*(backing+headIdx)));
            (backing + headIdx)->~T();
            headIdx = (headIdx + 1) & (capacity - 1);
            --mSize;
        }
        return result;
    }

    T unchecked_dequeue_and_get() {
        T result = std::move(*(backing+headIdx));
        (backing + headIdx)->~T();
        headIdx = (headIdx + 1) & (capacity - 1);
        --mSize;
        return result;
    }

    T& front() { return *(backing + headIdx); }
    
    const T& front() const { return *(backing + headIdx); }

    bool empty() const { return mSize == 0; }

    size_t size() const { return mSize; }
};

}

#endif
