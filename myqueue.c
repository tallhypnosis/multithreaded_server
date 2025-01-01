#include "myqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

node_t* head = NULL;
node_t* tail = NULL;

typdef struct {
  int *client_socket;
  struct timeval enqueue_time;
} Task;

double calculate_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

void enqueue(int *client_socket){
    pthread_mutex_lock(&my_mutex);
    Task new_task;
    new_task.client_socket = client_socket;

    gettimeofday(&new_task.enqueue_time, NULL);

    node_t* newnode = malloc(sizeof(node_t));
    newnode->task = new_task;
    newnode->next = NULL;
    if(tail == NULL){
        head = newnode;
    } else {
        tail->next = newnode;
    }
    tail = newnode;
    pthread_mutex_unlock(&my_mutex);
}

// returns NULL if the queue is empty
// returns the pointer to a client_socket, if there is one 
int *dequeue() {
    pthread_mutex_lock(&my_mutex);
    if(head == NULL){
        return NULL;
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    node_t *prev = NULL;
    node_t *current = head;
    node_t *timeout_prev = NULL;
    node_t *timeout_node = NULL;

    while(current != NULL) {
      double wait_time = (current_time.tv_sec - current->task.enqueue_time.tv-sec) + (current_time.tv_usec - current->task.enqueue_time.tv_usec) / 1e6;

      if(wait_time > 5.0) {
        timeout_node = current;
        timeout_prev = prev;
        break;
      }

      prev = current;
      current = current->next;
    }

    if (timeout_node != NULL) {
        if(timeout_prev == NULL) {
            head = timeout_node->next;
        } else {
              timeout_prev->next = timeout_node->next;
        }
        if (timeout_node == tail) {
            tail = timeout_prev;
        }

        int *result = timeout_node->task.client_socket;
        free(timeout_node);
        return result;
        }

        node_t *temp = head;
        int *result = temp->task.client_socket;

        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }
        free(temp);
        pthread_mutex_unlock(&my_mutex);
        return result;
}
