#include "serialise.hh"

namespace rust {

// Depending on the internal representation of Rust slices is slightly
// evil...
template<typename T>
struct Slice
{
    T * ptr;
    size_t size;

    Slice(T * ptr, size_t size) : ptr(ptr), size(size)
    {
        assert(ptr);
    }
};

struct StringSlice : Slice<char>
{
    StringSlice(const std::string & s): Slice((char *) s.data(), s.size()) {}
};

struct Source
{
    size_t (*fun)(void * source_this, rust::Slice<uint8_t> data);
    nix::Source * _this;

    Source(nix::Source & _this)
        : fun(sourceWrapper), _this(&_this)
    {}

    // FIXME: how to propagate exceptions?
    static size_t sourceWrapper(void * _this, rust::Slice<uint8_t> data)
    {
        auto n = ((nix::Source *) _this)->read(data.ptr, data.size);
        return n;
    }
};

/* C++ representation of Rust's Result<T, CppException>. */
template<typename T>
struct Result
{
    unsigned int tag;

    union {
        T data;
        std::exception_ptr * exc;
    };

    /* Rethrow the wrapped exception or return the wrapped value. */
    T unwrap()
    {
        if (tag == 0)
            return data;
        else if (tag == 1)
            std::rethrow_exception(*exc);
        else
            abort();
    }
};

template<typename T>
struct CBox
{
    T * ptr;

    T * operator ->()
    {
        return ptr;
    }

    CBox(T * ptr) : ptr(ptr) { }
    CBox(const CBox &) = delete;
    CBox(CBox &&) = delete;

    ~CBox()
    {
        free(ptr);
    }
};

}
