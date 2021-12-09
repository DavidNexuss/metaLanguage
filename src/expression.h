#pragma once
#include <list>
#include <string>
#include <iostream>
#include <string>
#include <iterator>

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

struct expression : public expression_vec {
    
    ex_type type = ex_type::statement;
    std::string strvalue{};

    expression() { }
    expression(std::string&& str) : type(ex_type::identifier), strvalue(std::move(str)) { }

    void write(std::ostream& out) const {
        if(type == ex_type::identifier) out << strvalue << " ";
        else{
            out << "( ";
            for(const auto& child : *this) child.write(out);
            out << ") ";
        }
    }

    void read(const std::string& str,const std::string& filename) {
        *this = Parser::parse(str,filename);
    }

    static expression fromString(const std::string& str,const std::string& filename = "internal") { 
        std::cerr << "Input text: " << str << std::endl;
        return Parser::parse(str,filename); 
    }

    expression& at(size_t index) {
        auto it = begin();
        for (size_t i = 0; i < index; i++) {
            ++it;
        }
        return *(it);
    }
    
    void replace(const std::string& literal,const expression& expr){
        if(type == ex_type::identifier && strvalue == literal) *this = expr;
        for(auto& child : *this) child.replace(literal, expr);
    }

    std::string flat() {
        std::string result;
        if(type == ex_type::identifier) result = strvalue;
        else {
            for (auto& child : *this) result += child.flat() + " ";
        }
        return result;
    }
};

inline std::ostream& operator<<(std::ostream&os,const expression& expr) {
    expr.write(os);
    return os;
}