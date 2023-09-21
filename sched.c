/* Calvin Aduma */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct Task {
    int task_id; /* alphabetic tid 'A'+(task_counter++) */
    int arrival_time, service_time;
    int remaining_time, completion_time, response_time;
    int wait_time;
    struct Task *next;
} Task;

void readFile( Task ** );
void FIFO( char *, Task ** );
void SJF( char *, Task ** );
void RR( char *, Task **);
void part1( char *, Task ** );
void part2( char *, Task ** );
void part3( Task ** );
void find_Wait_Response_Completion_Time( char *, Task ** );
void ServiceTime_BubbleSort( Task ** );
void ArrivalServiceTime_BubbleSort( Task ** );
void TaskID_BubbleSort( Task ** );
void SJF_BubbleSort( Task ** );
void swapLinkedList( Task *, Task * );
void insertLinkedList( Task *, Task ** );
Task* deletedLinkedList( Task * );
void printLinkedList(Task *);
void increaseWaitTimeInQueue( Task* );

//     0     1   2  3  4   5  
// ./a.out -fifo < in2 > out2 
int main( int argc, char* argv[] ){
    Task *tasks = NULL; //( Task* )malloc( sizeof( Task) );
    int tries = 3;
    char *input = argv[1];

    // command line arg check
    while( tries > 0 ){
        if ( strcmp( input,"-fifo" ) == 0 || strcmp( input,"-sjf" ) == 0 || strcmp( input,"-rr" ) == 0){
            fprintf(stderr,"You have to enter one of the following schedulers: \"-fifo\" \"-sjf\" \"-rr\"\n");
            fprintf(stderr, "Attempts Remaining: %d | Re-enter: ", tries );
            scanf( "%[^\n]",input );
        } else { 
            break;
        }
        tries--;
        if ( tries == 0 ) exit(0);
    }

    // read input from file and return number of tasks
    readFile( &tasks );

    // FIFO
    if( strcmp( input,"-fifo" ) == 0 ) FIFO( input, &tasks );
    
    // SJF
    else if( strcmp( input,"-sjf" ) == 0 ) SJF( input, &tasks );
    
    // RR
    else if( strcmp( input,"-rr" ) == 0 ) RR( input, &tasks );
    
    free ( tasks );
    exit( 0 );
}

void readFile( Task **tasks ){
    char taskID = 'A';
    char *line;
    char lineBuffer[ 50 ];
    while( 1 ){
        Task *newTask = ( Task* )malloc( sizeof( Task ) );
        newTask->next = NULL;
        newTask->task_id = ( int )taskID - 64; // subtract 64 to start ID at 1, because 'A' = 65

        // gets line by line from file
        line = fgets( lineBuffer, sizeof( lineBuffer ), stdin );
        // end of file
        if( !line || strcmp( line,"\n" ) == 0 ) break;
        
        // reads line into variable
        sscanf( line, "%d %d", &newTask->arrival_time, &newTask->service_time );
       
        // inserts to linked list
        if( *tasks == NULL ) *tasks = newTask;
        else {
            Task *temp = *tasks;
            while ( temp->next != NULL ) temp = temp->next;
            temp->next = newTask;
        }
        taskID++;
    }
    return;
}

void FIFO( char* scheduler, Task** taskList ){
    printf( "FIFO Scheduling Results\n\n" );
    part1( scheduler, taskList );
    part2( scheduler, taskList );
    part3( taskList );
}

void SJF( char* scheduler, Task** taskList ){
    printf( "SJF(Preemptive) Scheduling Results\n\n" );
    part1( scheduler, taskList );
    part2( scheduler, taskList );
    part3(  taskList );
}

void RR( char* scheduler, Task** taskList ){
    printf(  "RR Scheduling Results (time slice is 1) Scheduling Results\n\n" );
    part2( scheduler, taskList );
    part3( taskList );
}

