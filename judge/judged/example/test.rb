#!/usr/bin/env ruby

require 'json'
require 'fiddle'
require 'pathname'

base_dir = Pathname(__FILE__).realpath.dirname

lib_fn = base_dir.dirname.join('lib').join('libjudge.so').to_s
lib = Fiddle.dlopen lib_fn
judge_func = Fiddle::Function.new(
    lib['judge_json_ret'],
    [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP],
    Fiddle::TYPE_VOIDP
)

task_json = {
    limit_time_cpu: 1000,
    limit_time_real: 3000,
    limit_memory: 1024 * 1024,
    limit_output: 64 * 1024,
    limit_stack: 1 * 1024,
    limit_process_number: 10,
    work_dir: Pathname.getwd.to_s,
    src_path: '',     #'main.c',
    compile_cmd: '',  #'gcc -ansi main.c -o main',
    exe_path: "./main",
    argv: ["./main", nil],
    envp: ["PATH=/bin:/usr/bin", nil],
    input_path: base_dir.join('1.in').to_s,
    output_path: base_dir.join('1.out').to_s,
    error_path: '/dev/null',
    ans_path: base_dir.join('1.inout').to_s
}.to_json
puts task_json

start_time = Time.now
puts JSON.parse judge_func.call(task_json, 'koj.log').to_s
end_time = Time.now
puts "Run 1 Time: #{end_time - start_time}s"

start_time = Time.now
10.times { JSON.parse judge_func.call(task_json, 'koj.log').to_s }
end_time = Time.now
puts "Run 10 Times: #{end_time - start_time}s"
