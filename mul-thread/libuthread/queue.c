#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "queue.h"


typedef struct node_ptr {
	void *address;         //address holds date
	struct node_ptr* next;
}node_ptr;
typedef struct node_ptr* node;

typedef struct queue {
	/* TODO */
	node front;          //the first element of queue
	node rear;           //the last element of queue
	int queue_length;    //the lenth of queue
}queue;

node new_node(void *data){
	node new;
	new = (node)malloc(sizeof(node_ptr));
	new->address = data;
	new->next = NULL;
	return new;
}
//Initialize queue
queue_t queue_create(void)
{
	queue_t queue_q;
	queue_q = (queue_t)malloc(sizeof(queue));
	queue_q->front = NULL;
	queue_q->rear = NULL;
	queue_q->queue_length = 0;
	return queue_q;
	/* TODO */
	return NULL;
}

int queue_destroy(queue_t queue)
{
	/* TODO */
	//in case of queue is not empty
	if(queue->front!=NULL){
		return -1;
	}
	else{
		free(queue);
		return 0;
	}
}
//add element to queue
int queue_enqueue(queue_t queue, void *data)
{
	/* TODO */
	if(!data){
		return -1;
	}

	node n = new_node(data);

	queue->queue_length = queue->queue_length + 1;       //after enqueue,the length of queue would increment 1
	//in case of queue is empty,if not empty the last element would point to new element
	if(queue->front==NULL){
		queue->front = n;
		queue->rear = n;
	}
	else{
		queue->rear->next = n;
		queue->rear = n;
	}

	return 0;
}
//dequeue and recieve the first element of queue
int queue_dequeue(queue_t queue, void **data)
{
	/* TODO */
	if(!data){
		return -1;
	}
	//in case of queue is empty
	if(queue->front==NULL){
		return -1;
	}
    //dequeue the first element,the second element would become first element
    //in case of the length of queue is 1,then Initialize queue
	node node_old = queue->front;
	*data = queue->front->address;
	if(queue->front->next!=NULL){
		queue->front = queue->front->next;
	}
	else{
		queue->front = NULL;
		queue->rear = NULL;
	}
	free(node_old);
	queue->queue_length = queue->queue_length -1;

	return 0;
}
//Delete a specific element in the queue
int queue_delete(queue_t queue, void *data)
{
	/* TODO */
	//in case of queue is empty
	if(!data||queue->front==NULL){
		return -1;
	}
    //in case of delete first element in the queue
	node find_data = queue->front;
	if(queue->front->address == data){
		queue->front = queue->front->next;
		free(find_data);
		queue->queue_length--;
		return 0;
	}
//delete element and the previous element points to the next element of deleted element
	while(find_data->next!=NULL){
		if(find_data->next->address == data){
			node find_it = find_data->next;
			if(find_data->next!=queue->rear){
				find_data->next = find_data->next->next;
			}
			else{
				queue->rear = find_data;
				find_data->next = NULL;
			}
			free(find_it);
			queue->queue_length--;
			return 0;
		}
		find_data = find_data->next;
	}
	return -1;
}
//iterate and function call
int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	/* TODO */
	//in caes of queue is empty and no function recieved
	if(queue->front==NULL||func==NULL){
		return -1;
	}

	int return_value;
	node current_data = queue->front;
//iterating until last element
	while(current_data->next!=NULL){
		return_value = func(queue,current_data->address,arg);   //get retuen value of function call
		if(return_value==1){
			if(data!=NULL){
				*data = current_data->address;
				return 0;
			}
		}
		current_data = current_data->next;
	}
	
	return_value = func(queue,current_data->address,arg);
	if(return_value==1){
		if(data!=NULL){
			*data = current_data->address;
		}
		return 0;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	/* TODO */
	if(queue->front==NULL){
		return -1;
	}
	else{
		return queue->queue_length;
	}
}

int queue_print(queue_t queue){
	node start = queue->front;
	while(start!=NULL){
		printf("print: %d\n",*(int*)start->address);
		start = start->next;
	}
	return 0;
}

