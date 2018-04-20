#!/usr/bin/ruby

require 'bunny'
conn=Bunny.new.start
ch=conn.channel
q=ch.queue 'koj'
q.purge
ch.close
conn.close
