#!/usr/bin/python

# Run the petri-net simulation with various parameters
# Output:	a list with each row as following
# 		Param	Prob	MTTF	Util

# Usage: python batch.py net param1_num param2_num min max inc


import  os, sys, re, fileinput

distribution_pattern = re.compile(r'^([0-9\.]+)$')

distribution = []
results = set([])

net = sys.argv[1]
param1 = int(sys.argv[2])
param2 = int(sys.argv[3])
param_min = float(sys.argv[4])
param_max = float(sys.argv[5])
inc = float(sys.argv[6])


inputFile = open(net+"/distribution.txt","r")

for line in inputFile:
	match_result = distribution_pattern.match(line)
	if match_result:
		distribution.append( float(match_result.group(1)) )

inputFile.close()

os.rename(net+"/distribution.txt", net+"/dist.tmp")

num_param = int((param_max - param_min) / inc)

stat_fname= "stat-"+str(param1)+"-"+str(param2)+'.txt'
stat_file = open(stat_fname,"w")

p1_seq=[param_min+ inc*i for i in range(num_param)]
p2_seq=p1_seq

for p1 in p1_seq:
	for p2 in p2_seq:
		i=0
		print p1, p2
		outputFile = open(net+"/distribution.txt", "w")
		os.remove("result.txt")
		for lambda_ in distribution:
			i=i+1
			if i==param1:
				lambda_=p1
			elif i==param2:
				lambda_=p2
			outputFile.write(str(lambda_))
			outputFile.write("\n")

		outputFile.close()
		os.system('./petri-net 100000 2 '+net+' >>result.txt')
		stat_file.write(str(p1)+'\t'+str(p2)+'\t')
		os.system('R --no-save < count.R > /dev/null')
		ff=open('stat.tmp')
		stat_file.write(ff.read())
		ff.close()

stat_file.close()
os.rename(net+"/dist.tmp", net+"/distribution.txt")

