size=readline.to_i
walk=""
build=""
(size**2).times do
	w,b=readline.split.map{|i| i.to_i}
	walk+='1' if w>0
	walk+='0' if w==0
	walk+='-' if w<0
	build+='1' if b>0
	build+='0' if b==0
	build+='-' if b<0
	
end
puts walk,build