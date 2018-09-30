#include "utils.hpp"

std::string readFile(const char *filename)
{
    std::ifstream is(filename, std::ios::binary);
    std::stringstream ss;
    ss << is.rdbuf();
    is.close();
    return ss.str();
}