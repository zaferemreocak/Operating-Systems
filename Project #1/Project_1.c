#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_CHILDPROCESS 256

struct histNode{
    struct histNode *nextHistNode;
    char args[10][MAX_LINE/2 +1];
    int number;
};
struct pipeNode{
    struct pipeNode *next;
    char *args[MAX_LINE];
};

void systemCommands(char *args[], int counter, int background);
void builtInCommands(char *args[]);
void pipeCommands(struct pipeNode *command);
void changeNode(struct pipeNode **head,char *args[]);
void addCommandIntoNode(struct pipeNode *head,char *args[]);
int histNum = 10; //Default number of commands of history buffer, which can be set later.
int bg=0;
void addToHist(char *args[]);
void callHist(char *args[]);
typedef struct histNode *histNP;
histNP firstHist=NULL;
int check_background();

int myct=0;//total element
int pipIn=-1;//stores where pipe symbol is
int np=0;//total # of pipes
int parentID=0;
int argIndex=0;//To use locate args's elements
char lastp[64];
char *launch[1];

//char cwd[1024];

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	//fprintf(stderr,">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */
     myct=ct;



	//for (i = 0; i <= ct; i++)
		//printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

void builtInCommands(char *args[]){
    //Initialize the current path and backward to reach previous path
    char cwd[1024];
    char backward[250];
    strcpy(cwd,getcwd(cwd,sizeof(cwd)));
    //CD
    if(strcmp(args[0],"cd") == 0){
        if(args[1] != NULL && args[1][0] == '/'){
            chdir(args[1]);
        }
        else if(args[1] != NULL && args[1][0] != '/' && args[1][0] != '.'){
            char *newPath = (char *) malloc(1 + strlen(cwd)+ strlen(args[1]) );
            strcpy(backward,cwd);
            strcpy(newPath, cwd);
            strcat(newPath, "/");
            strcat(newPath, args[1]);
            chdir(newPath);
        }
        else if(args[1] != NULL && strcmp(args[1],"..") == 0){
            chdir("..");
        }
        else{
            chdir(getenv("HOME"));
        }

    }
    //DIR
    else if(strcmp(args[0],"dir") == 0){
        fprintf(stderr,"%s\n",getcwd(cwd,sizeof(cwd)));
    }
    //CLEAR
    else if(strcmp(args[0],"clr") == 0){
        system("clear");
    }
    //WAIT
    else if(strcmp(args[0],"wait")==0){
        char prep[64];int nbg=0;

        while(1){
            strcpy(prep,lastp);
            nbg=check_background();

            if(strcmp(prep,lastp) != 0 && nbg == 1){
                return;
            }
            else{
                fprintf(stderr,"Waiting...\n");
                wait(NULL);
            }
        }
    }
    //HIST
    else if(strcmp(args[0],"hist")==0 || strcmp(args[0],"!")==0){
        callHist(args);
    }
    //EXIT
    else if(strcmp(args[0],"exit")==0){
        if(check_background() > 1){
            fprintf(stderr,"Warning, there are still background process available...\n");
        }
        else{
            exit(0);
        }
    }
}

