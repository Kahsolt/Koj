#!/usr/bin/env ruby

require 'json'
require 'socket'
require 'pathname'

HOST = 'localhost'
PORT = 7000

base_dir = Pathname(__FILE__).realpath.dirname
task = {
    limit_time_cpu: 1000,
    limit_time_real: 3000,
    limit_memory: 1024 * 1024,
    limit_output: 64 * 1024,
    limit_stack: 1 * 1024,
    limit_process_number: 10,
    work_dir: Pathname.getwd.to_s,
    src_path: 'main.c',
    compile_cmd: 'gcc -ansi main.c -o main',
    exe_path: "./main",
    argv: ["./main", nil],
    envp: ["PATH=/bin:/usr/bin", nil],
    input_path: base_dir.join("1.in").to_s,
    output_path: base_dir.join("1.out").to_s,
    error_path: "/dev/null",
    ans_path: base_dir.join('1.inout').to_s
}
task_json = task.to_json
puts task_json

sock = TCPSocket.new HOST, PORT
sock.print task_json
sock.flush
puts sock.gets
sock.close

# Case not compile and no code_length
task['src_path'] = ''
task['compile_cmd'] = ''
task_json = task.to_json
puts task_json

sock = TCPSocket.new HOST, PORT
sock.puts task_json
sock.flush
puts sock.gets
sock.close