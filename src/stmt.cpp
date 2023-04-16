#include <iostream>

#include "stmt.h"
#include "error.h"


Stmt::Stmt(int _line, Reg _lhs, Opcode _opcode): line(_line), lhs(_lhs), opcode(_opcode), next(nullptr) {}

int Stmt::get_line() const { return line; }

Reg Stmt::get_lhs() const { return lhs; }

Opcode Stmt::get_opcode() const { return opcode; }

Stmt *Stmt::get_next() const { return next; }

void Stmt::set_next(Stmt *stmt) { next = stmt; }

double get_wait_cost(double cost_acc, double wait_until) {
  return cost_acc >= wait_until ? 0 : wait_until - cost_acc;
}


/** terminators */

StmtRet::StmtRet(int _line, Value _val): Stmt(_line, RegNone, Ret), val(_val) {}

pair<uint64_t, double> StmtRet::get_val(double cost_acc, RegFile &regfile) const {
  auto ret = val.get_value(regfile);
  return make_pair(ret.first, get_wait_cost(cost_acc, ret.second));
}

pair<double, double> StmtRet::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  return make_pair(0, 0);
}

StmtBrUncond::StmtBrUncond(int _line, string _bb): Stmt(_line, RegNone, BrUncond), bb(move(_bb)) {}

string StmtBrUncond::get_bb() const { return bb; }

pair<double, double> StmtBrUncond::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  return make_pair(0, 0);
}

StmtBrCond::StmtBrCond(int _line, Value _cond, string _true_bb, string _false_bb):
Stmt(_line, RegNone, BrCond), cond(_cond), true_bb(move(_true_bb)), false_bb(move(_false_bb)) {}

pair<string, double> StmtBrCond::get_bb(double cost_acc, RegFile& regfile) {
  auto c = cond.get_value(regfile);
  if (c.first != 0) {
    eval = true;
    return make_pair(true_bb, get_wait_cost(cost_acc, c.second));
  } else {
    eval = false;
    return make_pair(false_bb, get_wait_cost(cost_acc, c.second));
  }
}

bool StmtBrCond::get_eval() const { return eval; }

pair<double, double> StmtBrCond::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  return make_pair(0, 0);
}

StmtSwitch::StmtSwitch(int _line, Value _cond): Stmt(_line, RegNone, Switch), cond(_cond) {}

bool StmtSwitch::set_bb(uint64_t val, string bb) {
  auto it = bb_map.find(val);
  if (it != bb_map.end())
    return false;
  bb_map.insert(pair<uint64_t, string>(val, move(bb)));
  return true;
}

pair<string, double> StmtSwitch::get_bb(double cost_acc, RegFile& regfile) const {
  auto c = cond.get_value(regfile);
  auto it = bb_map.find(c.first);
  if (it == bb_map.end())
    return make_pair(default_bb, get_wait_cost(cost_acc, c.second));
  return make_pair(it->second, get_wait_cost(cost_acc, c.second));
}

void StmtSwitch::set_default(string bb) { default_bb = move(bb); }

bool StmtSwitch::case_exists(uint64_t val) const {
  auto it = bb_map.find(val);
  return it != bb_map.end();
}

pair<double, double> StmtSwitch::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  return make_pair(0, 0);
}


/** memory operations */

StmtMalloc::StmtMalloc(int _line, Reg _lhs, Value _val): Stmt(_line, _lhs, Malloc), val(_val) {}

pair<double, double> StmtMalloc::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto size = val.get_value(regfile);
  uint64_t addr;
  double cost = memory.exec_malloc(size.first, addr);
  regfile.write_reg(get_lhs(), addr);
  return make_pair(cost, get_wait_cost(cost_acc, size.second));
}

StmtFree::StmtFree(int _line, Value _ptr): Stmt(_line, RegNone, Free), ptr(_ptr) {}

pair<double, double> StmtFree::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto addr = ptr.get_value(regfile);
  return make_pair(memory.exec_free(addr.first), get_wait_cost(cost_acc, addr.second));
}

