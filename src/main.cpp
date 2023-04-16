#include <iostream>
#include <fstream>
#include <iomanip>

#include "parser.h"
#include "state.h"
#include "error.h"

using namespace std;


int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "USAGE: swpp-interpreter <input assembly file>" << endl;
    return 1;
  }

  string filename = argv[1];
  error_filename = filename;

  Program* program = parse(filename);
  if (program == nullptr) {
    cout << "Error: cannot find " << filename << endl;
    return 1;
  }

  State state;
  state.set_program(program);
  uint64_t ret = state.exec_program();

  ofstream log("swpp-interpreter.log");
  double exec_cost = state.get_cost_value();
  double max_heap_size = state.get_max_alloced_size();
  log << fixed << setprecision(4);
  log << "Returned: " << ret << endl;
  log << "Execution cost: " << exec_cost << endl;
  log << "Max heap usage (bytes): " << max_heap_size << endl;
  log << "Total cost: " << exec_cost + max_heap_size * 16.0 << endl;
  log.close();

  ofstream cost_log("swpp-interpreter-cost.log");
  cost_log << fixed << setprecision(4);
  cost_log << "Total waiting cost: " << state.get_total_wait_cost() << endl;
  cost_log << state.get_cost()->to_string("");
  cost_log.close();

  ofstream inst_log("swpp-interpreter-inst.log");
  inst_log << state.inst_log_to_string();
  inst_log.close();

  return 0;
}
