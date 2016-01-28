//ECS150_program1.3.c
//Trevor Gudmundsen 999675892
//Robert Gonzales   912431466
//Natalia Sandoval  998436223

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FALSE 0
#define TRUE 1
#define PATH_MAX 512

//node struct to be used with Queue
typedef struct node {
    struct node * next;
    struct dirent * file;
    char * fullpath;
} node;

//Queue data structure implemented as a linked list
typedef struct Queue {
    node * head;
    node * tail;
} Queue;

//constructor-like function for allocating new nodes
node * node_new(struct dirent * nfile, char * fullp) {
    node * newnode = malloc(sizeof(node));
    newnode->file = nfile;
    newnode->fullpath = fullp;
    newnode->next = NULL;
    return newnode;
}

//return how many nodes are in queue
int length(Queue queue) {
    node * current = queue.head;
    if (current == NULL) return 0;
    int count = 1;
    while (current != queue.tail) {
        current = current->next;
        count++;
    }
    return count;
}

//push node pointer eq to the back of the queue
void enqueue(node * eq, Queue * queue) {
    if (queue->head == NULL) {
        queue->tail = queue->head = eq;
    }
    else {
        queue->tail->next = eq;
        queue->tail = eq;
    }
}

//pop off the head of the queue and return it
node * dequeue(Queue * queue) {
    node * ret = queue->head;
    if (queue->head == queue->tail) {
        queue->head = queue->tail = NULL;
    }
    else {
        queue->head = queue->head->next;
    }
    return ret;
}

//check if file specified by fullpath is a directory
int isDir(char * fullpath) {
    struct stat s;
    if (stat(fullpath, &s) != 0) perror("stat did not return 0");
    //mask the mode and test to see if it specifies a directory
    if (S_ISDIR(s.st_mode) != 0)  return TRUE;
    else return FALSE;
}

void BreadthFirstSearch(char * dirName, char * searchName) {
    //change to the correct directory if not already there
    //also print the starting directory
    if (dirName[0] == '/') {
        chdir(dirName);
        printf("%s\n", dirName);
    }
    else {
        printf("%s\n", realpath(dirName, NULL));
    }
    //initialize and open directory with error checking
    struct dirent * d;
    DIR * dp;
    if ((dp = opendir(dirName)) == NULL) {
        perror("failed to open directory");
        exit(1);
    }
    //enter the specified directory
    chdir(dirName);
    Queue queue;
    queue.head = queue.tail = NULL;
    //load starting dirents into queue
    while (d = readdir(dp)) {
        if (d->d_ino != 0 && strcmp(d->d_name, ".") && strcmp(d->d_name, "..")) { 
            //printf("%s\n", d->d_name);
            enqueue(node_new(d, realpath(d->d_name, NULL)), &queue);
        }
    }
    
    //loop to perform breadth first search
    while (length(queue) > 0) {
        int len = length(queue);
        //dequeue from the queue
        node * dq = dequeue(&queue);
        printf("%s\n", dq->fullpath);
        //compare the file name to searchName
        //exit the function if found
        if (strcmp(dq->file->d_name, searchName) == 0) {
            printf("file found\n");
            return;
        }
        //if the file is a directory enter it
        //and enqueue the files inside to the queue
        //return to the parent directory when finished
        if (isDir(dq->fullpath)) {
            //save the current working directory
            char temppath[PATH_MAX];
            getcwd(temppath, PATH_MAX);
            chdir(dq->fullpath);
            free(dq);
            //entries to enqueue from the Directory stream dstrm
            struct dirent * dIns;
            DIR * dstrm;
            if ((dstrm = opendir(dq->fullpath)) == NULL) {
                perror("failed to open directory");
            }
            //enqueue all files in directory
            while (dIns = readdir(dstrm)) {
                if ((dIns->d_ino != 0) && strcmp(dIns->d_name, ".") && strcmp(dIns->d_name, "..")) { 
                    enqueue(node_new(dIns, realpath(dIns->d_name, NULL)), &queue);
                }
            }
            //restore previous working directory
            chdir(temppath);
        }
    }
    printf("file not found\n");
}

int main(int argc, char * argv[]) {
    //call bfs using argv[1] as starting directory and argv[2] as
    //search parameter
    if (argc == 3) BreadthFirstSearch(argv[1], argv[2]);
    exit(0);
}

