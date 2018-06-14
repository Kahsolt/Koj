#!/usr/bin/env ruby

require 'pp'
require 'json'
require 'bunny'
require 'socket'
require 'fiddle'
require 'iniparse'
require 'pathname'
require 'securerandom'
require 'byebug'

# paths
$LOAD_PATH.push(File.dirname(__FILE__))
KOJ_ROOT = Pathname(__FILE__).dirname.realpath
KOJ_PATH = {
    :conf => KOJ_ROOT.join('conf'),
    :test => KOJ_ROOT.join('test'),
    :data => KOJ_ROOT.join('data'),
    :log  => KOJ_ROOT.join('log'),
    :tmp  => KOJ_ROOT.join('tmp'),
    :lib  => KOJ_ROOT.join('judged').join('lib'),
}

# logger
$log_fn = KOJ_PATH[:log].join('judger.log').to_s
$logger = Logger.new($log_fn, 'daily')
$logger.progname = 'Judger'

# write pid file
File.open(KOJ_PATH[:log].join('judger.pid').to_s, 'wb') do |pid_pf|
  pid_pf.write(Process.pid)
  pid_pf.flush
  pid_pf.close
end

class Judger
  def initialize
    @uuid = SecureRandom.uuid
    @prof = open(KOJ_PATH[:log].join("#{@uuid}_task_roundtime.log").to_s, 'a+')

    # Folders
    KOJ_PATH.each_value { |dir| Dir.mkdir dir unless Dir.exist? dir }

    # Toolchain
    @toolchains = {}
    @languages = []
    lang_conf_dir = KOJ_PATH[:conf].join('languages')
    Dir.new(lang_conf_dir).each_entry do |conf_fn|
      next unless conf_fn.start_with? 'E', 'N'
      begin
        conf = IniParse.parse File.read lang_conf_dir.join(conf_fn).to_s
        section = conf[:Meta]
        @toolchains[section[:name]] = conf
        @languages << {name: section[:name], display_name: section[:display_name]}
      rescue
        if conf_fn.start_with? 'E'
          $logger.fatal "language config error for <#{conf_fn}>"
          exit
        else
          $logger.warn "language config error <#{conf_fn}>, safely ignore"
        end
      end
    end

    #  Data Sync & Message Queue Config
    conf = IniParse.parse File.read KOJ_PATH[:conf].join('judger.conf').to_s
    section = conf[:DataSync]
    sync_cmd = "scp -rpBC #{section[:user]}@#{section[:host]}:#{section[:data_dir]} #{KOJ_PATH[:data]}"
    $logger.warn 'error sync data dir' unless system sync_cmd

    section = conf[:MessageQueue]
    @connection = Bunny.new(
        :host => section[:host],
        :port => section[:port],
        :user => section[:user],
        :pass => section[:pass],
        :vhost=> section[:vhost]).start
    @channel = @connection.create_channel
    @channel.prefetch 3
    @queue = @channel.queue section[:queue], exclusive: false
    @exchange = @channel.default_exchange

    # ffi judge func
    lib = Fiddle.dlopen KOJ_PATH[:lib].join('libjudge.so').to_s
    @judge_ffi = Fiddle::Function.new(
        lib['judge_json_ret'],
        [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP],
        Fiddle::TYPE_VOIDP)
  end

  def start
    count = 0
    t = Thread.new do
      prof_fp = File.open(KOJ_PATH[:log].join("#{@uuid}_task_finished_10s.log"), 'a+')
      prof_fp.puts Time.now
      prof_fp.flush
      while true do
        cut_point = count
        sleep 10
        work_load = count - cut_point
        prof_fp.puts work_load
        prof_fp.flush
        puts ">> #{work_load}"
      end
    end
    t.abort_on_exception = true

    @queue.subscribe block: true, exclusive: false, manual_ack: true do |delivery_info, properties, body|
      starttime = Time.now
      data = JSON.parse body
      conf = @toolchains[data['lang']]
      if conf
        # prepare tmp work_dir
        submit_dir = KOJ_PATH[:tmp].join(data['uuid'])
        Dir.mkdir submit_dir unless Dir.exist? submit_dir
        Dir.chdir submit_dir

        # write src file
        src_fn = conf[:File][:src] || ''
        out_fn = conf[:File][:out] || ''
        File.open(src_fn, 'wb') do |src_fp|
          src_fp.write(data['src'])
          src_fp.flush
          src_fp.close
        end

        # init result
        result = {
            :result => 'SE',
            :time => nil,
            :memory => nil,
            :code_length => nil,
        }

        # compiler
        result[:code_length] = data['src'].length
        compiler_cmd = conf[:Compiler][:cmd]
        if compiler_cmd
          compiler_cmd = compiler_cmd.gsub('$src', src_fn).gsub('$out', out_fn)
          result[:result] = 'CE' unless system 'timeout -s KILL 5 nice -10 ' + compiler_cmd
        end

        # runner
        unless result[:result] == 'CE'
          exec_path = conf[:Runner][:exec] && conf[:Runner][:exec].gsub('$out', out_fn) || ''
          argv = conf[:Runner][:argv] ?
                     [Pathname(exec_path).basename] +
                         JSON.parse(conf[:Runner][:argv].gsub('$src', src_fn).gsub('$out', out_fn)) << nil :
                     [Pathname(exec_path).basename, nil]
          enpv = conf[:Runner][:envp] ?
                     JSON.parse(conf[:Runner][:envp]) << nil :
                     [nil]
          ans_path = submit_dir.join('ans').to_s
          data_dir = KOJ_PATH[:data].join(data['pid'].to_s)
          Dir.new(data_dir).each_entry do |in_fn|
            next unless in_fn.end_with? '.in'
            out_fn = in_fn.sub '.in', '.out'
            next unless File.file? data_dir.join(out_fn).to_s

            input_path  = data_dir.join(in_fn).to_s
            output_path = data_dir.join(out_fn).to_s
            task_json = {
                limit_time_cpu:  data['time'],
                limit_time_real: data['time'] * 3,
                limit_memory: data['memory'],
                limit_output: data['output'],
                limit_stack:  16 * 1024,  # 16M stack
                limit_process_number: 10,
                work_dir: submit_dir,
                src_path: '',
                compile_cmd: '',
                exe_path: exec_path,
                argv: argv,
                envp: enpv,
                input_path: input_path,
                output_path: output_path,
                error_path: '/dev/null',
                ans_path: ans_path,
            }.to_json

            res_json = @judge_ffi.call(task_json, $log_fn).to_s
            begin
              res = JSON.parse res_json
              result[:time] = res['time'] if result[:time].nil? or res['time'] > result[:time]
              result[:memory] = res['memory'] if result[:memory].nil? or res['memory'] > result[:memory]
              result[:result] = res['result']
              break unless res['result'] == 'AC'
            rescue
              $logger.error "json parse fail: #{res_json}"
            end
          end
        end

        # collect result and enqueue
        count += 1
        puts "[#{count}] #{result}"
        @exchange.publish result.to_json,
                          routing_key: properties.reply_to,
                          correlation_id: properties.correlation_id
        @channel.acknowledge delivery_info.delivery_tag, false

        # rm work_dir
        system "rm -rf #{submit_dir}"

        endtime = Time.now
        @prof.puts endtime - starttime
        @prof.flush
      else
        $logger.warn "unsupported language #{data['lang']}"
      end
    end
  end

  def stop
    @channel.close
    @connection.close
    @prof.close
  end

  def status
    @status = {
        :id           => @uuid,
        :hostname     => `hostname`.chomp,
        :os           => `lsb_release -d | cut -f2`.chomp,
        :arch         => `arch`.chomp,
        :kernel       => `uname -r`.chomp,
        :processor    => `cat /proc/cpuinfo | grep processor -c`.to_i,
    } unless @status
    @status[:memory], @status[:memory_free], @status[:swap], @status[:swap_free] =
        `cat /proc/meminfo | grep -E 'MemTotal|MemAvailable|SwapTotal|SwapFree' | cut -f2 -d:`
            .split("\n").map {|e| e.to_i / 1024}
    @status[:load_avg] = `cat /proc/loadavg`.split[0..2].map! {|e| e.to_f}

    @status
  end

  def debug
    puts '=' * 20 + '[Koj Judger]' + '=' * 20
    puts '[System Status]'
    pp   self.status
    puts '[Language Toolchains]'
    pp   @languages.map {|lang| lang[:display_name]}
    puts '[Message Queue]'
    puts [@connection, @channel, @queue, @exchange]
    puts '=' * (40 + 12)
    puts 'Ctrl + C to exit'
  end
end

begin
  server = Judger.new
  server.debug
  server.start
rescue Interrupt => _
  server.stop
end
