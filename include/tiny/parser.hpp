#pragma once
#include "tiny/ast.hpp"
#include "tiny/token.hpp"
#include <vector>

namespace tiny {
Program parse(const std::vector<Token>& toks);
}  // namespace tiny
