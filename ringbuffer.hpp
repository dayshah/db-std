#ifndef DBSTD_RINGBUFFER
#define DBSTD_RINGBUFFER

#include <memory>
#include <optional>
#include <bit>
#include <thread>
#include <new>

// idk gcc on my machine setup didn't define the necessary macros
namespace std {
inline constexpr size_t hardware_destructive_interference_size = 256;
inline constexpr size_t hardware_constructive_interference_size = 256;
}

namespace dbstd {

// Lock-free thread safe single producer single consumer queue

template<typename T, typename Allocator=std::allocator<T>>
class RingBuffer {

private:
    Allocator mAlloc;
    size_t mCapacity;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> mSize;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> mHeadIdx;
    T* mBacking;

    void freeBacking() {
        size_t end = mHeadIdx + mSize;
        for (size_t idx = mHeadIdx; idx < end; ++idx)
            (mBacking + (idx % mCapacity))->~T();
        mAlloc.deallocate(mBacking, mCapacity);
    }

public:
    // capacity will be closest larger power of 2
    RingBuffer(size_t minimumCapacity, const Allocator& alloc=std::allocator<T>())
    : mAlloc(alloc)
    , mCapacity(std::bit_ceil(minimumCapacity))
    , mSize(0)
    , mHeadIdx(0)
    , mBacking(mAlloc.allocate(mCapacity))
    {}

    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer& operator=(const RingBuffer& other) = delete;
    
    RingBuffer(RingBuffer&& other) noexcept
    : mAlloc(other.mAlloc)
    , mCapacity(other.mCapacity)
    , mSize(other.mSize)
    , mHeadIdx(other.mHeadIdx) 
    , mBacking(other.mBacking) {
        other.mBacking = nullptr;
        other.head = nullptr;
    }

    RingBuffer& operator=(RingBuffer&& other) noexcept {
        if (this != &other) {
            freeBacking();
            mAlloc = (other.mAlloc);
            mCapacity = other.mCapacity;
            mSize = other.mSize;
            mBacking = other.mBacking;
            mHeadIdx = other.mHeadIdx;
            other.mBacking = nullptr;
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
        size_t idx = mHeadIdx + mSize;
        new 
            (mBacking + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
        return true;
    }

    template <typename ...Args>
    void blocking_enqueue(Args&&... args) {
        while (mSize == mCapacity) {}
        size_t idx = mHeadIdx + mSize;
        new 
            (mBacking + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    template <typename ...Args>
    void blocking_sleeping_enqueue(int sleepNano, Args&&... args) {
        while (mSize == mCapacity) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepNano));
        }
        size_t idx = mHeadIdx + mSize;
        new 
            (mBacking + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    template <typename ...Args>
    void unchecked_enqueue(Args&&... args) {
        size_t idx = mHeadIdx + mSize;
        new 
            (mBacking + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    bool dequeue() {
        if (mSize > 0) {
            (mBacking + mHeadIdx)->~T();
            mHeadIdx = (mHeadIdx + 1) & (mCapacity - 1);
            --mSize;
            return true;
        }
        return false;
    }

    void unchecked_dequeue() {
        (mBacking + mHeadIdx)->~T();
        mHeadIdx = (mHeadIdx + 1) & (mCapacity - 1);
        --mSize;
    }

    std::optional<T> dequeue_and_get() {
        std::optional<T> result = std::nullopt;
        if (mSize > 0) {
            result.emplace(std::move(*(mBacking+mHeadIdx)));
            (mBacking + mHeadIdx)->~T();
            mHeadIdx = (mHeadIdx + 1) & (mCapacity - 1);
            --mSize;
        }
        return result;
    }

    T unchecked_dequeue_and_get() {
        T result = std::move(*(mBacking+mHeadIdx));
        (mBacking + mHeadIdx)->~T();
        mHeadIdx = (mHeadIdx + 1) & (mCapacity - 1);
        --mSize;
        return result;
    }

    T& front() { return *(mBacking + mHeadIdx); }
    
    const T& front() const { return *(mBacking + mHeadIdx); }

    bool empty() const { return mSize == 0; }

    size_t size() const { return mSize; }

    size_t capacity() const { return mCapacity; }
};

}

#endif
