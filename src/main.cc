
#include <fstream>
#include "expression.h"

int main(int argc, char** argv)
{
    std::string filename = argv[1];
    std::ifstream f(filename);
    std::string buffer(std::istreambuf_iterator<char>(f), {});
    std::cout << "Input text: " << buffer << std::endl;
    expression expr;
    expr.read(buffer,filename);
    expr.write(std::cout);
}