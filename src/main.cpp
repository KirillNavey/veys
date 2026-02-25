#include "core/Application.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        veys::Application app{};
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
