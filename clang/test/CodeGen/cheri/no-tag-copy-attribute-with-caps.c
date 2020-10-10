// REQUIRES: riscv-registered-target
// RUN: %riscv64_cheri_purecap_cc1 %s -o - -emit-llvm | FileCheck %s
// Diagnostics are only emitted when generating assembly (with optimizations)
// RUN: %riscv64_cheri_purecap_cc1 -debug-info-kind=standalone %s -o /dev/null -O1 -S -verify

struct OneCap {
  void *__capability b;
} dst;

// define void @test_addrof_char(%struct.OneCap addrspace(200)* {{%[a-z0-9]+}}, i8 signext {{%[a-z0-9]+}}, i128 {{%[a-z0-9]+}}) addrspace(200) #0 {

void test_addrof_char(struct OneCap *cap, char c, __uint128_t u) {
  // CHECK-LABEL: void @test_addrof_char(
  // Since this is an address-of expression we should be able to detect that
  // the source does not contain tags
  __builtin_memmove(cap, &c, sizeof(c));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 {{%[a-z0-9.]+}}
  // CHECK-SAME: , i64 1, i1 false) [[MUST_PRESERVE_ATTR:#[0-9]+]]{{$}}
  __builtin_memmove(&c, cap, sizeof(c));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 {{%[a-z0-9]+}}.addr, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 1, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR:#[0-9]+]]{{$}}
  __builtin_memmove(cap, &u, sizeof(u));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]{{$}}
  __builtin_memmove(&u, cap, sizeof(u));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

void test_small_copy(struct OneCap *cap1, struct OneCap *cap2) {
  // CHECK-LABEL: void @test_small_copy(
  __builtin_memmove(cap1, cap2, sizeof(*cap1));
  // This copy preserves tags
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
  __builtin_memmove(cap1, cap2, 2);
  // TODO :This copy is too small -> should not preserve tags
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 2, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

struct strbuf {
  char data[16];
};

void test_addrof_char_buf(struct OneCap *cap, struct strbuf s) {
  // CHECK-LABEL: void @test_addrof_char_buf(

  // Since this is an address-of expression we should be able to detect that
  // the source does not contain tags
  // FIXME: the struct contains a char[] so maybe we should assume that it might
  // be used to hold tags? But then the user should have added an _Alignas()
  // attribute so this should be safe
  __builtin_memmove(cap, &s, sizeof(s));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[NO_PRESERVE_ATTR:#[0-9]+]]
  __builtin_memmove(&s, cap, sizeof(s));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[NO_PRESERVE_ATTR]]
}

void test_array_decay(struct OneCap *cap) {
  // CHECK-LABEL: void @test_array_decay(
  // array-decay -> We can tell that the buffer does not contain tags
  int buf[16];
  __builtin_memmove(cap, buf, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 4 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]
  __builtin_memmove(buf, cap, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 4 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}

  // char array aligned to one byte -> does not contain tags
  char buf2[16];
  __builtin_memmove(cap, buf2, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]
  __builtin_memmove(buf2, cap, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}

  // But we don't add the attribute for aligned char array decay since those are
  // often used for buffers that contain anything.
  _Alignas(void *__capability) char aligned_char_buf[16];
  __builtin_memmove(cap, aligned_char_buf, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]
  __builtin_memmove(aligned_char_buf, cap, sizeof(*cap));
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

void test_string_constant(struct OneCap *cap) {
  // CHECK-LABEL: void @test_string_constant(
  // Same for string -> char*
  __builtin_memmove(cap, "abcdefghijklmnopqrstuvwxyz", sizeof(*cap));
  // expected-warning@-1{{memcpy operation with capability argument <unknown type> and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // FIXME: shouldn't print <unknown type>
  // FIXME: why is this printing memcpy?
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 getelementptr inbounds ([27 x i8], [27 x i8] addrspace(200)* @.str
  // CHECK-SAME: , i64 0
  // CHECK-SAME: , i64 0)
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]{{$}}
}

void test_void_buffer(struct OneCap *cap, void *buf) {
  // CHECK-LABEL: void @test_void_buffer(
  // A void* means unknown contents and therefore we preserve tags.
  __builtin_memmove(cap, buf, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument <unknown type> and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // FIXME: the above shouldn't print <unknown type>
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]
  __builtin_memmove(buf, cap, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument 'struct OneCap' and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

void test_char_buffer(struct OneCap *cap, char *buf) {
  // CHECK-LABEL: void @test_char_buffer(
  // We have to also assume that char* means unknown contents.
  __builtin_memmove(cap, buf, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument <unknown type> and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // FIXME: the above shouldn't print <unknown type>
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 1 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]{{$}}
  __builtin_memmove(buf, cap, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument 'struct OneCap' and underaligned destination (aligned to 1 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

void test_int_buffer(struct OneCap *cap, int *buf) {
  // CHECK-LABEL: void @test_int_buffer(
  // However, for int* we can assume it does not contain tags.
  __builtin_memmove(cap, buf, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument <unknown type> and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // FIXME: the above shouldn't print <unknown type>
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 {{%[a-z0-9]+}}, i8 addrspace(200)* align 4 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_ATTR]]{{$}}
  __builtin_memmove(buf, cap, sizeof(*cap));
  // expected-warning@-1{{memmove operation with capability argument 'struct OneCap' and underaligned destination (aligned to 4 bytes) may be inefficient or result in CHERI tags bits being stripped}} expected-note@-1{{For more information}}
  // CHECK: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 4 {{%[a-z0-9]+}}, i8 addrspace(200)* align 16 {{%[a-z0-9]+}}
  // CHECK-SAME: , i64 16, i1 false) [[MUST_PRESERVE_WITH_TYPE_ATTR]]{{$}}
}

// CHECK: attributes #0 = {
// CHECK-DAG: attributes [[MUST_PRESERVE_ATTR]] = { must_preserve_cheri_tags }
// CHECK-DAG: attributes [[MUST_PRESERVE_WITH_TYPE_ATTR]] = { must_preserve_cheri_tags "frontend-memtransfer-type"="'struct OneCap'" }
// CHECK-DAG: attributes [[NO_PRESERVE_ATTR]] = { no_preserve_cheri_tags }
