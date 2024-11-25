require 'socket'

starttime = Process.clock_gettime(Process::CLOCK_MONOTONIC)

s = TCPSocket.new 'localhost', 8989

s.write("./tmp1/#{ARGV[0]}.c\n")

  s.each_line do |line|
    # process the line if needed
    # puts line
  end

  s.close

  endtime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
  elapsed = endtime - starttime
  puts "Elapsed: #{elapsed} (#{ARGV[0]})"
