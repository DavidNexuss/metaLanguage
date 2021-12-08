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

namespace Parser {
    expression parse(const std::string& buffer,const std::string& filename);
};

struct expression {
    
    ex_type type = ex_type::statement;
    expression_vec exprs;

    std::string strvalue{};

    expression() { }
    expression(std::string&& str) : type(ex_type::identifier), strvalue(std::move(str)) { }

    void push_back(expression&& other) { exprs.push_back(std::move(other)); }

    void write(std::ostream& out) const {
        if(type == ex_type::identifier) out << strvalue << " ";
        else{
            out << "( ";
            for(const auto& child : exprs) child.write(out);
            out << ") ";
        }
    }

    void read(const std::string& str,const std::string& filename) {
        *this = Parser::parse(str,filename);
    }
};

inline std::ostream& operator<<(std::ostream&os,const expression& expr) {
    expr.write(os);
    return os;
}