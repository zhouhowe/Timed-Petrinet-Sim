
str<-"./trim-output/"
filelist <- dir(path=str)
reps<-100000

cat("instance\tDeadlock%\tStdev\tCI-\tCI+\n", file="stat/deadlock.txt", append=T)
cat("instance\tMTTD\tSTDEV\tCI-\tCI+\tMTTF\tSTDEV\tCI-\tCI+\n",file="stat/runtime.txt", append=T)
for(filename in filelist) {
	net<- read.table(paste(str,filename,sep=""), header=T)
	fn_str<-unlist(strsplit(unlist(strsplit(filename, '\\.')[1]),'-'))
	row_name<-paste(fn_str[1],fn_str[3], sep="_")

	reps<-length(net$status)

	m<- mean(1-net$status)
	stdev<-sd(1-net$status)
	ci<- stdev*1.96/sqrt(reps)

	dl<-net$sim_time[net$status==0]
	fn<-net$sim_time[net$status==1]
	m_dl<-mean(dl)
	sd_dl<-sd(dl)
	ci_dl<-sd_dl*1.96/sqrt(reps)
	m_fn<-mean(fn)
	sd_fn<-sd(fn)
	ci_fn<-sd_fn*1.96/sqrt(reps)

	cat(row_name,m,stdev,m-ci, m+ci,'\n', file="stat/deadlock.txt",sep='\t', append=T)
	cat(row_name,m_dl,sd_dl,m_dl-ci_dl,m_dl+ci_dl, m_fn,sd_fn,m_fn-ci_fn,m_fn+ci_fn,'\n',file="stat/runtime.txt",sep='\t', append=T)
}