StmtLoad::StmtLoad(int _line, Reg _lhs, bool _is_async, MSize _size, Value _ptr, uint64_t _ofs):
Stmt(_line, _lhs, Load), is_async(_is_async), size(_size), ptr(_ptr), ofs(_ofs) {}

pair<double, double> StmtLoad::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto res = ptr.get_value(regfile);
  uint64_t addr = res.first + ofs;
  uint64_t result;
  double cost = memory.exec_load(is_async, size, addr, result);
  double wait_cost = get_wait_cost(cost_acc, res.second);
  regfile.write_reg(get_lhs(), result);

  if (is_async) {
    if (is_stack(size, addr)) {
      regfile.set_async(get_lhs(), cost_acc + wait_cost + CurrentMachine->machine_cost->ALOAD + CurrentMachine->machine_cost->WAIT_STACK);
    }
    else if (is_heap(size,addr)) {
      regfile.set_async(get_lhs(), cost_acc + wait_cost + CurrentMachine->machine_cost->ALOAD + CurrentMachine->machine_cost->WAIT_HEAP);
    }
    else
      invoke_runtime_error("accessing address between 10248 and 20480");
  }

  return make_pair(cost, wait_cost);
}

StmtStore::StmtStore(int _line, MSize _size, Value _val, Value _ptr, uint64_t _ofs):
Stmt(_line, RegNone, Store), size(_size), val(_val), ptr(_ptr), ofs(_ofs) {}

pair<double, double> StmtStore::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto res = ptr.get_value(regfile);
  uint64_t addr = res.first + ofs;
  auto v = val.get_value(regfile);
  double wait_cost = max(get_wait_cost(cost_acc, res.second), get_wait_cost(cost_acc, v.second));
  return make_pair(memory.exec_store(size, addr, v.first), wait_cost);
}


/** binary operations */

StmtBop::StmtBop(int _line, Reg _lhs, BopKind _bop_kind, Value _val1, Value _val2, Size _size):
Stmt(_line, _lhs, Bop), bop_kind(_bop_kind), val1(_val1), val2(_val2), size(_size) {}

bool is_signed_op(BopKind bop_kind) {
  switch(bop_kind) {
    case Udiv:
    case Urem:
    case Mul:
    case Shl:
    case Lshr:
    case And:
    case Or:
    case Xor:
    case Add:
    case Sub:
    case Eq:
    case Ne:
    case Ugt:
    case Uge:
    case Ult:
    case Ule:
      return false;
    case Ashr:
    case Sdiv:
    case Srem:
    case Sgt:
    case Sge:
    case Slt:
    case Sle:
      return true;
  }
}

bool is_shift_op(BopKind bop_kind) {
  switch(bop_kind) {
    case Shl:
    case Lshr:
    case Ashr:
      return true;
    default:
      return false;
  }
}

uint64_t get_op1(BopKind bop_kind, Size size, uint64_t val) {
  if (is_signed_op(bop_kind)) {
    switch (size) {
      case Size1:
        if (val % 2 == 1)
          return -1;
        else
          return 0;
      case Size8:
        return (int8_t)val;
      case Size16:
        return (int16_t)val;
      case Size32:
        return (int32_t)val;
      case Size64:
        return val;
    }
  }
  else {
    switch (size) {
      case Size1:
        if (val % 2 == 1)
          return 1;
        else
          return 0;
      case Size8:
        return (uint8_t)val;
      case Size16:
        return (uint16_t)val;
      case Size32:
        return (uint32_t)val;
      case Size64:
        return val;
    }
  }
}

uint64_t get_op2(BopKind bop_kind, Size size, uint64_t val) {
  if (is_shift_op(bop_kind)) {
    return val % bw_of(size);
  }

  return get_op1(bop_kind, size, val);
}

uint64_t get_result(Size size, uint64_t val) {
  switch (size) {
    case Size1:
      return val % 2;
    case Size8:
      return (uint8_t)val;
    case Size16:
      return (uint16_t)val;
    case Size32:
      return (uint32_t)val;
    case Size64:
      return val;
  }
}