void callHist(char *args[]){
    char *tempargs[(MAX_LINE/2)+1];//To pass the command from firstHist->args to type char *args
    //hist or !
        if(strcmp(args[0],"hist") == 0  && args[1] == NULL){ //hist
            int n=0,c=1; //n tek bir nodedaki argumanlari ilerletiyor. c ise historyde ne kadar ileri gidilecegi.
            histNP currentNode = firstHist;

            while(currentNode != NULL){
                if(c>histNum)
                    break;
                while(n<10){
                    fprintf(stderr,"%s ",currentNode->args[n]);
                    n++;
                }
                fprintf(stderr,"\n");
                n=0;
                c++;
                currentNode=currentNode->nextHistNode;
            }
        }
        //hist ise ve 1.arguman bos degilse ve 'set' ise
        else if(strcmp(args[0],"hist") == 0 && args[1] != NULL && strcmp(args[1],"-set") == 0 && args[2] != NULL){
            histNum = atoi(args[2]);
            //fprintf(stderr,"\nhistNum set to : %d\n",histNum);
        }
        else if(strcmp(args[0],"!") == 0 ) { // ! number
            int o=0,n=0;
            int number = atoi(args[1]);
            if(strcmp(args[1],"-1") == 0){//-1 ise en son cagrilan komutu cagiracak.
                while(n<10){
                    fprintf(stderr,"%s ",firstHist->args[n]);
                    n++;
                }
                fprintf(stderr,"\n");
                while(firstHist->args[o][0]!='\0'){
                        tempargs[o]=firstHist->args[o];
                        o++;
                    }
                    o=0;
//                if((strcmp(firstHist->args[0],"cd") == 0) || (strcmp(firstHist->args[0],"dir") == 0) || (strcmp(firstHist->args[0],"clr") == 0)
//                   || (strcmp(firstHist->args[0],"wait") == 0) || (strcmp(firstHist->args[0],"hist") == 0) || (strcmp(firstHist->args[0],"!") == 0) || (strcmp(firstHist->args[0],"exit") == 0)){
//                   //CALL BUILT IN COMMANDS
//                    //addToHist(args);
//                    //builtInCommands(tempargs);
//                }else if( (firstHist->args[0] != NULL) || (firstHist->args[1] != NULL && strcmp(firstHist->args[0],"|") != 0) ){
//                    //CALL SYSTEM COMMANDS
//                    //addToHist(args);
//                    //systemCommands(tempargs,myct,bg);
//                }
            }else if (number > 0 && number <= histNum){ // -1 degilse ve numbersa
                histNP currentNode = firstHist;
                while (number != currentNode->number ) { //linnked list icinde giderken aranan sayiya gidene kadar ilerlenir
                    if (currentNode->nextHistNode != NULL) {
                        currentNode = currentNode->nextHistNode;
                    }
                    else //listenin sonuna (basina) geldiyse
                        break;
            }
            while(currentNode->args[o][0]!='\0'){
                        tempargs[o]=currentNode->args[o];
                        o++;
                    }
                    o=0;
//                if((strcmp(currentNode->args[0],"cd") == 0) || (strcmp(currentNode->args[0],"dir") == 0) || (strcmp(currentNode->args[0],"clr") == 0)
//                   || (strcmp(currentNode->args[0],"wait") == 0) || (strcmp(currentNode->args[0],"hist") == 0) || (strcmp(currentNode->args[0],"!") == 0) || (strcmp(currentNode->args[0],"exit") == 0)){
//                    //CALL BUILT IN COMMANDS
//                    //addToHist(args);
//                    //builtInCommands(tempargs);
//                }else if( (args[0] != NULL) || (args[1] != NULL && strcmp(currentNode->args[0],"|") != 0) ){
//                    //CALL SYSTEM COMMANDS
//                    //addToHist(args);
//                    //systemCommands(tempargs,myct,bg);
//                }
                while(n<10){
                    fprintf(stderr,"%s ",currentNode->args[n]);
                    n++;
                }
                fprintf(stderr,"\n");

            }
            else if(args[1] != NULL && strlen(args[1]) > 1 ){//ikinci arguman olarak 1 chardan uzun bir sey girildiyse
                if( strstr(firstHist->args[0],args[1]) != NULL ){
                    //KOMUTU CALISTIR
                    while(firstHist->args[o][0]!='\0'){
                        tempargs[o]=firstHist->args[o];
                        o++;
                    }
                    o=0;
                    while(n<10){
                    fprintf(stderr,"%s ",firstHist->args[n]);
                    n++;
                    }
                    fprintf(stderr,"\n");
//                    if((strcmp(firstHist->args[0],"cd") == 0) || (strcmp(firstHist->args[0],"dir") == 0) || (strcmp(firstHist->args[0],"clr") == 0)
//                       || (strcmp(firstHist->args[0],"wait") == 0) || (strcmp(firstHist->args[0],"hist") == 0) || (strcmp(firstHist->args[0],"!") == 0) || (strcmp(firstHist->args[0],"exit") == 0)){
//                        //CALL BUILT IN COMMANDS
//                        //addToHist(args);
//                        //builtInCommands(tempargs);
//                        fprintf(stderr,"\n%s\n",firstHist->args[0]);
//                    }else if( (firstHist->args[0] != NULL) || (firstHist->args[1] != NULL && strcmp(firstHist->args[0],"|") != 0) ){
//                        //CALL SYSTEM COMMANDS
//                        //addToHist(args);
//                        //systemCommands(tempargs,myct,bg);
//
//                    }
                }
            }
        }

}//end of function

