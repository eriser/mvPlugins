#pragma once

namespace mvSynth {

template <typename T>
class Frame
{
    friend class Synth;
    friend class Module;

    T* mData;
    size_t mDataSize;  // data size in bytes
    size_t mSize;      // data size in samples

public:
    Frame()
    {
        mData = 0;
        mSize = 0;
        mDataSize = 0;
    }

    ~Frame()
    {
        if (mData)
        {
            _aligned_free(mData);
            mData = 0;
        }

        mDataSize = 0;
        mDataSize = 0;
    }

    bool Resize(size_t newSize)
    {
        if (mSize == newSize)
            return true;

        mDataSize = sizeof(T) * newSize;
        mData = (T*)_aligned_realloc(mData, mDataSize, 64);
        Zero();

        if (newSize)
        {
            mSize = newSize;
            return true;
        }

        mSize = 0;
        mDataSize = 0;
        return false;
    }

    void Zero()
    {
        memset(mData, 0, mDataSize);
    }

    void Clear(const T &value)
    {
        for (size_t i = 0; i < mSize; i++)
            mData[i] = value;
    }

    // access
    T& operator[] (size_t i) const
    {
        return mData[i];
    }


    // assign
    Frame& operator= (const Frame &src)
    {
        if (mSize == src.mSize)
        {
            memcpy(mData, src.mData, sizeof(T) * mSize);
        }

        return *this;
    }

    // accumulate
    Frame& operator+= (const Frame &src)
    {
        if (mSize == src.mSize)
        {
            for (size_t i = 0; i < mSize; i++)
                mData[i] += src.mData[i];
        }

        return *this;
    }

    void Store(T *dest) const
    {
        for (size_t i = 0; i < mSize; i++)
            dest[i] = mData[i];
    }
};

} // namespace mvSynth