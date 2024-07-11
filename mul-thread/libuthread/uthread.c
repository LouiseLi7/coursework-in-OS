#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "queue.h"
#include "private.h"
#include "uthread.h"

#define Max_size 100

/* TODO */

/*elements in uthread*/
typedef struct uthread_tcb {
	uthread_t tid;
	char state[Max_size];
	void *stack;
	uthread_ctx_t* context;
	int return_val;
	bool joined;
	int parent_tid;
}uthread_tcb;
//typedef struct uthread_tcb, including its TID, state, stack, context, return value, and the state of join or not

/*global variables*/
uthread_t uthread_tid = 0;
queue_t queue;
uthread_tcb* current_thread;


/*initialization of queue, and the initial thread, main*/
int uthread_start(int preempt)
{
	(void)preempt;
	queue = queue_create();
	
	uthread_tcb* init_thread;
	uthread_ctx_t* init_thread_context;
	init_thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	init_thread_context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));

	init_thread->tid = uthread_tid;
	strcpy(init_thread->state,"running");
	init_thread->context = init_thread_context;
	init_thread->return_val = -1;
	init_thread->stack = NULL;
	init_thread->joined = false;
	init_thread->parent_tid = -1;

	current_thread = init_thread;
	/*put it into the queue as the first thread*/
	queue_enqueue(queue,init_thread);

	if(queue_length(queue)==1){
		return 0;
	}
	return -1;
}

/*end of running when there is no thread*/
int uthread_stop(void)
{
	/* TODO */
	if(queue_destroy(queue)==0){
		return 0;
	}
	return -1;
}

/*create new thread*/
int uthread_create(uthread_func_t func)
{
	/* initialization */
	uthread_tcb* new_thread;
	uthread_ctx_t* new_thread_context;
	new_thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	new_thread_context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));

	/*creation failure*/
	if(new_thread==NULL||new_thread_context==NULL){
		return -1;
	}

	uthread_tid++;
	new_thread->tid = uthread_tid;
	strcpy(new_thread->state, "ready");
	new_thread->stack = uthread_ctx_alloc_stack();
	new_thread->return_val = -1;
	new_thread->joined = false;
	new_thread->parent_tid = -1;
	
	/*creation failure*/
	if(new_thread->stack == NULL){
		return -1;
	}

	/*store and initialize context, put the new thread into queue*/
	uthread_ctx_init(new_thread_context,new_thread->stack,func);
	new_thread->context = new_thread_context;
	queue_enqueue(queue,(void*)new_thread);

	return(new_thread->tid);
}

/*other threads should run first, put the current thread to the end of current queue*/
void uthread_yield(void)
{
	/*initialization*/
	uthread_tcb* next_thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	queue_dequeue(queue,(void**)&next_thread); //to get the next thread
	uthread_tcb* now_thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	
	now_thread = current_thread;//get current thread
	strcpy(now_thread->state, "ready"); //change state from running to ready
	queue_enqueue(queue,now_thread);//put it into the end
	while(queue_length(queue)>0){
		/*find the first thread in queue ready to run*/
		if(strncmp(next_thread->state,"ready",Max_size)==0){
			strcpy(next_thread->state, "running");
			current_thread = next_thread;
			/*change thread from now thread to next ready thread*/
			uthread_ctx_switch(now_thread->context,next_thread->context);
			break;
		}
		else{
			/*if next thread is not ready to run(blocked), get the next thread*/
			queue_enqueue(queue,next_thread);
			queue_dequeue(queue,(void**)&next_thread);
		}

	}
	
}

uthread_t uthread_self(void)
{
	/*if the current thread is not NULL, get its tid*/
	if(current_thread){
		return current_thread->tid;
	}

	return -1;
}

/*find tcb with same required tid in queue*/
static int find_tid(queue_t q, void* tcb_queue, void* t)
{
    uthread_tcb* tcb = (uthread_tcb*)tcb_queue;
	uthread_t* child_tcb = (uthread_t*)t;
    (void)q; //unused

    if (*child_tcb == tcb->tid){
		return 1;
	}
    return 0;
}

/*exit current thread*/
void uthread_exit(int retval)
{
	/* if a function finishes running, it will return an int value.*/
	uthread_tcb* exit_thread = current_thread;
	strcpy(exit_thread->state, "zombie");
	exit_thread->return_val = retval;
	
	/*if it has parent_tid, we will find its parent_tcb and change its state from blocked to ready*/
	if(exit_thread->parent_tid != -1){
		uthread_tcb* parent_tcb = (uthread_tcb*)malloc(sizeof(uthread_tcb));
		uthread_t parent_tid = exit_thread->parent_tid;
		queue_iterate(queue, find_tid, (void*)&parent_tid, (void**)&parent_tcb);
		strcpy(parent_tcb->state, "ready");
	}

	uthread_yield();
}


/*child thread runs first, parent thread waits for it*/
int uthread_join(uthread_t tid, int *retval)
{
	/*get parent/child tcb */
	uthread_tcb* parent_tcb = current_thread;
	uthread_tcb* child_tcb = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	queue_iterate(queue, find_tid, (void*)&tid, (void**)&child_tcb);	

	/*some conditions may return false*/
	if(tid==0||parent_tcb->tid==tid||child_tcb==NULL){
		return -1;
	}
	if(child_tcb->joined==true){
		return -1;
	}
	
	/*change state*/
	child_tcb->joined = true;

	while(queue_length(queue)>0){
		/*still be blocked if child tcb is ready or running*/
		if(strncmp(child_tcb->state,"running",Max_size)==0||strncmp(child_tcb->state,"ready",Max_size)==0){
			strcpy(parent_tcb->state,"blocked");
			child_tcb->parent_tid = parent_tcb->tid;
			uthread_yield();
		}
		else if(strncmp(child_tcb->state,"zombie",Max_size)==0){
			/*child tcb ends*/
			/*get return value*/
			*retval = child_tcb->return_val;
			/*delete the thread, free space, parent tcb ready to run*/
			queue_delete(queue,child_tcb);
			free(child_tcb->stack);
			free(child_tcb->context);
			free(child_tcb);
			break;
		}
		else{
			/*child gets other states*/
			printf("wrong state\n");
		}
	}

	return 0;
}
