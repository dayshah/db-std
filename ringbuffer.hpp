#ifndef DBSTD_RINGBUFFER
#define DBSTD_RINGBUFFER

#include <memory>
#include <optional>
#include <bit>

namespace dbstd {

template<typename T, typename Allocator=std::allocator<T>>
class RingBuffer {

private:
    Allocator mAlloc;
    size_t mCapacity;
    size_t mSize;
    T* mBacking;
    size_t mHeadIdx;

    void freeBacking() {
        size_t end = mHeadIdx + mSize;
        for (size_t idx = mHeadIdx; idx < end; ++idx)
            (mBacking + (idx % mCapacity))->~T();
        mAlloc.deallocate(mBacking, mCapacity);
    }

public:
    RingBuffer(size_t minimumCapacity, const Allocator& alloc=std::allocator<T>())
    : mAlloc(alloc)
    , mCapacity(std::bit_ceil(minimumCapacity)) // closest larger or equal power of 2
    , mSize(0)
    , mBacking(mAlloc.allocate(mCapacity))
    , mHeadIdx(0)
    {}

    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer& operator=(const RingBuffer& other) = delete;
    
    RingBuffer(RingBuffer&& other) noexcept
    : mAlloc(other.mAlloc)
    , mCapacity(other.mCapacity)
    , mSize(other.mSize)
    , mBacking(other.mBacking)
    , mHeadIdx(other.mHeadIdx) {
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
    void unchecked_enqueue(Args&&... args) {
        size_t idx = mHeadIdx + mSize;
        new 
            (mBacking + (idx & (mCapacity - 1)))
            T(std::forward<Args>(args)...);
        ++mSize;
    }

    void dequeue() {
        if (mSize > 0) {
            (mBacking + mHeadIdx)->~T();
            mHeadIdx = (mHeadIdx + 1) & (mCapacity - 1);
            --mSize;
        }
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
