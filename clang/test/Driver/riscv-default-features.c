// RUN: %clang -target riscv32-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32
// RUN: %clang -target riscv64-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-NORVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NORVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-NORVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NORVC
// The -mxcheri-rvc flag to explicitly disable xcheri-norvc:
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-EXPLICIT-RVC

// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-no-v8-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NOV8COMPAT
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-v8-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-V8COMPAT

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcherimin -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERIMIN
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcherimin -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERIMIN

// RV32: "target-features"="+a,+c,+m,+relax,-save-restore"
// RV64: "target-features"="+64bit,+a,+c,+m,+relax,-save-restore"

// RV32-XCHERI: "target-features"="+cheri-capabilities,+relax,+xcheri,-save-restore"
// RV64-XCHERI: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,-save-restore"

// RV32-XCHERI-RVC: "target-features"="+relax,+xcheri,-save-restore"
// RV64-XCHERI-RVC: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,-save-restore"
// RV32-XCHERI-NORVC: "target-features"="+cheri-capabilities,+relax,+xcheri,+xcheri-norvc,-save-restore"
// RV64-XCHERI-NORVC: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,+xcheri-norvc,-save-restore"
// RV32-XCHERI-EXPLICIT-RVC: "target-features"="+cheri-capabilities,+relax,+xcheri,-save-restore,-xcheri-norvc"
// RV64-XCHERI-EXPLICIT-RVC: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,-save-restore,-xcheri-norvc"

// RV64-XCHERI-V8COMPAT: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,-save-restore,-xcheri-no-v8-compat"
// RV64-XCHERI-NOV8COMPAT: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri,+xcheri-no-v8-compat,-save-restore"

// RV32-XCHERIMIN: "target-features"="+cheri-capabilities,+relax,+xcheri-no-v8-compat,+xcherimin,-save-restore"
// RV64-XCHERIMIN: "target-features"="+64bit,+cheri-capabilities,+relax,+xcheri-no-v8-compat,+xcherimin,-save-restore"

// Dummy function
int foo(){
  return  3;
}
