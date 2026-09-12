#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace tiny {

enum class Op : std::uint8_t {
  Const, Load, Store, Add, Sub, Mul, Div,
  Eq, Ne, Lt, Gt, Le, Ge,
  Jmp, Jz, Call, Ret, Print, Pop
};

struct Chunk {
  std::vector<std::uint8_t> code;
  std::vector<double> consts;
  std::vector<std::string> names;
  int arity{0};
};

}  // namespace tiny
