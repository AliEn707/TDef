m1=[]
s="config.gridsize"
name="AreaArray"
mass="config.area_array"
mass_s="config.area_size"
size=30

printf "#include \"grid.h\"\n\n" 
printf "#define tmp_size %s\n",mass_s
printf "#define tmp %s\n",mass 
printf "#define shift %s\n\n",s 
printf "void init%s(){\n",name

#printf "int * tmp[%d];\n",size-1
size.times do |i|
	shift=50
	m=[]
	0.step(100,1) do |x|
		0.step(100,1) do |y|
			if ((x-shift)**2+(y-shift)**2<+(i+0.5)**2)
				m<<[x-shift,y-shift]
			end
		end
	end
	if i!=0
		z=m-m1
		printf "\tveci tmp%d[%d]={",i-1,z.size 
		z.size.times do |k|
			printf "{%d,%d}",z[k][0],z[k][1]
			printf ", " if k!=z.size-1
		end
		printf "};\n"
#		printf "tmp[%d]=tmp%d;\n",i-1,i-1
		printf "\tif((tmp[%d]=malloc(sizeof(tmp%d)))==0)\n",i-1,i-1
		printf "\t\tperror(\"malloc tmp%d %s\");\n",i-1,name
		printf "\tmemcpy(tmp[%d],tmp%d,sizeof(tmp%d));\n",i-1,i-1,i-1
		printf "\ttmp_size[%d]=%d;\n\n",i-1,z.size
	end
	m1=m
end
printf "}\n\n"

printf "void realize%s(){\n",name
printf "\tint i;\n\tfor(i=0;i<%d;i++)\n\t\tfree(tmp[i]);\n",size

printf"}\n\n"
printf "#undef shift\n"
printf "#undef tmp\n"
