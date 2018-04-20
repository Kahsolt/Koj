#!/usr/bin/env ruby

require 'json'
require 'bunny'
require 'pathname'
require 'securerandom'

PID = 'example'
base_dir = Pathname(__FILE__).realpath.dirname

# hard sync test data
data_dir = base_dir.dirname.dirname.join('data').join(PID).to_s
puts data_dir
system "mkdir -p #{data_dir}"
system "cp -u *.in *.out #{data_dir}"

# put submit in queue
conn = Bunny.new(:automatically_recover => false).start
q    = conn.create_channel.queue('koj')

submit = {
    :uuid => SecureRandom.uuid,
    :pid  => PID,
    :time => 1000,
    :memory => 512 * 1024,
    :output => 64 * 1024,
    :lang => 'c',
    :src  => File.read(base_dir.join('main.c').to_s),
}.to_json
q.publish(submit, :persistence => false)
puts "Enqueue: #{submit}"

conn.close