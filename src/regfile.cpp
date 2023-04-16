#include <sstream>

#include "error.h"
#include "regfile.h"
#include "memory.h"

using namespace std;


RegFile::RegFile(): nargs(0) {
  for (uint64_t& i: regfile)
    i = 0;
  for (double& c: async)
    c = -1.0;
  regfile[RegSp] = STACK_MAX;
}

void RegFile::set_nargs(int _nargs) { nargs = _nargs; }

void RegFile::set_value(Reg reg, uint64_t val) {
  if (reg == RegNone)
    return;
  regfile[reg] = val;
}

double RegFile::resolve_async(Reg reg) {
  if (!is_writable(reg))
    return -1.0;
  double wait_until = async[reg];
  async[reg] = -1.0;
  return wait_until;
}

pair<uint64_t, double> RegFile::read_reg(Reg reg) {
  if (reg == RegNone)
    invoke_runtime_error("reading an unknown register");
  if ((int)A1 + nargs <= reg && reg <= A16)
    invoke_runtime_error("reading out-of-range argument");
  return make_pair(regfile[reg], this->resolve_async(reg));
}

void RegFile::write_reg(Reg reg, uint64_t val) {
  if (reg == RegNone)
    return;
  if (A1 <= reg && reg <= A16)
    invoke_runtime_error("writing to a read-only register");
  resolve_async(reg);
  regfile[reg] = val;
}

void RegFile::set_async(Reg reg, double cost) {
  if (reg == RegNone)
    return;
  if (A1 <= reg && reg <= A16)
    invoke_runtime_error("writing to a read-only register");
  if (is_writable(reg) && async[reg] >= 0)
    invoke_runtime_error("writing to a register that is waiting for async load to be resolved");
  async[reg] = cost;
}

string RegFile::to_string() const {
  stringstream ss;
  for (int i = 0; i < 16; i++)
    ss << "r" << (i + 1) << "[" << regfile[i] << "]" << " ";
  for (int i = 16; i < 32; i++)
    ss << "arg" << (i - 15) << "[" << regfile[i] << "]" << " ";
  ss << "sp[" << regfile[32] << "]";

  return ss.str();
}
