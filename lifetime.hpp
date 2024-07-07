#ifndef DBSTD_LIFETIME
#define DBSTD_LIFETIME

#include <iostream>
#include <ostream>

namespace dbstd {

struct Lifetime {
    std::string* data;

    Lifetime(std::string_view data): data(new std::string{data})
    { std::cout << "constructing" << std::endl; }

    Lifetime(const Lifetime& other)
    : data(new std::string(*other.data))
    { std::cout << "copying" << std::endl; }

    Lifetime(Lifetime&& other) noexcept
    : data(other.data)
    {
        other.data = nullptr;
        std::cout << "moving" << std::endl;
    }

    Lifetime& operator=(const Lifetime& other) {
        std::cout << "copy assigning" << std::endl;
        if (this != &other) {
            delete this->data;
            this->data = new std::string(*other.data);
        }
        return *this;
    }

    Lifetime& operator=(Lifetime&& other) noexcept {
        std::cout << "move assigning" << std::endl;
        if (this != &other) {
            delete this->data;
            this->data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    ~Lifetime() 
    {
        std::cout << "destructing" << std::endl;
        delete this->data;
    }

    friend std::ostream& operator<<(std::ostream& out, const Lifetime& lfObj) {
        out << *lfObj.data;
        return out;
    }
};

}

#endif
