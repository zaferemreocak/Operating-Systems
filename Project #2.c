#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

//Initialize Semaphores
int full=0,empty=0;
sem_t fromReaderLine;
sem_t toReplacerLine;
sem_t fromReplacerLine;
sem_t toUpperLine;
sem_t fromUpperLine;
sem_t toWriterLine;

//Initialize Mutexes
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

//Struct Definition
struct arg_carrier{

    char *file;//POINTER TO KNOW ABOUT ARGV[1]
    char lineBuffer[256];//MAX LINE
    struct arg_carrier *next;//ACCESS NEXT ELEMENT
};
struct arg_carrier *head=NULL;//GLOBAL HEADER POINTER
FILE *write_file,*read_line;

//Function definitions
void *readFile(void *args);
void *replaceFile(void *args);
void *upperFile(void *args);
void *writerFile(void *args);


//Read file line by line with the help of read_thread
void *readFile(void *args){


    //Assign local pointer to incoming argument
    struct arg_carrier *read_it = (struct arg_carrier *) args;
    FILE *read_file = fopen(read_it->file, "r");
    write_file=fopen("/home/zaferemreocak/Masaüstü/output.txt","w");
    int i=0;
    //Check argv[1] if there is not
    if(read_file == NULL){
        fprintf(stderr, "ERROR! There is no input file!\n");
        exit(0);
    }
    char line[256];

    while(fgets(line, 256, read_file)){

        struct arg_carrier *newNode = (struct arg_carrier *) malloc(sizeof(struct arg_carrier));

        sem_wait(&toReplacerLine);

        //Critical Section started
        pthread_mutex_lock(&mutex1);
        if(head==NULL){

            head=newNode;
            head->file=NULL;
            strcpy(head->lineBuffer,line);
            read_it=newNode;
        }
        else{
            read_it=head;

            while(read_it->next != NULL){
                read_it=read_it->next;
            }

            read_it->next=newNode;
            newNode->file=NULL;
            newNode->next=NULL;
            strcpy(newNode->lineBuffer,line);
        }
        fprintf(write_file,"Reader-Thread\t");
        i=0;
        while(i<=strlen(line) && i<=63){
            fprintf(write_file,"%c",line[i]);
            i++;
        }
        i=0;
        fprintf(write_file,"\t***\n");
        //Critical Section Finished
        pthread_mutex_unlock(&mutex1);
        sem_post(&fromReaderLine);
    }

    pthread_exit(NULL);
}
//Replace spaces with - with the help of replace_thread
void *replaceFile(void *args){

    int i=0;
    //Assign local pointer to incoming argument
    struct arg_carrier *replace_it = (struct arg_carrier *) args;

    write_file = fopen("/home/zaferemreocak/Masaüstü/output.txt","w");

    //Critical Section started
    sem_wait(&fromReaderLine);//full
    sem_wait(&toUpperLine);//empty

    replace_it=head;

    while(replace_it/*->next*/ != NULL){

        pthread_mutex_lock(&mutex1);
        pthread_mutex_lock(&mutex2);

        fprintf(write_file,"\nReplace-Thread\t");

        while(i<=strlen(replace_it->lineBuffer) && i<=63){

            fprintf(write_file,"%c",replace_it->lineBuffer[i]);//Before change
            if(replace_it->lineBuffer[i] == ' '){
                replace_it->lineBuffer[i] = '-';
            }
            i++;
        }
        i=0;
        fprintf(write_file,"\t");
        while(i<=strlen(replace_it->lineBuffer) && i<=63){
            fprintf(write_file,"%c",replace_it->lineBuffer[i]);
            i++;
        }
        i=0;
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_unlock(&mutex1);
        replace_it=replace_it->next;

    }
    i=0;

    //Critical Section Finished
    sem_post(&toReplacerLine);
    sem_post(&fromReplacerLine);
    pthread_exit(NULL);
    //return NULL;

}

