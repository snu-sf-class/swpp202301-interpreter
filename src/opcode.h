#ifndef SWPP_ASM_INTERPRETER_OPCODE_H
#define SWPP_ASM_INTERPRETER_OPCODE_H

#include <string>

using namespace std;


enum Opcode {
  // terminators
  Ret = 0,
  BrUncond,
  BrCond,
  Switch,

  // memory operations
  Malloc,
  Free,
  Load,
  Store,

  // binary operations
  Bop,

  // sum
  Sum,

  // unary operations
  Uop,

  // ternary operation
  Select,

  // function call
  Call,

  // assertion
  Assert,

  // read and write
  Read,
  Write,

  LEN_OPCODE
};

enum BopKind {
  // arithmetic operations
  Udiv = 0,
  Sdiv,
  Urem,
  Srem,
  Mul,

  // logical operations
  Shl,
  Lshr,
  Ashr,
  And,
  Or,
  Xor,
  Add,
  Sub,

  // comparisons
  Eq,
  Ne,
  Ugt,
  Uge,
  Ult,
  Ule,
  Sgt,
  Sge,
  Slt,
  Sle
};

enum UopKind {
  Incr = 0,
  Decr
};

struct Cost {
  // cost of terminators
  double RET;
  double BRUNCOND;
  double BRCOND_TRUE;
  double BRCOND_FALSE;
  double SWITCH;

  // cost of memory operations
  double MALLOC;
  double FREE;
  double STACK;
  double HEAP;
  double ALOAD;
  double WAIT_STACK;
  double WAIT_HEAP;

  // cost of binary operations
  double MULDIV;
  double LOGICAL;
  double ADDSUB;

  // cost of sum operation
  double SUM;

  // cost of unary operartions
  double UOP;

  // cost of comparison
  double COMP;

  // cost of ternary operation
  double TERNARY;

  // cost of function call
  double CALL;
  double CALL_ORACLE;
  double PER_ARG;

  // cost of assertion
  double ASSERT;
};

enum MachineKind {
  Normal = 0,
  Oracle,

  LEN_MACHINE
};

struct Machine {
  // cost of terminators
  MachineKind machine_kind;
  Cost *machine_cost;
};

extern Machine *CurrentMachine;

void switch_to_normal();
void switch_to_oracle();
bool is_normal();
bool is_oracle();
bool is_oracle_function(const string& fname);

#endif //SWPP_ASM_INTERPRETER_OPCODE_H
