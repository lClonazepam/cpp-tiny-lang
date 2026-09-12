#include "tiny/lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace tiny {

std::vector<Token> lex(const std::string& src) {
  static const std::unordered_map<std::string, Tok> kw{
      {"fn", Tok::KwFn}, {"let", Tok::KwLet}, {"if", Tok::KwIf},
      {"else", Tok::KwElse}, {"while", Tok::KwWhile}, {"return", Tok::KwReturn},
      {"print", Tok::KwPrint}};
  std::vector<Token> out;
  std::size_t i = 0;
  int line = 1;
  auto push = [&](Tok k, std::string t) { out.push_back({k, std::move(t), line}); };
  while (i < src.size()) {
    char c = src[i];
    if (c == '\n') { ++line; ++i; continue; }
    if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }
    if (c == '/' && i + 1 < src.size() && src[i + 1] == '/') {
      while (i < src.size() && src[i] != '\n') ++i;
      continue;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
      std::size_t j = i;
      while (j < src.size() && (std::isdigit(static_cast<unsigned char>(src[j])) || src[j] == '.')) ++j;
      push(Tok::Number, src.substr(i, j - i));
      i = j;
      continue;
    }
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      std::size_t j = i;
      while (j < src.size() && (std::isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) ++j;
      auto t = src.substr(i, j - i);
      auto it = kw.find(t);
      push(it == kw.end() ? Tok::Ident : it->second, t);
      i = j;
      continue;
    }
    auto two = src.substr(i, 2);
    if (two == "==") { push(Tok::EqEq, two); i += 2; continue; }
    if (two == "!=") { push(Tok::BangEq, two); i += 2; continue; }
    if (two == "<=") { push(Tok::LtEq, two); i += 2; continue; }
    if (two == ">=") { push(Tok::GtEq, two); i += 2; continue; }
    switch (c) {
      case '+': push(Tok::Plus, "+"); break;
      case '-': push(Tok::Minus, "-"); break;
      case '*': push(Tok::Star, "*"); break;
      case '/': push(Tok::Slash, "/"); break;
      case '<': push(Tok::Lt, "<"); break;
      case '>': push(Tok::Gt, ">"); break;
      case '=': push(Tok::Assign, "="); break;
      case '(': push(Tok::LParen, "("); break;
      case ')': push(Tok::RParen, ")"); break;
      case '{': push(Tok::LBrace, "{"); break;
      case '}': push(Tok::RBrace, "}"); break;
      case ',': push(Tok::Comma, ","); break;
      case ';': push(Tok::Semicolon, ";"); break;
      default: push(Tok::Unknown, std::string(1, c)); break;
    }
    ++i;
  }
  push(Tok::Eof, "");
  return out;
}

}  // namespace tiny
