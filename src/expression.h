#pragma once
#include <list>
#include <string>
#include <iostream>
#include <string>

#define ENUM_EXPRESSIONS(o) \
    o(identifier) \
    o(statement)

#define o(n) n,
enum class ex_type { ENUM_EXPRESSIONS(o) };
#undef o

using expression_vec = std::list<struct expression>;
struct expression {
    
    ex_type type = ex_type::statement;
    expression_vec exprs;

    std::string strvalue{};

    expression() { }
    expression(std::string&& str) : type(ex_type::identifier), strvalue(std::move(str)) { }

    void push_back(expression&& other) { exprs.push_back(std::move(other)); }

    void print(int depth = 0) const
    {
        if(type == ex_type::identifier) {
            std::cout << std::string(depth,' ') << strvalue << std::endl;
            return;
        }
        for(const auto& l : exprs) {
            l.print(depth + 1);
        }
    }
};
