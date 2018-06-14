#!/usr/bin/env ruby

require 'json'
require 'bunny'
require 'thread'
require 'pathname'
require 'securerandom'

class Submit
  attr_accessor :call_id, :response, :lock, :condition,
                :connection, :channel, :reply_queue, :queue_name, :exchange

  def initialize(queue_name)
    @connection = Bunny.new.start
    @channel = @connection.create_channel
    @exchange = @channel.default_exchange
    @queue_name = queue_name

    @lock = Mutex.new
    @condition = ConditionVariable.new
    @reply_queue = @channel.queue 'koj_res', exclusive: false

    that = self
    @reply_queue.subscribe exclusive: false do |_delivery_info, properties, payload|
      if properties[:correlation_id] == that.call_id
        that.response = payload
        # TODO: write Django DB
        # that.lock.synchronize { that.condition.signal }
      end
    end
  end

  def call(submit)
    @call_id = SecureRandom.uuid
    @exchange.publish submit,
                      routing_key: @queue_name,
                      correlation_id: @call_id,
                      reply_to: @reply_queue.name,
                      persistence: true
    # lock.synchronize { condition.wait(lock) }
    # @response
    "OK"
  end

  def stop
    @channel.close
    @connection.close
  end
end

# hard sync test data
PID = 'test'
base_dir = Pathname(__FILE__).realpath.dirname

data_dir = base_dir.dirname.join('data').join(PID).to_s
puts data_dir
system "mkdir -p #{data_dir}"
system "cp -u data/*.in data/*.out #{data_dir}"

src = {
  'c': [
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x+y);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d %d", (x+y)/10, (x+y)%10);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x-y);\nreturn 0;\n}',
    'int main()\n{\nprintf("CE")\nreturn 0;\n}',
    'int main[65535]={0};',
    '#include<stdlib.h>\nint main()\n{\nint *p=1;\nfree(p);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x=0;\nwhile(1) x++;\nreturn 0;\n}',
    '#include<stdlib.h>\nint main()\n{\nint *p;\nwhile(1) p=malloc(1024);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nwhile(1) printf("OLE");\nreturn 0;\n}',
  ],
  'cpp': [
    '#include<iostream>\nusing namespace std;\n\nint main()\n{\nint x, y;\ncin>>x>>y;\ncout<<x+y;\nreturn 0;\n}',
    '#include<iostream>\nusing namespace std;\n\nint main()\n{\nint x, y;\ncin>>x>>y;\ncout<<" "<<x+y;\nreturn 0;\n}',
    '#include<iostream>\nint main()\n{\n  int x, y;\n  cin>>x>>y;\ncout<<x+y;\nreturn 0;\n}',
  ],
  'java': [
    'import java.io.*;\nimport java.util.*;\n\npublic class Main {\npublic static void main(String args[]) throws Exception {\nScanner scanner = new Scanner(System.in);\nint x = scanner.nextInt();\nint y = scanner.nextInt();\nSystem.out.println(x+y);\n}\n}',
    'public class Hello {public static void main(String args[]){}}',
    'public class Main {}',
  ],
  'python2': [
    'print sum([int(x) for x in raw_input().split()])',
    'print 233',
    'import exit\nsys.exit()'
  ],
  'python3': [
    'print(sum([int(x) for x in input().split()]))',
    'print("154")',
    'p()'
  ],
  'ruby': [
    'print(gets.split.map {|e| e.to_i}.reduce(:+))',
    'puts "233"',
    'system "kill 9"',
  ],
  'lua': [
    'x=io.read("*number")\ny=io.read("*number")\nio.write(x+y)',
    'waht',
    'print(os.clock())',
  ],
}
lang = src.keys

# put submit in queue
begin
  count = 0
  client = Submit.new('koj')
  N = 100
  start_time = Time.now
  N.times do
    _lang = lang.sample
    _src = src[_lang].sample
    submit = {
        :uuid => SecureRandom.uuid,
        :pid  => PID,
        :time => 1500,
        :memory => 512 * 1024,
        :output => 32 * 1024,
        :lang => _lang.to_s,
        :src  => _src,
    }
    submit_json = submit.to_json
    count += 1
    puts " [#{count}] Requesting a submit on #{_lang}"
    client.call(submit_json)
    # puts " [#{count}] Got #{response}"
  end
  end_time = Time.now
  puts "Publish #{N} submits: #{end_time - start_time}s"
  puts "#{start_time} -> #{end_time}"
rescue Interrupt => _
  client.stop
end