//CHECK BACKGROUND PROCESSES
int check_background(){
    char *command="pgrep -P";
    char ID[8];int id,status;
    sprintf(ID,"%d",parentID);
    char *seeAllChild = (char *) malloc( 1 + strlen(command)+ strlen(ID) );
    strcpy(seeAllChild, command);
    strcat(seeAllChild," ");
    strcat(seeAllChild, ID);

    FILE *output=popen(seeAllChild , "r");

    if(!output){
        //return -1;
        return -1;
    }

    char buffer[64];
    int i=0;
    char currentChildID[MAX_CHILDPROCESS][64];//MAX 256 PROCESS AND 64 CHARACTER
    while(fgets(buffer,sizeof(buffer),output)!= NULL){

    strcpy(currentChildID[i],buffer);
    //fprintf(stderr,"%s",currentChildID[i]);
    i++;
    }
    strcpy(lastp,currentChildID[i-1]);
    return i;

}


//SYSTEM COMMANDS IN
void systemCommands(char *args[],int ct,int background){

	char *checkPath;
	int i=0, j=0, childpid, resultCpath, destPath;
	struct pipeNode *command=NULL;

	while(i<ct){
		checkPath=args[i];
		if(checkPath[0] == '|'){
            pid_t saveParent;
            saveParent=fork();
            if(saveParent==0){
                changeNode(&command,args);
                free(command);
                kill(getpid(),SIGTERM);

            }
            else{
                wait(NULL);
                return;
            }
		}
		i++;
	}
	i=0;
    //EXECL CONTROL
	while(i < ct){

		checkPath = args[i];

		if(checkPath[0] == '/'){
			if(i != 0 && i != 1){
                destPath=i;
			}
			resultCpath = 1;
		}
		//CHECK PARAMETERS
		else if(checkPath[0] == '&'){
            args[i]=NULL;
		}
		else{
			resultCpath = 0;
		}
		i++;
	}

    signal(SIGCHLD,SIG_IGN);
	childpid = fork();

	i=0;


	//If child
	if(childpid == 0){
		if(resultCpath == 0){
			execvp(args[0], args);
			kill(getpid(),SIGTERM);
		}
		else if(resultCpath == 1){
			execl(args[0], args[1], args[2], args[3], args[4], args[5], args[destPath], (char *)NULL);
			kill(getpid(),SIGTERM);
		}
	}
    //If parent
	if(childpid != 0){
        parentID=getpid();

		if(background == 0){
            waitpid(childpid,NULL,0);
            return;
        }

		else{
            return;
        }
	}

}

void addToHist(char *savedArg[]){
    int i = 0,j=0;
    if(firstHist==NULL){ //hist e eklenecek olan ilk komutsa
        firstHist = (histNP) malloc(sizeof(struct histNode));
        for(i=0;i<10;i++){
            for(j=0;j<41;j++){
                firstHist->args[i][j]='\0';
            }
        }
        i=0;j=0;
        while (savedArg[i] != NULL){
            strcpy(firstHist->args[i], savedArg[i]);
            firstHist->number=1;
            i++;
        }
        firstHist->nextHistNode=NULL;
    }else { //hist e eklenmis komutlar var ise
        histNP newNode = (histNP) malloc(sizeof(struct histNode));
        for(i=0;i<10;i++){
            for(j=0;j<41;j++){
                newNode->args[i][j]='\0';
            }
        }
        i=0;j=0;
        while (savedArg[i] != NULL) {
            strcpy(newNode->args[i], savedArg[i]);
            newNode->number = firstHist->number+1;
            i++;
        }
        newNode->nextHistNode = firstHist;
        firstHist = newNode;
    }
}

