#include <stdio.h>
#include <ruby.h>
#include "judge.h"

static VALUE method_hello(VALUE self, VALUE name) {
  char* yourname = RSTRING(name)->ptr;
  printf("Hello, %s\n", yourname);
  return rb_str_new2(yourname);
};

static VALUE method_plus(VALUE self, VALUE va, VALUE vb) {
  int a = NUM2INT(va);
  int b = NUM2INT(vb);
  return INT2NUM(a + b);
}

VALUE test = Qnil;

void Init_Test(){
  test = rb_define_module("Test");
  rb_define_method(test, "hello", RUBY_METHOD_FUNC(method_hello), 1);
  rb_define_method(test, "plus", RUBY_METHOD_FUNC(method_plus), 2);
};
