#include "tiny/compiler.hpp"
#include "tiny/lexer.hpp"
#include "tiny/parser.hpp"
#include "tiny/vm.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
  std::string src;
  if (argc > 1) {
    std::ifstream in(argv[1]);
    std::ostringstream ss;
    ss << in.rdbuf();
    src = ss.str();
  } else {
    src = R"(
fn fib(n) {
  if (n < 2) { return n; }
  return fib(n - 1) + fib(n - 2);
}
fn main() {
  let i = 0;
  while (i < 10) {
    print fib(i);
    i = i + 1;
  }
  return 0;
}
)";
  }
  auto toks = tiny::lex(src);
  auto ast = tiny::parse(toks);
  auto mod = tiny::compile(ast);
  tiny::run(mod, "main");
}
