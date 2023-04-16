#ifndef SWPP_ASM_INTERPRETER_STMT_H
#define SWPP_ASM_INTERPRETER_STMT_H

#include <string>
#include <map>
#include <utility>

#include "opcode.h"
#include "value.h"
#include "size.h"
#include "memory.h"

using namespace std;


class Stmt {
private:
  const int line;
  const Reg lhs;
  const Opcode opcode;
  Stmt* next;

public:
  Stmt(int _line, Reg _lhs, Opcode _opcode);

  int get_line() const;
  Reg get_lhs() const;
  Opcode get_opcode() const;
  Stmt* get_next() const;
  void set_next(Stmt* stmt);

  virtual pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const = 0;
};


/** terminators */

class StmtRet: public Stmt {
private:
  const Value val;

public:
  explicit StmtRet(int _line, Value _val);

  pair<uint64_t, double> get_val(double cost_acc, RegFile &regfile) const;
  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtBrUncond: public Stmt {
private:
  const string bb;

public:
  explicit StmtBrUncond(int _line, string _bb);

  string get_bb() const;
  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtBrCond: public Stmt {
private:
  const Value cond;
  const string true_bb;
  const string false_bb;
  bool eval = true;

public:
  StmtBrCond(int _line, Value _cond, string _true_bb, string _false_bb);

  pair<string, double> get_bb(double cost_acc, RegFile& regfile);
  bool get_eval() const;
  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtSwitch: public Stmt {
private:
  const Value cond;
  map<uint64_t, string> bb_map;
  string default_bb;

public:
  explicit StmtSwitch(int _line, Value _cond);

  bool set_bb(uint64_t val, string bb);
  void set_default(string bb);
  bool case_exists(uint64_t val) const;
  pair<string, double> get_bb(double cost_acc, RegFile& regfile) const;
  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** memory operations */

class StmtMalloc: public Stmt {
private:
  const Value val;

public:
  StmtMalloc(int _line, Reg _lhs, Value _val);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtFree: public Stmt {
private:
  const Value ptr;

public:
  explicit StmtFree(int _line, Value _ptr);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtLoad: public Stmt {
private:
  const bool is_async;
  const MSize size;
  const Value ptr;
  const uint64_t ofs;

public:
  StmtLoad(int _line, Reg _lhs, bool _is_async, MSize _size, Value _ptr, uint64_t _ofs);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtStore: public Stmt {
private:
  const MSize size;
  const Value val;
  const Value ptr;
  const uint64_t ofs;

public:
  StmtStore(int _line, MSize _size, Value _val, Value _ptr, uint64_t _ofs);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** binary operations */

class StmtBop: public Stmt {
private:
  const BopKind bop_kind;
  const Value val1;
  const Value val2;
  const Size size;

  uint64_t compute(uint64_t op1, uint64_t op2) const;

public:
  StmtBop(int _line, Reg _lhs, BopKind _bop_kind, Value _val1, Value _val2, Size size);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** sum operation */

class StmtSum: public Stmt {
public:
  const static int num_operands = 8;
private:
  vector<Value> values;
  const Size size;

public:
  StmtSum(int _line, Reg _lhs, const vector<Value>& _values, Size _size);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** unary operations */

class StmtUop: public Stmt {
private:
  const UopKind uop_kind;
  const Value val;
  const Size size;

public:
  StmtUop(int _line, Reg _lhs, UopKind _uop_kind, Value _val, Size _size);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** ternary operation */

class StmtSelect: public Stmt {
private:
  const Value cond;
  const Value val_true;
  const Value val_false;

public:
  StmtSelect(int _line, Reg _lhs, Value _cond, Value _val_true, Value _val_false);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** function call */

class StmtCall: public Stmt {
private:
  const string fname;
  vector<Value> args;

public:
  StmtCall(int _line, Reg _lhs, string _fname);

  string get_fname() const;
  void push_arg(Value arg);
  int get_nargs();
  double setup_args(double cost_acc, RegFile& old, RegFile& regfile);
  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** assertion */

class StmtAssert: public Stmt {
private:
  const Value op1;
  const Value op2;

public:
  StmtAssert(int _line, Value _op1, Value _op2);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};


/** read and write */

class StmtRead: public Stmt {
public:
  StmtRead(int _line, Reg _lhs);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

class StmtWrite: public Stmt {
private:
  const Value val;

public:
  StmtWrite(int _line, Reg _lhs, Value _val);

  pair<double, double> exec(double cost_acc, RegFile& regfile, Memory& memory) const override;
};

#endif //SWPP_ASM_INTERPRETER_STMT_H
