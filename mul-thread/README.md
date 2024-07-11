# queue.c uthread.c preempt.c

## Summary

In this program, we first implemented queue.c to build a dynamic queue and its
functions that can make sure queue runs successfully. After implementing
queue.c, we then finished uthread.c using the queue structure and its functions.
Finally, preempt.c defines some of the functions inside private.h, so we used
signal library to implement it. Therefore, after implementating these codes,
user-level threads can run. 

## Implementation

The implementation of this program follows such logics:

1. finish queue.c using the hints in queue.h and test it by queue_tester_example.c 
2. finish uthread.c using the hints in uthread.h by making use of queue and the
functions in context.c, then test it by uthread_hello.c and uthread_yield.c
3. finish preempt.c and makefile

### queue.c implementation

The first step is to build a structure for queue. Queue is made up of node, so
we also need to define a sturcture of node. Node contains two part, one part
is data pointer `void * address`, and the other part is the pointer to the
next node. They are both pointer types and in order to make the the type look
clearer, we definie `node` as the structure's pointer type. As we have type
definied node, queue's structure contains front node, end node, and the queue's length. In queue.h, after building the `queue` structure, it definies the pointer of queue as `queue_t`, which is similar to the step in node.
Data inside of node can be any type, such as int or char. To create new node,
we allocate memory to new node and give the node data. 
The first function to implement after definition is `queue_create()`, we
create new queue_t and allocate memory to it. We initialize its front and end
node to NULL, and the length is also 0 temoporary. The next function to
implement is `queue_destroy()`, we only free the queue if there is no node inside. 
The next function to implement is `queue_enqueue()`, if the queue is NULL
right now, the node will be both the front node and the end node. If there is
other nodes already existing, it will be the end node and the previous end
node will connect to it. The queue length increments by 1.
Later, we follow the similar logic to implement `queue_dequeue()`, we will get
the first(oldest) node in the queue and let the second node to be the first
node. The queue_length also decrements by 1. After the reconnection, the space
for the oldest node will be freed. 
Then, `queue_delete()` implementation may be a little bit harder as it
requires a loop. I begins it by front and the loop compares from the second
node to the last node. If the value of `address` from node and argument `data`
are same, the node will be freed and the previous node will connect to the
deleted node's next node. The queue length decrements by 1.
In `queue_iterate()`, we will follow the loop from the first to the last two
node and do the function of the last node separaetely. The reason for using
`->next` is we only have one pointer to the next node in node structure
instead of two pointers to both previous and next nodes. After running `func`,
and the return value is equal to 1, we will write the current's node data
pointer to the `data` argument. 
`queue_length()` is simple, it only needs to return the current queue's
length.
For testing, we also adds a new function named `queue_print()`, which can
print the queue data if it is int type.

### Testing
Professor has provided queue_tester_example.c to test the case, it makes sure
`queue_create()` `queue_enqueue()` `queue_dequeue()` `queue_iterate()`
`queue_delete()` can succeed. In the beginning, when we cannot pass the last
case, we use printf in `inc_item()` and `find_item()` to check which step is
wrong. Finally, we solve it out and the problem is about pointer and pass the
case.

### uthread.c implementation phase 2

The implementation is relatively vague for me at first. Therefore, we first
build up a structure with only four elements:tid, state, stack, context and
return value. This is defineid as `uthread_tcb` Then, we define some global 
variables such as queue, uthread_tid, and current thread. In the beginning,
we only create a thread queue in `uthread_start()`. In `uthread_stop()`, we
will  only destroy the queue `queue` if the queue is empty. Therefore, we
start to implement `uthread_create()`. We allocate memory for `new_thread`,
and its context and stack. The state should be `ready`.
After initializing the context using `uthread_ctx_init()`, we enqueue the
thread to the thread queue. In giving value to the new thread, we realize we
need to give TID to the main thread, and other elements in `uthread_tcb` to
make it as the first thread in queue. 
In `uthread_self()`, we only need to return the tid of the current thread,
which is the global variable. In `uthread_yield()`, we do the
initialization of `next_thread` and `now_thread`, now_thread is equal to the
current_thread, and it should be put to the end of the current queue. Then,
we find the frist `ready` state in queue, if the next thread is not `ready`,
we will enqueue it and then dequeue the next one. If the next thread is
`ready`, the current thread will be turned to this thread and we use
`uthread_ctx_switch()` function in `context.c` to change the running function.
In `uthread_exit()`, if the current function finishes running, we will make
the thread's state become zombie so that it is waiting for its parent to collect and the thread's return_value will be got from function.

### Testing
In phase 2 running, by using professor's test case yield.c, we find out if a
function(thread2) finishes running, it may not go to next thread in queue(it
may not go to thread 3 and the main function), so we need to add
`uthread_yield()` in the end of the `uthread_exit()`function. 

### uthread.c implementation phase 3
In phase 3, we need to edit `uthread_join()`. We find out we need to add some
more elements in the uthread_tcb structure, bool `joined` and int
`parent_tid`. If there is a child thread and it is running or ready, the
parent thread will be blocked in the `uthread_join()`. Then we will assign
the parent tid to the child thread. Later, it goes to `uthread_yield()`,
after the child thread finishes running, in function `uthread_exit()`,  it
will collect the return value of its function and turns the parent thread's
state to `ready`. In order to get the thread process by using tid value, we
add a new function `find_tid()`, it will return 1 if the tid value of one
`uthrea_tcb*` in the queue is same to the desired value. In order to find it
one by one, we also use the function `queue_iterate()` and take `find_tid()`
as the argument `func`. The next time when the parent thread is running
`uthread_join()`, its child state has already been `zombie` so the parrent
collects it and frees its space.

### Testing
We know the two phases' results should be the same, but one problem has
interrupted us for a while. When I assign `parent_tid` the type of
`uthread_t` in `uthread_tcb` structure, and assign the initial value of it
`-1`, the comparison in `uthread_exit()` :`if(exit_thread->parent_tid != -1)`
will return error, `it is always true because of type limits`. Later we find
out the reason is `uthread_t` is an unsigned short int so we cannot simply
assign it to `-1`. Therefore, in order to make the comparison work, we change
the type of `parent_tid` to `int` so that it can work properly.

### makefile  
We use makefile to compile programs, which is faster and fundamentally more 
efficient. And after compiling, there are no errors and warnings. It would 
generate a static library archive named libuthread.a. It can be executed 
directly through other C files. In order to make it easier to modify the
content in the future, we use $@ $^ to make the code more
concise.

### preempt.c implementation phase 4
To solve uncooperative threads could keep the processing resource for 
themselves if they never call uthread_yield().We use sigaction and Alarm to
achieve it. In signhandle function, it receives alarm signals "SIGVTALRM" in 
order to call uthread_yield(). In preempt_start().We set the alarm by using 
struct itimerval,which will fire an alarm a hundred times per second.
In preempt_enable() and preempt_disable(), we use sigprocmask() check if a signal has arrived.
By blocking and unblocking signals, we can disable or enable preemption.

## Source
https://steemit.com/utopian-io/@luisrod/dynamic-queues-in-c-language (queue.c reference)

## License

Copyright 2022, Louise Li, Yuanhao Lu
