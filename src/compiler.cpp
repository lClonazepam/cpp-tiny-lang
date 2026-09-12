#include "tiny/compiler.hpp"
#include <stdexcept>

namespace tiny {
namespace {

struct C {
  Chunk ch;
  std::unordered_map<std::string, std::uint8_t> locals;

  void emit(Op op) { ch.code.push_back(static_cast<std::uint8_t>(op)); }
  void emit_u8(std::uint8_t b) { ch.code.push_back(b); }
  std::size_t emit_jmp(Op op) {
    emit(op);
    auto at = ch.code.size();
    emit_u8(0);
    emit_u8(0);
    return at;
  }
  void patch(std::size_t at) {
    auto dest = static_cast<std::uint16_t>(ch.code.size());
    ch.code[at] = dest & 0xff;
    ch.code[at + 1] = dest >> 8;
  }
  std::uint8_t intern_name(const std::string& n) {
    for (std::size_t i = 0; i < ch.names.size(); ++i)
      if (ch.names[i] == n) return static_cast<std::uint8_t>(i);
    ch.names.push_back(n);
    return static_cast<std::uint8_t>(ch.names.size() - 1);
  }
  std::uint8_t intern_const(double v) {
    ch.consts.push_back(v);
    return static_cast<std::uint8_t>(ch.consts.size() - 1);
  }

  void expr(const Expr* e) {
    if (auto n = dynamic_cast<const NumberExpr*>(e)) {
      emit(Op::Const);
      emit_u8(intern_const(n->value));
    } else if (auto v = dynamic_cast<const VarExpr*>(e)) {
      emit(Op::Load);
      emit_u8(intern_name(v->name));
    } else if (auto b = dynamic_cast<const BinaryExpr*>(e)) {
      expr(b->lhs.get());
      expr(b->rhs.get());
      switch (b->op) {
        case '+': emit(Op::Add); break;
        case '-': emit(Op::Sub); break;
        case '*': emit(Op::Mul); break;
        case '/': emit(Op::Div); break;
        case '=': emit(Op::Eq); break;
        case '!': emit(Op::Ne); break;
        case '<': emit(Op::Lt); break;
        case '>': emit(Op::Gt); break;
        case 'l': emit(Op::Le); break;
        case 'g': emit(Op::Ge); break;
      }
    } else if (auto c = dynamic_cast<const CallExpr*>(e)) {
      for (auto& a : c->args) expr(a.get());
      emit(Op::Call);
      emit_u8(intern_name(c->name));
      emit_u8(static_cast<std::uint8_t>(c->args.size()));
    }
  }

  void stmt(const Stmt* s) {
    if (auto l = dynamic_cast<const LetStmt*>(s)) {
      expr(l->init.get());
      emit(Op::Store);
      emit_u8(intern_name(l->name));
    } else if (auto a = dynamic_cast<const AssignStmt*>(s)) {
      expr(a->value.get());
      emit(Op::Store);
      emit_u8(intern_name(a->name));
    } else if (auto p = dynamic_cast<const PrintStmt*>(s)) {
      expr(p->expr.get());
      emit(Op::Print);
    } else if (auto r = dynamic_cast<const ReturnStmt*>(s)) {
      expr(r->expr.get());
      emit(Op::Ret);
    } else if (auto e = dynamic_cast<const ExprStmt*>(s)) {
      expr(e->expr.get());
      emit(Op::Pop);
    } else if (auto b = dynamic_cast<const BlockStmt*>(s)) {
      for (auto& x : b->stmts) stmt(x.get());
    } else if (auto i = dynamic_cast<const IfStmt*>(s)) {
      expr(i->cond.get());
      auto jz = emit_jmp(Op::Jz);
      stmt(i->then_b.get());
      if (i->else_b) {
        auto jmp = emit_jmp(Op::Jmp);
        patch(jz);
        stmt(i->else_b.get());
        patch(jmp);
      } else {
        patch(jz);
      }
    } else if (auto w = dynamic_cast<const WhileStmt*>(s)) {
      auto start = static_cast<std::uint16_t>(ch.code.size());
      expr(w->cond.get());
      auto jz = emit_jmp(Op::Jz);
      stmt(w->body.get());
      emit(Op::Jmp);
      emit_u8(start & 0xff);
      emit_u8(start >> 8);
      patch(jz);
    }
  }
};

}  // namespace

Module compile(const Program& p) {
  Module m;
  for (auto& f : p.fns) {
    C c;
    c.ch.arity = static_cast<int>(f.params.size());
    for (auto& prm : f.params) c.intern_name(prm);
    c.stmt(f.body.get());
    c.emit(Op::Const);
    c.emit_u8(c.intern_const(0));
    c.emit(Op::Ret);
    m.fns[f.name] = std::move(c.ch);
  }
  if (!m.fns.count("main")) throw std::runtime_error("missing fn main");
  return m;
}

}  // namespace tiny
