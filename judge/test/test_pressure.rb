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
    @response
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

# put submit in queue
begin
  count = 0
  client = Submit.new('koj')
  N = 100
  start_time = Time.now
  N.times do
    submit_dir = base_dir.join('submit')
    file = '' ; file = Dir.new(submit_dir).entries.sample until file.start_with? 'test'

    submit = {
        :uuid => SecureRandom.uuid,
        :pid  => PID,
        :time => 1500,
        :memory => 512 * 1024,
        :output => 32 * 1024,
        :lang => file.split('.').last,
        :src  => File.read(submit_dir.join(file).to_s),
    }
    submit_json = submit.to_json
    count += 1
    puts " [#{count}] Requesting a submit on #{submit[:lang]}"
    client.call(submit_json)
    # puts " [#{count}] Got #{response}"
  end
  end_time = Time.now
  puts "Publish #{N} submits: #{end_time - start_time}s"
  puts "#{start_time} -> #{end_time}"
rescue Interrupt => _
  client.stop
end
