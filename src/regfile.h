#ifndef SWPP_ASM_INTERPRETER_REGFILE_H
#define SWPP_ASM_INTERPRETER_REGFILE_H

#include <cinttypes>
#include <string>
#include <utility>

#include "reg.h"

using namespace std;


class RegFile {
private:
  uint64_t regfile[NREGS];
  double async[NREGS];
  int nargs;

  double resolve_async(Reg reg);

public:
  RegFile();

  static bool is_writable(Reg reg) {
    return (R1 <= reg && reg <= R32) || (reg == RegSp);
  }

  void set_nargs(int _nargs);
  void set_value(Reg reg, uint64_t val);
  pair<uint64_t, double> read_reg(Reg reg);
  void write_reg(Reg reg, uint64_t val);
  void set_async(Reg reg, double cost);
  string to_string() const;
};

#endif //SWPP_ASM_INTERPRETER_REGFILE_H
