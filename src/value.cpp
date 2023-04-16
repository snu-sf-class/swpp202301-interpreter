#include "value.h"


Value::Value(Reg _reg): kind(true), reg(_reg), literal(0) {}

Value::Value(uint64_t _literal): kind(false), reg(RegNone), literal(_literal) {}

pair<uint64_t, double> Value::get_value(RegFile& regfile) const {
  if (kind)
    return regfile.read_reg(reg);
  else
    return make_pair(literal, -1.0);
}