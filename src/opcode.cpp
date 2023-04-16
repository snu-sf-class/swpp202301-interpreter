#include "opcode.h"

Cost NormalCost =
  {
   // cost of terminators
   1.0, // RET
   1.0, // BRUNCOND
   6.0, // BRCOND_TRUE
   1.0, // BRCOND_FALSE
   4.0, // SWITCH

   // cost of memory operations
   50.0, // MALLOC
   50.0, // FREE
   20.0, // STACK
   30.0, // HEAP
   1.0, // ALOAD
   24.0, // WAIT_STACK
   34.0, // WAIT_HEAP

   // cost of binary operations
   1.0, // MULDIV
   4.0, // LOGICAL
   5.0, // ADDSUB

   // cost of sum operation
   10.0, // SUM

   // cost of unary operartions
   1.0, // UOP

   // cost of comparison
   1.0, // COMP

   // cost of ternary operation
   1.0, // TERNARY

   // cost of function call
   2.0, // CALL
   40.0, // CALL_ORACLE
   1.0, // PER_ARG

   // cost of assertion
   0.0, // ASSERT
  };

Cost OracleCost =
  {
   // cost of terminators
   1.0, // RET
   1.0, // BRUNCOND
   6.0, // BRCOND_TRUE
   1.0, // BRCOND_FALSE
   4.0, // SWITCH

   // cost of memory operations
   50.0, // MALLOC
   50.0, // FREE
   2.0, // STACK
   3.0, // HEAP
   1.0, // ALOAD
   24.0, // WAIT_STACK
   34.0, // WAIT_HEAP

   // cost of binary operations
   1.0, // MULDIV
   4.0, // LOGICAL
   5.0, // ADDSUB

   // cost of sum operation
   10.0, // SUM

   // cost of unary operartions
   1.0, // UOP

   // cost of comparison
   1.0, // COMP

   // cost of ternary operation
   1.0, // TERNARY

   // cost of function call
   2.0, // CALL
   40.0, // CALL_ORACLE
   1.0, // PER_ARG

   // cost of assertion
   0.0, // ASSERT
  };

Machine NormalMachine =
  {
   Normal, // machine_kind
   &NormalCost, // machine_cost
  };

Machine OracleMachine =
  {
   Oracle, // machine_kind
   &OracleCost, // machine_cost
  };

Machine *CurrentMachine = &NormalMachine;

void switch_to_normal() {
  CurrentMachine = &NormalMachine;
  return;
}

void switch_to_oracle() {
  CurrentMachine = &OracleMachine;
  return;
}

bool is_normal() {
  return (CurrentMachine->machine_kind == Normal);
}

bool is_oracle() {
  return (CurrentMachine->machine_kind == Oracle);
}

const string oracle_fname = "oracle";
bool is_oracle_function(const string& fname) {
  return (fname.compare(oracle_fname) == 0);
}
