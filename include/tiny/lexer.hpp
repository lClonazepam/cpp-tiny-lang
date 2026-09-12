#pragma once
#include "tiny/token.hpp"
#include <string>
#include <vector>

namespace tiny {
std::vector<Token> lex(const std::string& src);
}  // namespace tiny