uint64_t StmtBop::compute(uint64_t op1, uint64_t op2) const {
  op1 = get_op1(bop_kind, size, op1);
  op2 = get_op2(bop_kind, size, op2);
  uint64_t result = 0;

  switch (bop_kind) {
    case Udiv:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = op1 / op2;
      break;
    case Sdiv:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = (int64_t) op1 / (int64_t) op2;
      break;
    case Urem:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = op1 % op2;
      break;
    case Srem:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = (int64_t) op1 % (int64_t) op2;
      break;
    case Mul:
      result = op1 * op2;
      break;
    case Shl:
      result = op1 << op2;
      break;
    case Lshr:
      result = op1 >> op2;
      break;
    case Ashr:
      result = (int64_t) op1 >> op2;
      break;
    case And:
      result = op1 & op2;
      break;
    case Or:
      result = op1 | op2;
      break;
    case Xor:
      result = op1 ^ op2;
      break;
    case Add:
      result = op1 + op2;
      break;
    case Sub:
      result = op1 - op2;
      break;
    case Eq:
      if (op1 == op2)
        result = 1;
      else
        result = 0;
      break;
    case Ne:
      if (op1 != op2)
        result = 1;
      else
        result = 0;
      break;
    case Ugt:
      if (op1 > op2)
        result = 1;
      else
        result = 0;
      break;
    case Uge:
      if (op1 >= op2)
        result = 1;
      else
        result = 0;
      break;
    case Ult:
      if (op1 < op2)
        result = 1;
      else
        result = 0;
      break;
    case Ule:
      if (op1 <= op2)
        result = 1;
      else
        result = 0;
      break;
    case Sgt:
      if ((int64_t) op1 > (int64_t) op2)
        result = 1;
      else
        result = 0;
      break;
    case Sge:
      if ((int64_t) op1 >= (int64_t) op2)
        result = 1;
      else
        result = 0;
      break;
    case Slt:
      if ((int64_t) op1 < (int64_t) op2)
        result = 1;
      else
        result = 0;
      break;
    case Sle:
      if ((int64_t) op1 <= (int64_t) op2)
        result = 1;
      else
        result = 0;
      break;
  }

  return get_result(size, result);
}

double cost_of(BopKind bop_kind) {
  switch (bop_kind) {
    case Udiv:
    case Sdiv:
    case Urem:
    case Srem:
    case Mul:
      return CurrentMachine->machine_cost->MULDIV;
    case Shl:
    case Lshr:
    case Ashr:
    case And:
    case Or:
    case Xor:
      return CurrentMachine->machine_cost->LOGICAL;
    case Add:
    case Sub:
      return CurrentMachine->machine_cost->ADDSUB;
    case Eq:
    case Ne:
    case Ugt:
    case Uge:
    case Ult:
    case Ule:
    case Sgt:
    case Sge:
    case Slt:
    case Sle:
      return CurrentMachine->machine_cost->COMP;
  }
}

pair<double, double> StmtBop::exec(double cost_acc, RegFile& regfile, Memory& memory) const {
  auto op1 = val1.get_value(regfile);
  auto op2 = val2.get_value(regfile);
  uint64_t res = compute(op1.first, op2.first);
  regfile.write_reg(get_lhs(), res);
  double wait_cost = max(get_wait_cost(cost_acc, op1.second), get_wait_cost(cost_acc, op2.second));
  return make_pair(cost_of(bop_kind), wait_cost);
}


/** sum operation */

StmtSum::StmtSum(int _line, Reg _lhs, const vector<Value> &_values, Size _size):
Stmt(_line, _lhs, Sum), size(_size) {
  values.reserve(num_operands);
  for (int i = 0; i < num_operands; i++) {
    values.emplace_back(_values.at(i));
  }
}

pair<double, double> StmtSum::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  uint64_t res = 0;
  double wait_until = -1.0;
  for (auto value: values) {
    auto v = value.get_value(regfile);
    res += v.first;
    if (v.second > wait_until)
      wait_until = v.second;
  }
  regfile.write_reg(get_lhs(), res);
  return make_pair(CurrentMachine->machine_cost->SUM, get_wait_cost(cost_acc, wait_until));
}


