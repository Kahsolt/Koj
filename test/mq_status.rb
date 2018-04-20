#!/usr/bin/ruby

require 'bunny'
conn=Bunny.new.start
ch=conn.channel
q=ch.queue 'koj'
puts q.status
ch.close
conn.close
