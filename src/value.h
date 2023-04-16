#ifndef SWPP_ASM_INTERPRETER_VALUE_H
#define SWPP_ASM_INTERPRETER_VALUE_H

#include <cinttypes>
#include <utility>

#include "regfile.h"


class Value {
private:
  const bool kind;
  const Reg reg;
  const uint64_t literal;

public:
  explicit Value(Reg _reg);
  explicit Value(uint64_t _literal);

  pair<uint64_t, double> get_value(RegFile& regfile) const;
};

#endif //SWPP_ASM_INTERPRETER_VALUE_H