/** unary operations */

StmtUop::StmtUop(int _line, Reg _lhs, UopKind _uop_kind, Value _val, Size _size):
Stmt(_line, _lhs, Uop), uop_kind(_uop_kind), val(_val), size(_size) {}

pair<double, double> StmtUop::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto op = val.get_value(regfile);
  uint64_t res = op.first;
  if (uop_kind == UopKind::Incr)
    res++;
  else
    res--;
  res = get_result(size, res);
  regfile.write_reg(get_lhs(), res);
  return make_pair(CurrentMachine->machine_cost->UOP, get_wait_cost(cost_acc, op.second));
}


/** ternary operation */

StmtSelect::StmtSelect(int _line, Reg _lhs, Value _cond, Value _val_true, Value _val_false):
Stmt(_line, _lhs, Select), cond(_cond), val_true(_val_true), val_false(_val_false) {}

pair<double, double> StmtSelect::exec(double cost_acc, RegFile& regfile, Memory& memory) const {
  auto v_cond = cond.get_value(regfile);
  auto v_true = val_true.get_value(regfile);
  auto v_false = val_false.get_value(regfile);

  double wait_until = v_cond.second;

  if (v_cond.first != 0) {
    if (v_true.second > wait_until)
      wait_until = v_true.second;
    regfile.write_reg(get_lhs(), v_true.first);
  }
  else {
    if (v_false.second > wait_until)
      wait_until = v_false.second;
    regfile.write_reg(get_lhs(), v_false.first);
  }

  return make_pair(CurrentMachine->machine_cost->TERNARY, get_wait_cost(cost_acc, wait_until));
}


/** function call */

StmtCall::StmtCall(int _line, Reg _lhs, string _fname): Stmt(_line, _lhs, Call), fname(move(_fname)) {}

string StmtCall::get_fname() const { return fname; }

void StmtCall::push_arg(const Value arg) { args.push_back(arg); }

int StmtCall::get_nargs() { return args.size(); }

double StmtCall::setup_args(double cost_acc, RegFile &old, RegFile &regfile) {
  double wait_until = - 1.0;
  int r = (int)A1;
  for (auto it: args) {
    auto val = it.get_value(old);
    regfile.set_value((Reg)r, val.first);
    if (val.second > wait_until)
      wait_until = val.second;
    r++;
  }

  return get_wait_cost(cost_acc, get_wait_cost(cost_acc, wait_until));
}

pair<double, double> StmtCall::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  return make_pair(0, 0);
}


/** assertion */

StmtAssert::StmtAssert(int _line, Value _op1, Value _op2):
Stmt(_line, RegNone, Assert), op1(_op1), op2(_op2){}

pair<double, double> StmtAssert::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto val1 = op1.get_value(regfile);
  auto val2 = op2.get_value(regfile);
  double wait_until = max(val1.second, val2.second);

  if (val1.first == val2.first)
    return make_pair(CurrentMachine->machine_cost->ASSERT, get_wait_cost(cost_acc, wait_until));

  invoke_assertion_failed(regfile);
  return make_pair(0, 0);
}


/** read and write */

StmtRead::StmtRead(int _line, Reg _lhs): Stmt(_line, _lhs, Read) {}

pair<double, double> StmtRead::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  string input;
  cin >> input;

  try {
    uint64_t result = stoull(input);
    regfile.write_reg(get_lhs(), result);
    return make_pair(CurrentMachine->machine_cost->CALL, 0);
  } catch (exception& e) {
    invoke_runtime_error("invalid input");
    return make_pair(0, 0);
  }
}

StmtWrite::StmtWrite(int _line, Reg _lhs, Value _val): Stmt(_line, _lhs, Write), val(_val) {}

pair<double, double> StmtWrite::exec(double cost_acc, RegFile &regfile, Memory &memory) const {
  auto result = val.get_value(regfile);
  cout << result.first << endl;
  regfile.write_reg(get_lhs(), 0);
  return make_pair(CurrentMachine->machine_cost->CALL + CurrentMachine->machine_cost->PER_ARG, get_wait_cost(cost_acc, result.second));
}
