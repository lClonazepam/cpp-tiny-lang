#include "tiny/parser.hpp"
#include <stdexcept>

namespace tiny {
namespace {

struct P {
  const std::vector<Token>& t;
  std::size_t i{0};
  const Token& peek() const { return t[i]; }
  const Token& get() { return t[i++]; }
  bool match(Tok k) {
    if (peek().kind == k) { ++i; return true; }
    return false;
  }
  void expect(Tok k, const char* msg) {
    if (!match(k)) throw std::runtime_error(std::string(msg) + " at line " + std::to_string(peek().line));
  }

  std::unique_ptr<Expr> expr() { return cmp();
  }
  std::unique_ptr<Expr> cmp() {
    auto e = add();
    for (;;) {
      char op = 0;
      if (match(Tok::EqEq)) op = '=';
      else if (match(Tok::BangEq)) op = '!';
      else if (match(Tok::Lt)) op = '<';
      else if (match(Tok::Gt)) op = '>';
      else if (match(Tok::LtEq)) op = 'l';
      else if (match(Tok::GtEq)) op = 'g';
      else break;
      auto b = std::make_unique<BinaryExpr>();
      b->op = op;
      b->lhs = std::move(e);
      b->rhs = add();
      e = std::move(b);
    }
    return e;
  }
  std::unique_ptr<Expr> add() {
    auto e = mul();
    while (peek().kind == Tok::Plus || peek().kind == Tok::Minus) {
      char op = get().text[0];
      auto b = std::make_unique<BinaryExpr>();
      b->op = op;
      b->lhs = std::move(e);
      b->rhs = mul();
      e = std::move(b);
    }
    return e;
  }
  std::unique_ptr<Expr> mul() {
    auto e = primary();
    while (peek().kind == Tok::Star || peek().kind == Tok::Slash) {
      char op = get().text[0];
      auto b = std::make_unique<BinaryExpr>();
      b->op = op;
      b->lhs = std::move(e);
      b->rhs = primary();
      e = std::move(b);
    }
    return e;
  }
  std::unique_ptr<Expr> primary() {
    if (match(Tok::Number)) {
      auto n = std::make_unique<NumberExpr>();
      n->value = std::stod(t[i - 1].text);
      return n;
    }
    if (peek().kind == Tok::Ident) {
      auto name = get().text;
      if (match(Tok::LParen)) {
        auto c = std::make_unique<CallExpr>();
        c->name = name;
        if (!match(Tok::RParen)) {
          do {
            c->args.push_back(expr());
          } while (match(Tok::Comma));
          expect(Tok::RParen, "expected )");
        }
        return c;
      }
      auto v = std::make_unique<VarExpr>();
      v->name = name;
      return v;
    }
    if (match(Tok::LParen)) {
      auto e = expr();
      expect(Tok::RParen, "expected )");
      return e;
    }
    throw std::runtime_error("expected expression at line " + std::to_string(peek().line));
  }

  std::unique_ptr<Stmt> stmt() {
    if (match(Tok::KwLet)) {
      auto s = std::make_unique<LetStmt>();
      if (peek().kind != Tok::Ident) throw std::runtime_error("let name");
      s->name = get().text;
      expect(Tok::Assign, "let =");
      s->init = expr();
      expect(Tok::Semicolon, ";");
      return s;
    }
    if (match(Tok::KwPrint)) {
      auto s = std::make_unique<PrintStmt>();
      s->expr = expr();
      expect(Tok::Semicolon, ";");
      return s;
    }
    if (match(Tok::KwReturn)) {
      auto s = std::make_unique<ReturnStmt>();
      s->expr = expr();
      expect(Tok::Semicolon, ";");
      return s;
    }
    if (match(Tok::KwIf)) {
      auto s = std::make_unique<IfStmt>();
      expect(Tok::LParen, "(");
      s->cond = expr();
      expect(Tok::RParen, ")");
      s->then_b = stmt();
      if (match(Tok::KwElse)) s->else_b = stmt();
      return s;
    }
    if (match(Tok::KwWhile)) {
      auto s = std::make_unique<WhileStmt>();
      expect(Tok::LParen, "(");
      s->cond = expr();
      expect(Tok::RParen, ")");
      s->body = stmt();
      return s;
    }
    if (peek().kind == Tok::LBrace) return block();
    if (peek().kind == Tok::Ident && t[i + 1].kind == Tok::Assign) {
      auto s = std::make_unique<AssignStmt>();
      s->name = get().text;
      expect(Tok::Assign, "=");
      s->value = expr();
      expect(Tok::Semicolon, ";");
      return s;
    }
    auto s = std::make_unique<ExprStmt>();
    s->expr = expr();
    expect(Tok::Semicolon, ";");
    return s;
  }

  std::unique_ptr<BlockStmt> block() {
    expect(Tok::LBrace, "{");
    auto b = std::make_unique<BlockStmt>();
    while (peek().kind != Tok::RBrace && peek().kind != Tok::Eof) {
      b->stmts.push_back(stmt());
    }
    expect(Tok::RBrace, "}");
    return b;
  }

  FnDecl function() {
    expect(Tok::KwFn, "fn");
    FnDecl f;
    if (peek().kind != Tok::Ident) throw std::runtime_error("fn name");
    f.name = get().text;
    expect(Tok::LParen, "(");
    if (!match(Tok::RParen)) {
      do {
        if (peek().kind != Tok::Ident) throw std::runtime_error("param");
        f.params.push_back(get().text);
      } while (match(Tok::Comma));
      expect(Tok::RParen, ")");
    }
    f.body = block();
    return f;
  }
};

}  // namespace

Program parse(const std::vector<Token>& toks) {
  P p{toks};
  Program prog;
  while (p.peek().kind != Tok::Eof) prog.fns.push_back(p.function());
  return prog;
}

}  // namespace tiny
