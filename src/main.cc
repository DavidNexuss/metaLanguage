
#include <fstream>
#include <vector>
#include <functional>
#include <pstreams/pstream.h>
#include <map>
#include "expression.h"

using namespace std;
struct MatchRule {
    size_t fields;
    std::string name;
    std::function<expression(expression& expr)> expandRule;
    std::function<bool(const expression& expr)> specialMatching = [](const expression& expr){return true;};

    bool match(const expression& expr) {
        return expr.size() == fields && expr.front().type == ex_type::identifier && expr.front().strvalue == name;
    }
};

struct UserDefinedRules {
    std::string name;
    std::vector<std::string> arguments;
    expression templateExpression;

    bool match(const expression& expr) {
        return expr.size() == (arguments.size() + 1) && expr.front().type == ex_type::identifier && expr.front().strvalue == name;
    }

    expression expandRule(expression& expr) {

        expression result = templateExpression;
        size_t i = 1;
        for(const string& arg : arguments) {
            result.replace(arg,expr.at(i++));
        }

        result.write(std::cerr);
        return result;
    }
};
namespace Interpreter {

    std::vector<MatchRule> rules;
    std::vector<UserDefinedRules> userRules;
    std::map<std::string,expression> savedExpressions;

    const std::string& safeBuffer(std::string& input) {
        if(input[0] != '(' || input[input.size() - 1] != ')') input = '(' + input + ')';
        return input;
    }

    void execute(expression& expr) {

        for (size_t i = 0; i < rules.size(); i++) {
            if(rules[i].match(expr)) { 
                expr = rules[i].expandRule(expr);   
            }
        }

        for (size_t i = 0; i < userRules.size(); i++) {
            if(userRules[i].match(expr)) {
                expression result = userRules[i].expandRule(expr);
                execute(result);
                expr = result;
            }
        }
        for(auto& child : expr) {
            execute(child);
        }

        if(expr.size() == 1) expr = expr.at(0);
    }

    void initialize() {
        // include X
        rules.emplace_back(2,"include",[](expression& expr){
            std::string filename = (++expr.begin())->strvalue;
            std::ifstream f(filename);
            std::string buffer(std::istreambuf_iterator<char>(f), {});
            return expression::fromString(safeBuffer(buffer));
        });
        
        //for X L Expr
        rules.emplace_back(4,"for",[](expression& expr){
            expression& args = expr.at(2);
            Interpreter::execute(args);
            expression result;
            std::string pattern = expr.at(1).strvalue;

            for(auto& child : expr.at(2)) {
                expression newStatement = expr.at(3);
                newStatement.replace(pattern,child);
                result.push_back(newStatement);
            }
            return result;
        });

        rules.emplace_back(2,"exec",[](expression& expr){
            Interpreter::execute(expr.at(1));
            std::string command = expr.at(1).flat();
            redi::ipstream proc(command, redi::pstreams::pstdout | redi::pstreams::pstderr);
            std::string line;
            std::string buffer;
            while(getline(proc,line)) {
                buffer += line;
            }
            return expression::fromString("(" + buffer + ")");
        });

        rules.emplace_back(4,"define",[&](expression& expr){
            std::string commandName = expr.at(1).strvalue;
            userRules.emplace_back(commandName,expr.at(2).flatVector(),expr.at(3));
            return expression();
        });

        rules.emplace_back(2,"flat",[&](expression& expr){
            expression result = expr.at(1);
            Interpreter::execute(result);
            return expression::fromString("(" + result.flat() + ")");
        });

        rules.emplace_back(3,"set",[&](expression& expr){
            execute(expr.at(2));
            savedExpressions[expr.at(1).strvalue] = expr.at(2);
            return expression();
        });

        rules.emplace_back(2,"get",[&](expression& expr){
            return savedExpressions[expr.at(1).strvalue];
        });


        rules.emplace_back(3,"if",[&](expression& expr){
            execute(expr.at(1));
            if(expr.at(1).strvalue == "true") { 
                return expr.at(2);
            }
            return expression();
        });

        rules.emplace_back(3,"exists",[&](expression& expr){

            execute(expr.at(1));
            execute(expr.at(2));

            if(expr.at(2).exists(expr.at(1).strvalue)) return expression("true");
            return expression("false");
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
    std::cout << rootExpr.prettyFlat() << std::endl;
}
