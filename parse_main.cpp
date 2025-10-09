#include <iostream>

/**
 * This is the main entry point for the 'parse' executable.
 * For now, it's just a placeholder.
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::cout << "Parser executable is running, but parser logic is not yet implemented." << std::endl;
    std::cout << "Input file: " << argv[1] << std::endl;

    // In the future, you will add calls to your lexer and parser here.

    return 0;
}