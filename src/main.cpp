
#include "test.hpp"
#include <iostream>
#include <exception>

int main(int argc, char const *argv[]) {
    try {
        Tester().testInlineParse();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
