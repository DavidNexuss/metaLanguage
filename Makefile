all: bin/parser
obj/parser.cc.re: src/parser.yy
		bison src/parser.yy -o obj/parser.cc.re -Wall

obj/parser.cc: obj/parser.cc.re
		re2c obj/parser.cc.re -o obj/parser.cc

bin/parser: obj/parser.cc src/main.cc
		g++ -Wall $^ -std=c++20 -o bin/parser -I src -O3

clean: 
	rm -f obj/parser.cc.re obj/parser.cc bin/parser
