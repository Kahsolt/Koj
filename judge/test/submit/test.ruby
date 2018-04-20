#!/usr/bin/env ruby

print(gets.split.map {|e| e.to_i}.reduce(:+))
