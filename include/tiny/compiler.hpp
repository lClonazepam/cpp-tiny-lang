#pragma once
#include "tiny/ast.hpp"
#include "tiny/opcode.hpp"
#include <string>
#include <unordered_map>

namespace tiny {

struct Module {
  std::unordered_map<std::string, Chunk> fns;
};

Module compile(const Program& p);

}  // namespace tiny
