%skeleton "lalr1.cc"
%define parser_class_name {meta_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.error verbose
%locations   // <--

%code requires
{
    #include <expression.h>

    using std::move;
    struct lexcontext;
}

%param {lexcontext& ctx }

%code
{
    struct lexcontext
    {
        const char* cursor;
        yy::location loc;

        expression rootExpression;
        void setRootExpression(expression&& expr) { rootExpression = std::move(expr); } 
    };
    
    namespace yy { meta_parser::symbol_type yylex(lexcontext& ctx); }
}

%token END 0
%token OPEN_BRACE "("
%token END_BRACE ")"
%token IDENTIFIER

%type <std::string> IDENTIFIER
%type <expression> expression 
%type <expression> statement
%type <expression> expression-list

%%
library : statement                              { ctx.setRootExpression(move($1)); }
statement : OPEN_BRACE expression-list END_BRACE { $$ = move($2); }
          ;

expression-list : expression                     { $$ = expression(); $$.push_back(move($1)); } 
expression-list : expression-list expression     { $$ = move($1); $$.push_back(move($2)); }

expression: statement                            { $$ = move($1); } 
          | IDENTIFIER                           { $$ = expression(move($1)); }
          ;

%%

yy::meta_parser::symbol_type yy::yylex(lexcontext& ctx)
{
    const char* anchor = ctx.cursor;
    ctx.loc.step();
    auto s = [&](auto func, auto&&... params) { ctx.loc.columns(ctx.cursor - anchor); return func(params..., ctx.loc); };
    %{
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = "char";
        re2c:define:YYCURSOR = "ctx.cursor";

        "\000"                  { return s(meta_parser::make_END); }

        "\r\n" | [\r\n]         { ctx.loc.lines();   return yylex(ctx); }
        "//" [^\r\n]*           {                    return yylex(ctx); }
        [\t\v\b\f ]             { ctx.loc.columns(); return yylex(ctx); }


        "(" { return s(meta_parser::make_OPEN_BRACE); } 
        ")" { return s(meta_parser::make_END_BRACE); }


        [a-zA-Z_] [a-zA-Z_0-9]* { return s(meta_parser::make_IDENTIFIER,std::string(anchor,ctx.cursor)); } 
    %}
}

void yy::meta_parser::error(const location_type& l, const std::string& m)
{
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << '\n'; 
}

#include <fstream>
int main(int argc, char** argv)
{
    std::string filename = argv[1];
    std::ifstream f(filename);
    std::string buffer(std::istreambuf_iterator<char>(f), {});
    std::cout << "Input text: " << buffer << std::endl;
    lexcontext ctx;
    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &filename;
    ctx.loc.end.filename   = &filename;

    yy::meta_parser parser(ctx);
    parser.parse();
    ctx.rootExpression.print();
}
