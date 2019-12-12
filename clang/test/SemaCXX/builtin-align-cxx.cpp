// C++-specific checks for the alignment builtins
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -std=c++11 -o - %s -fsyntax-only -ast-dump -verify
// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -std=c++11 -o - %s -fsyntax-only -verify

template<typename T, long Alignment, long ArraySize = 16>
void test_templated_arguments() {
  T array[ArraySize]; // expected-error{{variable has incomplete type 'fwddecl'}}
  static_assert(__is_same(decltype(__builtin_align_up(array, Alignment)), T*), // expected-error{{requested alignment is not a power of 2}}
      "return type should be the decayed array type");
  static_assert(__is_same(decltype(__builtin_align_down(array, Alignment)), T*),
      "return type should be the decayed array type");
  static_assert(__is_same(decltype(__builtin_is_aligned(array, Alignment)), bool),
      "return type should be bool");
  T* x1 = __builtin_align_up(array, Alignment);
  T* x2 = __builtin_align_down(array, Alignment);
  bool x3 = __builtin_align_up(array, Alignment);
}

void test() {
  test_templated_arguments<int, 32>(); // fine
  test_templated_arguments<struct fwddecl, 16>();
  // expected-note@-1{{in instantiation of function template specialization 'test_templated_arguments<fwddecl, 16, 16>'}}
  // expected-note@-2{{forward declaration of 'fwddecl'}}
  test_templated_arguments<int, 7>(); // invalid alignment value
  // expected-note@-1{{in instantiation of function template specialization 'test_templated_arguments<int, 7, 16>'}}

}

template<typename T>
void test_incorrect_alignment_without_instatiation(T value) {
  int array[32];
  static_assert(__is_same(decltype(__builtin_align_up(array, 31)), int*), // expected-error{{requested alignment is not a power of 2}}
      "return type should be the decayed array type");
  static_assert(__is_same(decltype(__builtin_align_down(array, 7)), int*), // expected-error{{requested alignment is not a power of 2}}
      "return type should be the decayed array type");
  static_assert(__is_same(decltype(__builtin_is_aligned(array, -1)), bool), // expected-error{{requested alignment must be 1 or greater}}
      "return type should be bool");
  __builtin_align_up(array); // expected-error{{too few arguments to function call, expected 2, have 1}}
  __builtin_align_up(array, 31); // expected-error{{requested alignment is not a power of 2}}
  __builtin_align_down(array, 31); // expected-error{{requested alignment is not a power of 2}}
  __builtin_align_up(array, 31); // expected-error{{requested alignment is not a power of 2}}
  __builtin_align_up(value, 31); // This shouldn't want since the type is dependent
  __builtin_align_up(value); // Same here
}

// Check that we don't crash when using dependent types in __builtin_align:
template <typename a, a b>
void *c(void *d) { // expected-note{{candidate template ignored}}
  return __builtin_align_down(d, b);
}

struct x {};
x foo;
void test(void *value) {
  c<int, 16>(value);
  c<struct x, foo>(value); // expected-error{{no matching function for call to 'c'}}
}

// The original fix for the issue above broke some legitimate code.
// Here is a regression test:
typedef __SIZE_TYPE__ size_t;
void *allocate_impl(size_t size);
template <typename T>
T *allocate() {
  constexpr size_t allocation_size =
      __builtin_align_up(sizeof(T), sizeof(void *));
  return static_cast<T *>(
      __builtin_assume_aligned(allocate_impl(allocation_size), sizeof(void *)));
}
struct Foo {
  int value;
};
void *test2() {
  return allocate<struct Foo>();
}


// Check that pointers-to-members cannot be used

class MemPtr {
public:
  int data;
  void func();
  virtual void vfunc();
};
void test_member_ptr() {
  __builtin_align_up(&MemPtr::data, 64); // expected-error{{operand of type 'int MemPtr::*' where arithmetic or pointer type is required}}
  __builtin_align_down(&MemPtr::func, 64); // expected-error{{operand of type 'void (MemPtr::*)()' where arithmetic or pointer type is required}}
  __builtin_is_aligned(&MemPtr::vfunc, 64); // expected-error{{operand of type 'void (MemPtr::*)()' where arithmetic or pointer type is required}}
}