//Replace lower letters with upper letters with the help of upper_thread
void *upperFile(void *args){
    int i=0;

    //Assign local pointer to incoming argument
    struct arg_carrier *upper_it = (struct arg_carrier *) args;
    write_file=fopen("/home/zaferemreocak/Masaüstü/output.txt","w");
    //Critical Section started
    sem_wait(&fromReplacerLine);
    sem_wait(&toWriterLine);
    upper_it=head;

   while(upper_it/*->next*/ != NULL){

        pthread_mutex_lock(&mutex3);
        pthread_mutex_lock(&mutex2);

        fprintf(write_file,"Upper-Thread");
        fprintf(write_file,"\t");
        while(i<=strlen(upper_it->lineBuffer) && i<=63){
            fprintf(write_file,"%c",upper_it->lineBuffer[i]);//Before arrangement
            if(upper_it->lineBuffer[i] == 'a'){
                upper_it->lineBuffer[i] = 'A';

            }

            else if(upper_it->lineBuffer[i] == 'b'){
                upper_it->lineBuffer[i] = 'B';

            }

            else if(upper_it->lineBuffer[i] == 'c'){
                upper_it->lineBuffer[i] = 'C';

            }

            else if(upper_it->lineBuffer[i] == 'd'){
                upper_it->lineBuffer[i] = 'D';

            }

            else if(upper_it->lineBuffer[i] == 'e'){
                upper_it->lineBuffer[i] = 'E';

            }

            else if(upper_it->lineBuffer[i] == 'f'){
                upper_it->lineBuffer[i] = 'F';

            }

            else if(upper_it->lineBuffer[i] == 'g'){
                upper_it->lineBuffer[i] = 'G';

            }

            else if(upper_it->lineBuffer[i] == 'h'){
                upper_it->lineBuffer[i] = 'H';

            }

            else if(upper_it->lineBuffer[i] == 'i'){
                upper_it->lineBuffer[i] = 'I';

            }

            else if(upper_it->lineBuffer[i] == 'j'){
                upper_it->lineBuffer[i] = 'J';

            }

            else if(upper_it->lineBuffer[i] == 'k'){
                upper_it->lineBuffer[i] = 'K';

            }

            else if(upper_it->lineBuffer[i] == 'l'){
                upper_it->lineBuffer[i] = 'L';

            }

            else if(upper_it->lineBuffer[i] == 'm'){
                upper_it->lineBuffer[i] = 'M';

            }

            else if(upper_it->lineBuffer[i] == 'n'){
                upper_it->lineBuffer[i] = 'N';

            }

            else if(upper_it->lineBuffer[i] == 'o'){
                upper_it->lineBuffer[i] = 'O';

            }

            else if(upper_it->lineBuffer[i] == 'p'){
                upper_it->lineBuffer[i] = 'P';

            }
            else if(upper_it->lineBuffer[i] == 'q'){
                upper_it->lineBuffer[i] = 'Q';

            }

            else if(upper_it->lineBuffer[i] == 'r'){
                upper_it->lineBuffer[i] = 'R';

            }

            else if(upper_it->lineBuffer[i] == 's'){
                upper_it->lineBuffer[i] = 'S';

            }

            else if(upper_it->lineBuffer[i] == 't'){
                upper_it->lineBuffer[i] = 'T';

            }

            else if(upper_it->lineBuffer[i] == 'u'){
                upper_it->lineBuffer[i] = 'U';

            }

            else if(upper_it->lineBuffer[i] == 'v'){
                upper_it->lineBuffer[i] = 'V';

            }

            else if(upper_it->lineBuffer[i] == 'w'){
                upper_it->lineBuffer[i] = 'W';

            }

            else if(upper_it->lineBuffer[i] == 'x'){
                upper_it->lineBuffer[i] = 'X';

            }
            else if(upper_it->lineBuffer[i] == 'y'){
                upper_it->lineBuffer[i] = 'Y';

            }
            else if(upper_it->lineBuffer[i] == 'z'){
                upper_it->lineBuffer[i] = 'Z';

            }

            i++;
        }
        i=0;
        fprintf(write_file,"\n");
        while(i<=strlen(upper_it->lineBuffer) && i<=63){
            fprintf(write_file,"%c",upper_it->lineBuffer[i]);
            i++;
        }
        i=0;
        fprintf(write_file,"\t***");
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_unlock(&mutex3);
        upper_it=upper_it->next;

    }
    i=0;

    //Critical Section Finished
    sem_post(&toUpperLine);
    sem_post(&fromUpperLine);
    pthread_exit(NULL);
}

