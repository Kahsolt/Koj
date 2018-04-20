#!/usr/bin/env ruby

require 'json'
require 'fiddle'
require 'pathname'

lib_fn = Pathname(__FILE__).realpath.dirname.dirname.join('lib').join('libjudge.so').to_s
lib = Fiddle.dlopen lib_fn
judge_func = Fiddle::Function.new(
    lib['judge_json_ret'],
    [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP],
    Fiddle::TYPE_VOIDP
)

start_time = Time.now
Dir.new(Pathname.getwd).each_entry do |fn_src|
  next if %w[. ..].index fn_src
  src_path = Pathname(__FILE__).realpath.dirname.join(fn_src).to_s
  next unless fn_src.end_with? '.c', '.cpp'
  cc = fn_src.end_with?('.c') ? 'gcc': 'g++'
  fn_out = fn_src.split('.').first

  task = {
      limit_time_cpu: 1000,
      limit_time_real: 3000,
      limit_memory: 1024 * 1024,
      limit_output: 64 * 1024,
      limit_stack: 1 * 1024,
      limit_process_number: 10,
      work_dir: Pathname.getwd.to_s,
      src_path: src_path,
      compile_cmd: "#{cc} #{fn_src} -o #{fn_out}",
      exe_path: fn_out,
      argv: [fn_out, nil],
      envp: ["PATH=/bin:/usr/bin", "USER=nobody", nil],
      input_path: "/dev/null",
      output_path: "/dev/null",
      error_path: "/dev/null",
      ans_path: "/dev/null"
  }
  task_json = task.to_json
  puts task[:compile_cmd]
  puts JSON.parse judge_func.call(task_json, 'koj.log').to_s
  File.unlink fn_out if File.file? fn_out
end
end_time = Time.now
puts "Run All Tests: #{end_time - start_time}s"