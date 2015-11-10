
str<-"./trim-output/"
filelist <- dir(path=str)
reps<-100000

cat("instance\tUtil%\tStdev\tCI-\tCI+\n", file="stat/util.txt", append=T)
for(filename in filelist) {
	net<- read.table(paste(str,filename,sep=""), header=T)
	fn_str<-unlist(strsplit(unlist(strsplit(filename, '\\.')[1]),'-'))
	row_name<-paste(fn_str[1],fn_str[3], sep="_")

	reps<- length(net$status)

	if(fn_str[1]=="net3" || fn_str[1]=="net3c") {
		util<-2-net$in_0[net$status==1]
	} else {
		util<-(2-net$in_0[net$status==1]-net$in_1[net$status==1])
	}

	m<- mean(util)
	stdev<-sd(util)
	ci<- stdev*1.96/sqrt(reps)


	cat(row_name,m,stdev,m-ci, m+ci,'\n', file="stat/util.txt",sep='\t', append=T)
}

