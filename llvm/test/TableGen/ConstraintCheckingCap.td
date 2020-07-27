// RUN: llvm-tblgen -gen-instr-info -I %p -I %p/../../include %s
// RUN: llvm-tblgen -gen-instr-info -I %p -I %p/../../include %s | FileCheck %s
include "ConstraintChecking.inc"

def Src1MustBeTagged : TestInstructionWithConstraints<"@traps_if_untagged $src1">;
def Src1MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1">;
def Src1MustBeBothA : TestInstructionWithConstraints<"@traps_if_sealed $src1, @traps_if_untagged $src1">;
def Src1MustBeBothB : TestInstructionWithConstraints<"@traps_if_untagged $src1, @traps_if_sealed $src1">;

// CHECK-DAG: MCOperandInfo [[BOTH_OPINFO:OperandInfo[0-9]+]]{{.+}}{ TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability)|(1<<MCOI::TrapsIfUntaggedCapability), MCOI::OPERAND_REGISTER, 0 }
// CHECK-DAG: MCOperandInfo [[UNSEALED_OPINFO:OperandInfo[0-9]+]]{{.+}}{ TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }
// CHECK-DAG: MCOperandInfo [[TAGGED_OPINFO:OperandInfo[0-9]+]]{{.+}}{ TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfUntaggedCapability), MCOI::OPERAND_REGISTER, 0 }


// CHECK: { 193,{{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput)|(1ULL<<MCID::MayTrapOnUntaggedInput){{.+}}, [[BOTH_OPINFO]] },  // Inst #193 = Src1MustBeBothA
// CHECK: { 194, {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput)|(1ULL<<MCID::MayTrapOnUntaggedInput){{.+}},  [[BOTH_OPINFO]] },  // Inst #194 = Src1MustBeBothB
// CHECK: { 195, {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[UNSEALED_OPINFO]] },  // Inst #195 = Src1MustBeSealed
// CHECK: { 196, {{.+}}, 0|(1ULL<<MCID::MayTrapOnUntaggedInput){{.+}}, [[TAGGED_OPINFO]] },  // Inst #196 = Src1MustBeTagged
