#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "private.h"



int thread1(void)
{
	printf("test1!\n");
	return 0;
}
int main(void)
{
	
    preempt_start();
    preempt_disable();
    preempt_enable();
    printf("test successful!\n");
	
	
	return 0;
}