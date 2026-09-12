#include "tiny/vm.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace tiny {

namespace {
std::uint16_t read_u16(const Chunk& ch, std::size_t& ip) {
  auto lo = ch.code[ip++];
  auto hi = ch.code[ip++];
  return static_cast<std::uint16_t>(lo | (hi << 8));
}
}  // namespace

double run(const Module& m, const std::string& entry) {
  struct Frame {
    const Chunk* ch;
    std::size_t ip;
    std::size_t bp;
  };
  auto it = m.fns.find(entry);
  if (it == m.fns.end()) throw std::runtime_error("no entry");
  std::vector<double> stack;
  std::vector<Frame> frames;
  std::unordered_map<std::string, double> globals;
  frames.push_back({&it->second, 0, 0});

  auto load_name = [&](const Chunk& ch, std::uint8_t idx) -> double& {
    return globals[ch.names[idx]];
  };

  while (!frames.empty()) {
    auto& fr = frames.back();
    const Chunk& ch = *fr.ch;
    if (fr.ip >= ch.code.size()) {
      frames.pop_back();
      continue;
    }
    auto op = static_cast<Op>(ch.code[fr.ip++]);
    switch (op) {
      case Op::Const: stack.push_back(ch.consts[ch.code[fr.ip++]]); break;
      case Op::Load: stack.push_back(load_name(ch, ch.code[fr.ip++])); break;
      case Op::Store: {
        auto idx = ch.code[fr.ip++];
        load_name(ch, idx) = stack.back();
        stack.pop_back();
        break;
      }
      case Op::Add: { auto b = stack.back(); stack.pop_back(); stack.back() += b; break; }
      case Op::Sub: { auto b = stack.back(); stack.pop_back(); stack.back() -= b; break; }
      case Op::Mul: { auto b = stack.back(); stack.pop_back(); stack.back() *= b; break; }
      case Op::Div: { auto b = stack.back(); stack.pop_back(); stack.back() /= b; break; }
      case Op::Eq: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() == b; break; }
      case Op::Ne: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() != b; break; }
      case Op::Lt: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() < b; break; }
      case Op::Gt: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() > b; break; }
      case Op::Le: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() <= b; break; }
      case Op::Ge: { auto b = stack.back(); stack.pop_back(); stack.back() = stack.back() >= b; break; }
      case Op::Jmp: fr.ip = read_u16(ch, fr.ip); break;
      case Op::Jz: {
        auto dest = read_u16(ch, fr.ip);
        auto c = stack.back();
        stack.pop_back();
        if (c == 0) fr.ip = dest;
        break;
      }
      case Op::Call: {
        auto nidx = ch.code[fr.ip++];
        auto argc = ch.code[fr.ip++];
        auto name = ch.names[nidx];
        auto fit = m.fns.find(name);
        if (fit == m.fns.end()) throw std::runtime_error("unknown fn " + name);
        std::vector<double> args;
        for (int i = 0; i < argc; ++i) {
          args.push_back(stack.back());
          stack.pop_back();
        }
        std::reverse(args.begin(), args.end());
        frames.push_back({&fit->second, 0, stack.size()});
        for (int i = 0; i < argc; ++i) {
          globals[fit->second.names[static_cast<std::size_t>(i)]] = args[static_cast<std::size_t>(i)];
        }
        break;
      }
      case Op::Ret: {
        double v = stack.empty() ? 0 : stack.back();
        if (!stack.empty()) stack.pop_back();
        frames.pop_back();
        stack.push_back(v);
        break;
      }
      case Op::Print:
        std::cout << stack.back() << "\n";
        stack.pop_back();
        break;
      case Op::Pop:
        if (!stack.empty()) stack.pop_back();
        break;
    }
  }
  return stack.empty() ? 0 : stack.back();
}

}  // namespace tiny
