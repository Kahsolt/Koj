# Koj - Judge

----
This is a revised and optimized version of QingdaoOJ Judger

## Dependencies
seccomp
pthread

## Setup

`shell
make
make test
`

### API: crossing language call via libffi
  * Load this [DLL](/judged/lib/libjudge.so)
  * call C function: [char* judge_json_ret(char* task_json)](/judged/include/judge.h)
  
### API: json messages through TCP/Unix Socket

  参数顺序固定如下，且必须使用紧凑的json: 如Ruby的json包to_json方法
  注意：所有路径参数请传入绝对路径（相对路径仅用于内部测试）

`json
{
  "task": {
    "limit_time_cpu": 1000, // ms
    "limit_time_real": 3000,// ms
    "limit_memory": 4096,   // KB
    "limit_output": 4096,   // KB
    "limit_stack": 4096,    // KB
    "limit_process_number": 10,
    "work_dir": "example",                  // (set to "." if not used)
    "src_path": "main.c",                   // (set to "" if not used)
    "compile_cmd": "gcc main.c -o main",    // (set to "" if not used)
    "exe_path": "main",
    "argv": ["main", null],
    "envp": ["PATH=/bin:/usr/bin", null],
    "input_path": "1.in",       // std in
    "output_path": "1.out",     // std out(answer), set to "" if not used
    "error_path": "/dev/null",
    "ans_path": "ans"           // user output
  },
  "result": {
    "result": "AC",     // str@["AC", "CE", "RE", "TLE", "MLE", "OLE", "PE", "WA", "SE"]
    "time": 233,        // ms
    "memory": 500,      // KB
    "code_length": 666  // B, (0 if not available)
  }
}
`