void changeNode(struct pipeNode **head,char *args[]){
    int initial=0;
    struct pipeNode *temp=*head;
    int makezero=0;

    while(args[argIndex]!= NULL && args[argIndex-1]!=NULL){
        if(strcmp(args[argIndex],"|")!=0){

            struct pipeNode *newCommand=(struct pipeNode *) malloc(sizeof(struct pipeNode));
            //Make all elements 0 for a node's char *args!
            while(makezero<MAX_LINE){
            	memset(&newCommand->args[makezero], 0, sizeof(newCommand));
            	makezero++;
			}
            makezero=0;
            addCommandIntoNode(newCommand,args);
            newCommand->next=NULL;
            if(*head==NULL){
                *head=newCommand;
                temp=newCommand;
            }
            else{
                while(temp->next != NULL){
                    temp=temp->next;
                }
                temp->next=newCommand;
            }
            initial++;
        }
    }
    temp=*head;
    pipeCommands(temp);

//    TOO SEE THE CONTENT OF THE NODES
//    int y=0;
//    while(temp!=NULL){
//        while(temp->args[y]!=NULL){
//        	if(temp->args[y]!=NULL){
//        		printf("Node'un ici: %s\n ",temp->args[y]);
//			}
//            y++;
//        }
//        y=0;
//        temp=temp->next;
//    }
}

void addCommandIntoNode(struct pipeNode *newCommand,char *args[]){
    int i=argIndex;
    int j=0;
    //Add command and all of its parameters into newCommand
    while( args[i]!=NULL && (strcmp(args[i],"|")!=0) ){

        newCommand->args[j]=args[i];
        i++;
        j++;
    }
    argIndex=i+1;
    }

//Method to execute the pipe command
void pipeCommands(struct pipeNode *head){

    pid_t childpid;
    int fd[2*np];
    int i=0,j=0;
    struct pipeNode *command=head;
    //Set up all needed pipes
    for(i=0;i<np;i++)
        pipe(fd+(i*2));


    while(command){
        childpid=fork();
        //If it is child
        if(childpid==0){

            dup2(fd[j+1],1);
                //If this is not first command
            if(j!=0){
                dup2(fd[j-2],0);
            }
            //Close pipes
            for(i=0;i<2*np;i++){
                close(fd[i]);
            }
            //EXECUTE COMMANDS
            execvp(command->args[0],command->args);
        }

        command=command->next;
        j+=2;

    }
    //Parent closes pipes and waits for children
    for(i=0;i<(2*np);i++){
        close(fd[i]);
    }

    for(i=0;i<np+1;i++){
        wait(NULL);
        kill(getpid(),SIGTERM);
//        if(i==np-1){
//            wait(NULL);
//            return;
//        }
//        else{
//            wait(NULL);
//            kill(getpid(),SIGTERM);
//        }
    }

}


//MAIN FUNCTION
int main(void)
{
        char inputBuffer[MAX_LINE];
	    /*buffer to hold command entered */

	    int background;
	    /* equals 1 if a command is followed by '&' */

	    char *args[MAX_LINE/2 + 1];
	    /*command line arguments */

        //Launch shell for the first time condition
	    int n=0;
	    launch[0]="ls";
	    systemCommands(launch,1,0);
	    system("clear");


	while (1){
                        background = 0;
                        fprintf(stderr,"CSE333sh: ");

                        /*setup() calls exit() when Control-D is entered */
                        setup(inputBuffer, args, &background);
                        bg=background;
                        while(args[n]!=NULL){
                            if(strcmp(args[n],"|")==0){
                                np++;
                            }
                            n++;
                        }


                        if(  (strcmp(args[0],"cd") == 0) || (strcmp(args[0],"dir") == 0) || (strcmp(args[0],"clr") == 0)
                            || (strcmp(args[0],"wait") == 0) || (strcmp(args[0],"hist") == 0)
                            ||(strcmp(args[0],"!") == 0) || (strcmp(args[0],"exit") == 0)  ){
                            //CALL BUILT-IN COMMANDS
                            if((strcmp(args[0],"!") != 0)){
                                addToHist(args);
                            }
                            builtInCommands(args);
                        }
                        else if( (args[0] != NULL) || (args[1] != NULL && strcmp(args[1],"|") != 0) ){
                            //CALL SYSTEM COMMANDS
                            addToHist(args);
                            systemCommands(args,myct,background);
                        }
                        np=0;
/** the steps are:
(1) fork a child process using fork()
(2) the child process will invoke execvp() or execl()
(3) if background == 0, the parent will wait,
    otherwise it will invoke the setup() function again. */

	}
}
