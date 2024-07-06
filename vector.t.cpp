
#include <iostream>
#include <ostream>

#include "vector.hpp"
#include "lifetime.hpp"

int main() {
    using namespace dbstd;

    Vector<int> intVec{2};
    intVec.push_back(4);
    intVec.push_back(2);

    Vector<Lifetime> lfVec;
    lfVec.push_back(Lifetime{"moved lf"});
    const Lifetime toCopy{"copied lf"};
    lfVec.push_back(toCopy);
    lfVec.emplace_back("emplaced");

    std::cout << "\nfirst vec contents************\n";
    std::cout << lfVec[0] << '\n';
    std::cout << lfVec[1] << '\n';
    std::cout << lfVec[2] << '\n';
    std::cout << "**************************\n\n";

    Vector<Lifetime> copyConstructed{lfVec};
    std::cout << "\ncopy constructed contents************\n";
    std::cout << copyConstructed[0] << '\n';
    std::cout << copyConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    Vector<Lifetime> moveConstructed{std::move(lfVec)};
    std::cout << "\nmove constructed contents************\n";
    std::cout << moveConstructed[0] << '\n';
    std::cout << moveConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    copyConstructed = moveConstructed;
    std::cout << "\ncopy assigned contents************\n";
    std::cout << copyConstructed[0] << '\n';
    std::cout << copyConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    moveConstructed = std::move(copyConstructed);
    std::cout << "\nmove assigned contents************\n";
    std::cout << moveConstructed[0] << '\n';
    std::cout << moveConstructed[1] << '\n';
    std::cout << "**************************\n\n";


    return 0;
}
