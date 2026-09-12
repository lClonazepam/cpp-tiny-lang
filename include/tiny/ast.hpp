#pragma once
#include <memory>
#include <string>
#include <vector>

namespace tiny {

struct Expr {
  virtual ~Expr() = default;
};
struct NumberExpr : Expr { double value{}; };
struct VarExpr : Expr { std::string name; };
struct BinaryExpr : Expr {
  char op{};
  std::unique_ptr<Expr> lhs, rhs;
};
struct CallExpr : Expr {
  std::string name;
  std::vector<std::unique_ptr<Expr>> args;
};

struct Stmt {
  virtual ~Stmt() = default;
};
struct LetStmt : Stmt {
  std::string name;
  std::unique_ptr<Expr> init;
};
struct AssignStmt : Stmt {
  std::string name;
  std::unique_ptr<Expr> value;
};
struct ExprStmt : Stmt { std::unique_ptr<Expr> expr; };
struct PrintStmt : Stmt { std::unique_ptr<Expr> expr; };
struct ReturnStmt : Stmt { std::unique_ptr<Expr> expr; };
struct BlockStmt : Stmt { std::vector<std::unique_ptr<Stmt>> stmts; };
struct IfStmt : Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> then_b;
  std::unique_ptr<Stmt> else_b;
};
struct WhileStmt : Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> body;
};
struct FnDecl {
  std::string name;
  std::vector<std::string> params;
  std::unique_ptr<BlockStmt> body;
};
struct Program {
  std::vector<FnDecl> fns;
};

}  // namespace tiny
