# Judger for Koj

### Config
  see [Config Dir](/conf)

### Dependencies
  * libffi
   
### Run
  `make && make test && make run` 
  
### API Interface
  Data sync: [Test Data Folder](/data)
  MQ dequeue msg: {uuid: uuid, src: str, lang: str@enum, pid: str, time: int, memory: int, output: int}
  MQ enqueue msg: {result: str@enum, memory: int, time: int, code_length: int}