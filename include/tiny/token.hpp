#pragma once
#include <string>

namespace tiny {

enum class Tok {
  Eof, Number, Ident,
  Plus, Minus, Star, Slash,
  EqEq, BangEq, Lt, Gt, LtEq, GtEq,
  Assign, LParen, RParen, LBrace, RBrace,
  Comma, Semicolon,
  KwFn, KwLet, KwIf, KwElse, KwWhile, KwReturn, KwPrint,
  Unknown
};

struct Token {
  Tok kind{};
  std::string text;
  int line{1};
};

}  // namespace tiny
