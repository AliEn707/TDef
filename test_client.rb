require 'socket'      # Sockets are in standard library

#puts "hostname"
hostname = "localhost"#gets.chomp
#puts "port"
port = 7922#gets.to_i

s = TCPSocket.open(hostname, port)

#puts connected
s.write [99].pack("c")
s.write [4232].pack("l")
#s.write [1].pack("l")
p a=s.read(4).unpack("l")
p s.read(1)
#while line = s.gets   # Read lines from the socket
  #puts line.chop      # And print with platform line terminator
#end
s.close               # Close the socket when done