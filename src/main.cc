
#include <fstream>
#include <vector>
#include <functional>
#include "expression.h"


struct MatchRule {
    size_t fields;
    std::string name;
    std::function<expression(expression& expr)> expandRule;
    std::function<bool(const expression& expr)> specialMatching = [](const expression& expr){return true;};

    bool match(const expression& expr) {
        return expr.size() == fields && expr.front().type == ex_type::identifier && expr.front().strvalue == name;
    }
};
namespace Interpreter {

    std::vector<MatchRule> rules;

    const std::string& safeBuffer(std::string& input) {
        if(input[0] != '(' || input[input.size() - 1] != ')') input = '(' + input + ')';
        return input;
    }

    void execute(expression& expr) {

        for (size_t i = 0; i < rules.size(); i++) {
            if(rules[i].match(expr)) expr = rules[i].expandRule(expr);   
        }

        for(auto& child : expr) {
            execute(child);
        }
    }

    void initialize() {
        rules.emplace_back(2,"include",[](expression& expr){
            std::string filename = (++expr.begin())->strvalue;
            std::ifstream f(filename);
            std::string buffer(std::istreambuf_iterator<char>(f), {});
            return expression::fromString(safeBuffer(buffer));
        });

        rules.emplace_back(3,"for",[](expression& expr){
            expression& args = expr.at(1);
            Interpreter::execute(args);
            expression result;
            return result;
        });
    }
};
int main(int argc, char** argv)
{
    std::string filename = argv[1];
    std::ifstream f(filename);
    std::string buffer(std::istreambuf_iterator<char>(f), {});
    expression rootExpr = expression::fromString(buffer,filename);
    Interpreter::initialize();
    Interpreter::execute(rootExpr);
    rootExpr.write(std::cout);
}
