#include "ruby.h"
#include "cdjukebox.h"

VALUE CDPlayer;

static void cd_free(void *p) { 
  CDPlayerDispose(p); 
}
static void progress(CDJukebox *rec, int percent) 
{ 
  if (rb_block_given_p()) { 
    if (percent > 100) percent = 100; 
    if (percent < 0) percent = 0; 
    rb_yield(INT2FIX(percent)); 
  } 
}
static VALUE 
cd_seek(VALUE self, VALUE disc, VALUE track) 
{ 
  CDJukebox *ptr; 
  Data_Get_Struct(self, CDJukebox, ptr);
  CDPlayerSeek(ptr, 
               NUM2INT(disc), 
               NUM2INT(track), 
               progress); 
  return Qnil; 
}
static VALUE 
cd_seekTime(VALUE self) 
{ 
  double tm; 
  CDJukebox *ptr; 
  Data_Get_Struct(self, CDJukebox, ptr); 
  tm = CDPlayerAvgSeekTime(ptr); 
  return rb_float_new(tm); 
}
static VALUE 
cd_unit(VALUE self) 
{ 
  return rb_iv_get(self, "@unit"); 
}

static VALUE 
cd_init(VALUE self, VALUE unit) 
{ 
  rb_iv_set(self, "@unit", unit); 
  return self; 
}
VALUE cd_new(VALUE class, VALUE unit) 
{ 
  VALUE argv[1]; 
  CDJukebox *ptr = CDPlayerNew(NUM2INT(unit)); 
  VALUE tdata = Data_Wrap_Struct(class, 0, cd_free, ptr); 
  argv[0] = unit; 
  rb_obj_call_init(tdata, 1, argv); 
  return tdata; 
}
void Init_CDJukebox() { 
  cCDPlayer = rb_define_class("CDPlayer", rb_cObject); 
  rb_define_singleton_method(cCDPlayer, "new", cd_new, 1); 
  rb_define_method(cCDPlayer, "initialize", cd_init, 1); 
  rb_define_method(cCDPlayer, "seek", cd_seek, 2); 
  rb_define_method(cCDPlayer, "seekTime", cd_seekTime, 0); 
  rb_define_method(cCDPlayer, "unit", cd_unit, 0);
}

