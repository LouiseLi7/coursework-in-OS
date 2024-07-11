#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include "private.h"
#include "uthread.h"
#include <time.h>
/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
struct sigaction sig;
struct itimerval timer;

void signhandler(int signal){
     //if signgal recieve 
     if(signal==SIGVTALRM){
     uthread_yield();
     }
   
}
void preempt_start(void)
{
	/* TODO */
	sigemptyset(&sig.sa_mask);
    sig.sa_handler = signhandler;
    sigaction(SIGVTALRM, &sig, NULL);
    
    //timer which will fire an alarm hundred times per second.
	timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = HZ*100;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = HZ*100;
    setitimer(ITIMER_VIRTUAL,&timer,NULL);

}

void preempt_stop(void)
{
	/* TODO */
	sigdelset(&sig.sa_mask,SIGVTALRM);
    
}

void preempt_enable(void)
{
	  /* TODO */
	 //Check if a signal has arrived in order to enable
     sigprocmask(SIG_UNBLOCK, &sig.sa_mask, NULL);
   ;
}

void preempt_disable(void)
{
	 /* TODO */
     printf("preempt has disable");
     //Check if a signal has arrived in order to disable
     sigprocmask(SIG_BLOCK, &sig.sa_mask, NULL);
     
}

