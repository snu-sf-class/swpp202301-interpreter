#ifndef SWPP_ASM_INTERPRETER_STATE_H
#define SWPP_ASM_INTERPRETER_STATE_H

#include <vector>

#include "regfile.h"
#include "memory.h"
#include "program.h"
#include "opcode.h"

using namespace std;


class CostStack {
private:
  string fname;
  double cost;
  vector<CostStack*> callees;

public:
  explicit CostStack(const string& _fname);
  double get_cost() const;
  void add_cost(double _cost);
  void set_callee(CostStack* callee);
  string to_string(const string& indent) const;
};


class State {
private:
  RegFile regfile;
  Memory memory;
  CostStack* main_cost;
  double cost_per_inst[LEN_MACHINE][Opcode::LEN_OPCODE];
  int inst_count[LEN_MACHINE][Opcode::LEN_OPCODE];
  double total_wait_cost;
  Program* program;

  uint64_t exec_function(CostStack* parent, Function* function);
  void update_cost_log(Opcode opcode, double inst_cost, double wait_cost);
  string inst_log_line(MachineKind machine, Opcode opcode, const string &machine_name, const string &inst) const;
  string inst_log_machine(MachineKind machine, const string &machine_name) const;

public:
  State();

  void set_program(Program* _program);
  double get_cost_value() const;
  CostStack* get_cost() const;
  uint64_t get_max_alloced_size() const;
  uint64_t exec_program();
  string inst_log_to_string() const;
  double get_total_wait_cost() const;
};

#endif //SWPP_ASM_INTERPRETER_STATE_H
