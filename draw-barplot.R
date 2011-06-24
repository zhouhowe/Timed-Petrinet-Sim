postscript(file="plot%0d.eps",paper="special",width=12,height=10, onefile=F, horizontal=FALSE)
library(gplots)
dd<-read.table("stat/deadlock-net2.txt",header=T)
mp<- barplot2(dd$deadlock, names.arg=1:8, space=0.618, width=0.6, xlim=c(0,8), 
main="Deadlock Probability of Various Control Strategies", xlab="Control Strategy",
plot.ci=T, ci.l=dd$CI_L, ci.u=dd$CI_U, ci.lwd=2, plot.grid=T)

dd<-read.table("stat/runtime-net2.txt",header=T)
mp<- barplot2(dd$MTTD, names.arg=1:8, space=0.618, width=0.6, xlim=c(0,8), ylim=c(6,15), 
main="MTTD", xlab="Control Strategy", ylab="Time", xpd=F,
plot.ci=T, ci.l=dd$D_CI_L, ci.u=dd$D_CI_U, ci.lwd=2, plot.grid=T)

mp<- barplot2(dd$MTTF, names.arg=1:8, space=0.618, width=0.6, xlim=c(0,8), ylim=c(14,34),
main="MTTF", xlab="Control Strategy", ylab="Time", xpd=F,
plot.ci=T, ci.l=dd$F_CI_L, ci.u=dd$F_CI_U, ci.lwd=2, plot.grid=T)
dev.off()