void part1( char* scheduler, Task** taskList ){
    Task* current = *taskList;
    Task* Running = NULL;
    Task* Queue = NULL;
    printf( "time\tcpu\tready queue (tid/rst)\n- - - - - - - - - - - - - - - - - - - - \n");
    int time = 0;
    bool on = true;

    while( on ){
        printf( " %2d\t ", time);
        if( current != NULL && time == current->arrival_time ){
            current->remaining_time = current->service_time;
            if( Running == NULL ) {
                // insert into running queue if it is empty
                insertLinkedList( current, &Running );  
            }else{
                // puts new process at front of
                insertLinkedList( current, &Queue );
            }
            current = current->next;
        }
        // case where there is more than one process arriving at the same time
        while( current != NULL && time == current->arrival_time ){
            current->remaining_time = current->service_time;
            insertLinkedList( current, &Queue );
            current = current->next;
        }
        // for sjf, sort queue
        if( Queue != NULL && strcmp( scheduler,"-sjf" ) == 0 ){
            ServiceTime_BubbleSort( &Queue );
        }
        // aesthetics
        if( Running == NULL && Queue == NULL && current != NULL ) {
            printf( "\t\t- -");
        }
        // case where processes are in running queue and wait queue
        if( Running != NULL && Queue != NULL ){
            printf( "%c%d\t",Running->task_id+64,Running->remaining_time);
            Task* temp = Queue;
            while (temp!=NULL){
                printf( "%c%d, ",temp->task_id+64,temp->remaining_time);
                temp = temp->next;
            }
            Running->remaining_time--;
            increaseWaitTimeInQueue( Queue );
        // case where running queue is not empty but waiting queue is 
        }else if( Running != NULL && Queue == NULL ){
            printf( "%c%d\t- -",Running->task_id+64,Running->remaining_time);
            Running->remaining_time--;
        }
        // fifo scheduler part
        if( Running != NULL && Running->remaining_time == 0 ){
            Running = deletedLinkedList( Running );
            if( Queue != NULL ){
                insertLinkedList( Queue, &Running );
                Queue = deletedLinkedList ( Queue );
            }
        }
        time++;
        if( current == NULL && Queue == NULL && Running == NULL ){
            // ends loop when all queues and task is empty
            on = false;
            printf( "\n" );
        }
        printf( "\n");
    }
}
 
