#!/usr/bin/env ruby

require 'bunny'
require 'pathname'

base_dir = Pathname(__FILE__).realpath.dirname.dirname.join('judge')
log_fn = base_dir.join('log').join('mq_tasks_and_load_avg.log').to_s

begin
    log_fp = File.open(log_fn, 'a+')
    t = Time.now
    log_fp.puts t
    conn = Bunny.new.start
    ch = conn.channel
    q = ch.queue 'koj'
    while true do
        cnt = q.message_count
        load = `cat /proc/loadavg`
        log = "#{cnt} #{load}"
        puts log
        log_fp.puts log
        sleep 5
    end
rescue Interrupt => _
    log_fp.flush
    log_fp.close
    ch.close
    conn.close
end