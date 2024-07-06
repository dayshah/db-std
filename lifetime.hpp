#ifndef DBSTD_LIFETIME
#define DBSTD_LIFETIME

#include <iostream>
#include <ostream>

namespace dbstd {

struct Lifetime {
    std::string data;

    Lifetime(std::string_view data): data(data)
    { std::cout << "constructing" << std::endl; }

    Lifetime(const Lifetime& other)
    : data(other.data)
    { std::cout << "copying" << std::endl; }

    Lifetime(Lifetime&& other) noexcept
    : data(std::move(other.data))
    { std::cout << "moving" << std::endl; }

    friend void swap(Lifetime& first, Lifetime& second) {
        std::swap(first.data, second.data);
    }

    Lifetime& operator=(const Lifetime& other) {
        std::cout << "copy assigning" << std::endl;
        if (this == &other) return *this;
        auto otherCopy = other;
        swap(*this, otherCopy);
        return *this;
    }

    Lifetime& operator=(Lifetime&& other) noexcept {
        std::cout << "move assigning" << std::endl;
        if (this == &other) return *this;
        swap(*this, other);
        return *this;
    }

    ~Lifetime() 
    { std::cout << "destructing" << std::endl; }

    friend std::ostream& operator<<(std::ostream& out, const Lifetime& lfObj) {
        out << lfObj.data;
        return out;
    }
};

}

#endif
