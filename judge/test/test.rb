#!/usr/bin/env ruby

require 'json'
require 'bunny'
require 'pathname'
require 'securerandom'

PID = 'test'
base_dir = Pathname(__FILE__).realpath.dirname

# hard sync test data
data_dir = base_dir.dirname.join('data').join(PID).to_s
puts data_dir
system "mkdir -p #{data_dir}"
system "cp -u data/*.in data/*.out #{data_dir}"

# put submit in queue
conn = Bunny.new(:automatically_recover => false).start
ch   = conn.create_channel
ch.prefetch 10
q    = ch.queue 'koj'

submit_dir = base_dir.join('submit')
Dir.new(submit_dir).each_entry do |file|
  next unless file.start_with? 'test'

  submit = {
      :uuid => SecureRandom.uuid,
      :pid  => PID,
      :time => 1500,
      :memory => 512 * 1024,
      :output => 32 * 1024,
      :lang => file.split('.').last,
      :src  => File.read(submit_dir.join(file).to_s),
  }.to_json
  q.publish submit, priority: 100
  puts "Enqueue: #{submit}"
end

conn.close