//Print the line to stdout with the help of writer_thread
void *writerFile(void *args){


    int i=0;

    //Assign local pointer to incoming argument
    struct arg_carrier *write_it = (struct arg_carrier *) args;

    //Critical Section started
    sem_wait(&fromUpperLine);
    write_it=head;

    while(write_it/*->next*/ != NULL){
        pthread_mutex_lock(&mutex3);

        fprintf(write_file,"Writer-Thread\t");
        while(i<=strlen(write_it->lineBuffer) && i<=63){
            fprintf(stdout,"%c",write_it->lineBuffer[i]);
            fprintf(write_file,"%c",write_it->lineBuffer[i]);
            i++;
        }

        i=0;
        fprintf(stdout,"\n");
        fprintf(write_file,"\n");
        fprintf(write_file,"\t***");
        pthread_mutex_unlock(&mutex3);
        write_it=write_it->next;

    }
    i=0;

    //Critical Section finished
    sem_post(&toWriterLine);
    pthread_exit(NULL);
}
//Main function to create all variables, semaphores, mutexes and destroy them
int main(int argc, char *argv[]){

    if(argv[1]==NULL){
        fprintf(stderr,"ERROR! There is no input file!");
        exit(0);
    }
    read_line=fopen(argv[1],"r");
    char test[256];int count=0;
    while(fgets(test,256,read_line)){
        count++;
    }
	empty=count+1;

    pthread_t read_thread;
    pthread_t replace_thread;
    pthread_t upper_thread;
    pthread_t writer_thread;

    sem_init(&fromReaderLine,0,full);//full=0
    sem_init(&toReplacerLine,0,empty);//empty=20

    sem_init(&fromReplacerLine,0,full);//full=0
    sem_init(&toUpperLine,0,empty);//empty=20

    sem_init(&fromUpperLine,0,full);//full=0
    sem_init(&toWriterLine,0,empty);//empty=20

    pthread_mutex_init(&mutex1,NULL);
    pthread_mutex_init(&mutex2,NULL);
    pthread_mutex_init(&mutex3,NULL);

    //Assign argv[1] to stuff
    struct arg_carrier *stuff = (struct arg_carrier *) malloc(sizeof(struct arg_carrier));
    stuff->file=argv[1];

    //Create child threads
    pthread_create(&read_thread, NULL, &readFile, (void *)stuff);
    pthread_create(&replace_thread, NULL, &replaceFile, (void *)stuff);
    pthread_create(&upper_thread, NULL, &upperFile, (void *)stuff);
    pthread_create(&writer_thread, NULL, &writerFile, (void *)stuff);



    //Wait for the child threads
    pthread_join(read_thread, NULL);
    pthread_join(replace_thread, NULL);
    pthread_join(upper_thread, NULL);
    pthread_join(writer_thread, NULL);

    //Destroy all created mutexes
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);

    //Destroy all created semaphores
    sem_destroy(&fromReaderLine);
    sem_destroy(&toReplacerLine);
    sem_destroy(&fromReplacerLine);
    sem_destroy(&toUpperLine);
    sem_destroy(&fromUpperLine);
    sem_destroy(&toWriterLine);

    free(head);//Deallocate memory
    fclose(write_file);//Close the file

    pthread_exit(NULL);//main exit

}

