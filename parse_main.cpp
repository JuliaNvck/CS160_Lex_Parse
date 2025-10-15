#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Helper to split a string by a delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> parts;
    std::string part;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, part, delimiter)) {
        parts.push_back(part);
    }
    return parts;
}

// Helper to convert the string tokens from the file into Token structs
std::vector<Token> tokenize_input(const std::string& line) {
    std::vector<Token> tokens;
    std::vector<std::string> string_tokens = split(line, ' ');
    for (size_t i = 0; i < string_tokens.size(); ++i) {
        const auto& str_tok = string_tokens[i];
        if (str_tok.empty()) continue;

        size_t open_paren = str_tok.find('(');
        if (open_paren != std::string::npos) {
            // Token with value, e.g., Id(x) or Num(42)
            std::string type = str_tok.substr(0, open_paren);
            std::string value = str_tok.substr(open_paren + 1, str_tok.length() - open_paren - 2);
            tokens.push_back({type, value, i});
        } else {
            tokens.push_back({str_tok, "", i});
        }
    }
    return tokens;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: parse <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string line;
    std::getline(file, line);

    std::vector<Token> tokens = tokenize_input(line);

    try {
        Parser parser(tokens);
        std::unique_ptr<Program> ast = parser.parse();
        ast->print(std::cout);
        std::cout << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}