void part2( char* scheduler, Task** taskList ){
    // FIFO
    if ( strcmp( scheduler,"-fifo" ) == 0 ){       
        find_Wait_Response_Completion_Time( scheduler, taskList );
    // SJF
    } else if ( strcmp( scheduler,"-sjf" ) == 0 ){     
        find_Wait_Response_Completion_Time( scheduler, taskList );    
    // RR
    } else if (strcmp( scheduler,"-rr" ) == 0 ){       
        printf( "time\tcpu\tready queue (tid/rst)\n- - - - - - - - - - - - - - - - - - - - \n");
        find_Wait_Response_Completion_Time( scheduler, taskList );
    }
    printf( "\t\tarrival  service\tcompletion\tresponse  wait\ntid\t time\t\t time\t\t  time\t\t  time\t time\n" );
    printf(  "- - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    TaskID_BubbleSort( taskList );
    Task *current = *taskList;
    while ( current != NULL ){
        printf( " %c\t\t  %2d\t\t  %2d\t\t   %2d\t\t\t  %2d\t\t  %2d\n", current->task_id+64, current->arrival_time, current->service_time, current->completion_time, current->response_time, current->wait_time );
        current = current->next;
    }
    printf( "\n" );
}

void part3( Task** taskList ){
    ServiceTime_BubbleSort( taskList );
    printf( "service\twait\n time\t\ttime\n- - - - - - -\n" );
    Task *current = *taskList;
    while ( current != NULL ){
        printf( "  %2d\t\t %2d\n", current->service_time, current->wait_time );
        current = current->next;
    }
}

void find_Wait_Response_Completion_Time( char* scheduler, Task **taskList ){
    Task *current = *taskList;
    // base cases
    current->wait_time = 0;
    current->response_time = current->wait_time + current->service_time;
    current->completion_time = current->wait_time + current->arrival_time + current->service_time;
    if ( strcmp( scheduler,"-fifo" ) == 0 ){
        while ( current->next != NULL ){
            int current_completion_time = current->completion_time;
            if ( current_completion_time > current->next->arrival_time ) current->next->wait_time = current->completion_time - current->next->arrival_time;
            else current->next->wait_time = 0;
            current->next->response_time = current->next->wait_time + current->next->service_time;
            current->next->completion_time = current->next->wait_time + current->next->arrival_time + current->next->service_time;
            current = current->next;
        }
    }else if( strcmp( scheduler,"-sjf" ) == 0 ){
        Task* ReadyQueue = NULL;
        Task* TempQueue = NULL;
        Task* newList = NULL; 
        insertLinkedList( current, &newList );
        current = (*taskList)->next;
        int completion_time = newList->completion_time;

        while ( current != NULL ){
            // first case where completion time is greater
            if (completion_time > current->next->arrival_time){
                while ( current != NULL && completion_time > current->arrival_time ){
                    insertLinkedList( current, &ReadyQueue );
                    current = current->next;
                }
                // sort by service time only
                ServiceTime_BubbleSort( &ReadyQueue );
            } 
            // second case where ecompletion time is lesser
            if ( current != NULL && completion_time < current->arrival_time ){
                while ( current != NULL && completion_time < current->arrival_time ){
                    insertLinkedList( current, &TempQueue );
                    current = current->next;
                }
                // sort by arrival time then service time
                ArrivalServiceTime_BubbleSort( &TempQueue );
            }
            // process processes that are priority first
            while ( ReadyQueue != NULL ){
                ReadyQueue->wait_time = completion_time - ReadyQueue->arrival_time;
                ReadyQueue->completion_time = ReadyQueue->arrival_time + ReadyQueue->service_time + ReadyQueue->wait_time;
                ReadyQueue->response_time = ReadyQueue->wait_time + ReadyQueue->service_time;
                completion_time = ReadyQueue->completion_time;
                insertLinkedList( ReadyQueue, &newList );
                ReadyQueue = deletedLinkedList( ReadyQueue );  
            }
            bool first = true;
            // process processes after priority has finished
            while ( TempQueue != NULL ){
                if (first) {
                    TempQueue->wait_time = 0;
                    first = false;
                } else TempQueue->wait_time = completion_time - TempQueue->arrival_time;
                TempQueue->completion_time = TempQueue->arrival_time + TempQueue->service_time;
                TempQueue->response_time = TempQueue->wait_time + TempQueue->service_time;
                insertLinkedList( TempQueue, &newList );
                completion_time = TempQueue->completion_time;
                TempQueue = deletedLinkedList( TempQueue );
            }
        }

        (*taskList) = newList;
    }else if( strcmp( scheduler,"-rr" ) == 0){
        int time_quantum = 1;
        Task* Queue = NULL;
        Task* Running = NULL;
        Task* newList = NULL;
        int time = 0;
        int turn;
        bool on = true;
        while( on ){
            turn = 0;
            // loop for time slice
            while( turn < time_quantum ){
                printf( " %2d\t ", time);
                // adds new process to queue or running queue
                if( current != NULL && time == current->arrival_time ){
                    current->remaining_time = current->service_time;
                    if( Running == NULL ) {
                        insertLinkedList( current, &Running );                       
                    }else{
                        // puts new process at front of
                        insertLinkedList( current, &Queue );
                        // puts runing process into queue
                        insertLinkedList( Running, &Queue );
                        // empties running queue
                        Running = deletedLinkedList( Running );
                        // adds top of queue to runnning queue
                        insertLinkedList( Queue, &Running);
                        // removes top of queueu
                        Queue = deletedLinkedList( Queue );
                    }
                    current = current->next;
                }
                // case where there is more than one process arriving at the same time
                while( current != NULL && time == current->arrival_time ){
                    current->remaining_time = current->service_time;
                    insertLinkedList( current, &Queue );
                    current = current->next;
                }
                // aesthetics
                if( Running == NULL && Queue == NULL && current != NULL ) {
                    printf( "\t\t- -");
                }
                // case where processes are in running queue and wait queue
                if( Running != NULL && Queue != NULL ){
                    printf( "%c%d\t",Running->task_id+64,Running->remaining_time);
                    Task* temp = Queue;
                    while (temp!=NULL){
                        printf( "%c%d, ",temp->task_id+64,temp->remaining_time);
                        temp = temp->next;
                    }
                    Running->remaining_time--;
                    increaseWaitTimeInQueue( Queue );
                // case where running queue is not empty but waiting queue is 
                }else if( Running != NULL && Queue == NULL ){
                    printf( "%c%d\t- -",Running->task_id+64,Running->remaining_time);
                    Running->remaining_time--;
                }
                // swap of elements in running and queue for RR
                if( Running != NULL && Queue != NULL && Running->remaining_time != 0 ){
                    // future look ahead. If new process is coming in next, dont swap 
                    if( current != NULL && time+1 == current->arrival_time ){
                        turn++;
                        printf( "\n");
                        break;
                    }
                    // insert running process to end of queue
                    insertLinkedList( Running, &Queue );
                    // insert top of queue to running queue
                    insertLinkedList( Queue, &Running );
                    // delete top of both queues
                    Running = deletedLinkedList( Running );
                    Queue = deletedLinkedList( Queue );
                // case where a process is finished
                }else if( Running != NULL && Running->remaining_time == 0 ){
                    insertLinkedList( Running, &newList );
                    // remove if equal signifies end of process
                    Running = deletedLinkedList( Running );
                    // if process still in queue, insert into running queue
                    if( Queue != NULL ){
                        insertLinkedList( Queue, &Running );
                        Queue = deletedLinkedList( Queue );
                    }
                }
                turn++;
                printf( "\n");
            }
            time++;
            if( current == NULL && Queue == NULL && Running == NULL ){
                // ends loop when all queues and task is empty
                on = false;
                printf( "\n" );
            }
        }
        current = newList;
        while( current != NULL ){
            current->response_time = current->wait_time + current->service_time;
            current->completion_time = current->response_time + current->arrival_time;
            current = current->next;
        }
        *taskList = newList;
    }
    return;
}

// sort by service time ascending
void ServiceTime_BubbleSort( Task** taskList ){
    Task *p1 = *taskList;
    Task *p2 = NULL;
    while ( p1->next != NULL ){
        p2 = p1->next;
        while ( p2 != NULL ){
            if ( p1->service_time > p2->service_time ) swapLinkedList( p1, p2 );    
            if ( p2->next == NULL) break;
            p2 = p2->next;
        }
        p1 = p1->next;
    }
}

// sort by arrival time then by service time ascending
void ArrivalServiceTime_BubbleSort( Task** taskList ){
    Task *head = *taskList;
    Task *p1 = head;
    Task *p2 = NULL;
    while ( p1->next != NULL ){
        p2 = p1->next;
        while ( p2 != NULL ){
            if ( p1->arrival_time > p2->arrival_time ) swapLinkedList( p1, p2 );  
            else if ( p1->arrival_time == p2->arrival_time ){
                if ( p1->service_time > p2->service_time ) 
                    swapLinkedList( p1, p2 );  
            }
            if ( p2->next == NULL) break;
            p2 = p2->next;
        }
        p1 = p1->next;
    }
}

// sort by TaskID ascending order
void TaskID_BubbleSort( Task** taskList ){
    Task *head = *taskList;
    Task *p1 = head;
    Task *p2 = NULL;
    while ( p1->next != NULL ){
        p2 = p1->next;
        while ( p2 != NULL ){
            if ( p1->task_id > p2->task_id ) swapLinkedList( p1, p2 );   
            if ( p2->next == NULL) break;
            p2 = p2->next;
        }
        p1 = p1->next;
    }
}

void SJF_BubbleSort( Task** taskList){
    Task* p1 = *taskList;
    Task* p2 = NULL;
    if (p1->next != NULL) p2 = p1->next;
    else return;
    while ( p2 != NULL ){
        p2 = p1->next;
        if ( p1->arrival_time == p2->arrival_time )
            if ( p1->service_time > p2->service_time )
                swapLinkedList( p1, p2 );
        if ( p2->next == NULL) break;
        p2 = p2->next;
    }
}

void swapLinkedList(Task* a, Task* b){
    int a_task_id, a_arrival_time, a_service_time, a_remaining_time, a_completion_time, a_response_time, a_wait_time;
    a_task_id = a->task_id;
    a_arrival_time = a->arrival_time;
    a_service_time = a->service_time;
    a_remaining_time = a->remaining_time;
    a_completion_time = a->completion_time;
    a_response_time = a->response_time;
    a_wait_time = a->wait_time;
    a->task_id = b->task_id;
    a->arrival_time = b->arrival_time;
    a->service_time = b->service_time;
    a->remaining_time = b->remaining_time;
    a->completion_time = b->completion_time;
    a->response_time = b->response_time;
    a->wait_time = b->wait_time;
    b->task_id = a_task_id;
    b->arrival_time = a_arrival_time;
    b->service_time = a_service_time;
    b->remaining_time = a_remaining_time;
    b->completion_time = a_completion_time;
    b->response_time = a_response_time;
    b->wait_time = a_wait_time;
}

// adds segment onto end  of linked list
void insertLinkedList( Task* element, Task** List ){
    Task* newElement = ( Task* )malloc( sizeof( Task ) );
    newElement->task_id = element->task_id;
    newElement->arrival_time = element->arrival_time;
    newElement->service_time = element->service_time;
    newElement->completion_time = element->completion_time;
    newElement->response_time = element->response_time;
    newElement->wait_time = element->wait_time;
    newElement->next = NULL;
    newElement->remaining_time = element->remaining_time;
    if ( *List == NULL ){
        *List = newElement;
    } else {
        Task *temp = *List;
        while ( temp->next != NULL ) temp = temp->next;
        temp->next = newElement;
    }   
}

// deleted from front of linked list
Task* deletedLinkedList( Task* element ){
    return element->next;
}

void printLinkedList(Task* list){
    while (list!=NULL) {
        printf("%c %2d\n", 64+list->task_id, list->wait_time);
        list = list->next;
    }
}

void increaseWaitTimeInQueue( Task* Queue ){
    Task* current = Queue;
    while( current != NULL ){
        current->wait_time++;
        current = current->next;
    }
}

