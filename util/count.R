	net<- read.table(paste("result.txt",sep="\t"), header=T)

	reps<- length(net$status)

	util<-(net$in_0[net$status==1]+net$in_1[net$status==1])
	beta<-mean(util)
	prob_dead<- mean(1-net$status)
#	stdev<-sd(1-net$status)
#	ci<- stdev*1.96/sqrt(reps)

#	dl<-net$sim_time[net$status==0]
	fn<-net$sim_time[net$status==1]
#	m_dl<-mean(dl)
#	sd_dl<-sd(dl)
#	ci_dl<-sd_dl*1.96/sqrt(reps)
	MTTF<-mean(fn)
#	sd_MTTF<-sd(fn)
#	ci_MTTF<-sd_fn*1.96/sqrt(reps)

	cat(prob_dead, MTTF, beta,'\n',sep="\t",file='stat.tmp')
