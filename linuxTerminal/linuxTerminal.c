#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define SIZE 100
int howManyParam(char * theInput){
    theInput[strlen(theInput)-1] = '\0';
        //count how many word the user put
    int i=0;
    int countParam = 0;
    for(;theInput[i]!='\0';i++){
        if(theInput[i]==' ' || theInput[i]=='|' || theInput[i]=='>'|| theInput[i]=='<' || theInput[i]=='2'){
            countParam++;
            if(theInput[i]=='>' && theInput[i+1]=='>'){
                i++;
            }
            else if(theInput[i]=='2' && theInput[i+1]=='>'){
                i++;
            }
        }
        while(theInput[i]==' ' || theInput[i]=='|' || theInput[i]=='>' || theInput[i]=='<' || (theInput[i]=='2' && theInput[i+1]=='>')){
            i++;
        }
    }
    countParam++;
    return countParam;

}


int main (int argc, const char* argv[]){
    int debug = 0;
    int numOfCommand = 0;
    int numOfPipe = 0;
    int numOfInto = 0;
    int sumCharInAllCommand = 0;
    char buf[SIZE];
    struct passwd * pw;
    pw = getpwuid(getuid());
    
    while(1){
        printf("%s@%s>",pw->pw_name,getcwd(buf,SIZE));
        char theInput[510]; 
        fgets(theInput,510,stdin);
        numOfCommand++;
        sumCharInAllCommand+=strlen(theInput);
        sumCharInAllCommand-=1;//remove the \n
        
        if(!debug){
            int file = open(argv[1],O_WRONLY | O_APPEND | O_CREAT , 0777);
            write(file,theInput, strlen(theInput));
            close(file);
        }
        
        int doExit = strcmp(theInput, "done\n");
        if(doExit == 0){
            break;
        }
        /**************
         * first checks
         *************/
        int doCD = strcmp(theInput, "cd\n");//in case this is an cd
        if(doCD == 0){
            printf("Command not supported (Yet)\n");
        }

        int isEmpty = strcmp(theInput, "\n");//in case this is only an \n
        if(isEmpty == 0){
            continue;
        }

        int isAspace = strcmp(theInput, " \n");// in case this is only an space
        if(isAspace == 0){
            continue;
        }


        
        

        int countParam = howManyParam(theInput);

        if(debug){
            printf("count param = %d\n",countParam);
        }
        char** arg = malloc(sizeof(char*)*(countParam));
        if(arg==NULL){
            perror("malloce fail");
            exit(1);
        }
        //printf("malloc:%d\n",arg);
        int j = 0;
        int indexInInput = 0;
        while(theInput[indexInInput]==' '){//in case the user space at the begining
            indexInInput++;
        }
        int flag_have_pipe = 0;// in case we detect '|'
        int index_pipe = -1; // save here the index of the word came befor the |
        int flag_have_one_into = 0;// in case we detect '>'
        int flag_have_two_into = 0;// in case we detect '>>'
        int index_into = -1;// save the index of the word came before the > or the >>
        int flag_have_from = 0;//in case we have <
        int index_from =-1;// save the index of the word came before the <
        int flag_have_error_2 = 0; //in case we have 2>
        int index_error_2 = -1;// save the index of the word came before the 2>

        for(;j<countParam;j++){
            int countCharInWord = 0;
            int tempIndex = indexInInput;
            int flag_this_is_a_operator = 0; //in case we found '|' or '>' or '>>' or '2> or '<'
            while(theInput[tempIndex]!=' ' && theInput[tempIndex]!='\n' && theInput[tempIndex]!='\0'){
                //printf("%d -> %c\n",tempIndex,theInput[tempIndex]);
                //printf("tempIndex = %d, char = %c, j=%d \n",tempIndex,theInput[tempIndex],j);
                if(theInput[tempIndex]=='|'){
                    flag_have_pipe = 1;
                    if(theInput[tempIndex-1]==' '){
                        j--;
                        flag_this_is_a_operator = 1;
                        tempIndex++;
                    }
                    index_pipe = j;
                    numOfPipe++;
                    break;
                }
                else if(theInput[tempIndex]=='>'){
                    if(theInput[tempIndex+1] == '>'){
                        flag_have_two_into = 1;
                        if(theInput[tempIndex-1]==' '){
                            j--;
                            flag_this_is_a_operator = 1;
                        }
                        tempIndex++;
                        tempIndex++;
                    }
                    else{
                        flag_have_one_into = 1;
                        if(theInput[tempIndex-1]==' '){
                            j--;
                            flag_this_is_a_operator = 1;
                        }
                    }
                    
                    index_into = j;
                    tempIndex++;
                    numOfInto++;
                    break;
                }
                else if(theInput[tempIndex]=='<'){
                    flag_have_from = 1;
                    if(theInput[tempIndex-1]==' '/* || theInput[tempIndex+1]==' '*/){
                        j--;
                        flag_this_is_a_operator = 1;
                        tempIndex++;
                    }
                    index_from = j;
                    tempIndex++;
                    numOfInto++;
                    break;
                }
                else if(theInput[tempIndex]=='2' && theInput[tempIndex+1]=='>'){
                    flag_have_error_2 = 1;
                    tempIndex++;
                    tempIndex++;
                    if(theInput[tempIndex-3]==' '/* || theInput[tempIndex+1]==' '*/){
                        j--;
                        flag_this_is_a_operator = 1;
                        tempIndex++;
                    }
                    index_error_2 = j;
                    numOfInto++;
                    break;
                }
                tempIndex++;
                countCharInWord++;
            }
            if(!flag_this_is_a_operator){
                arg[j] = (char*)(malloc(sizeof(char)*(countCharInWord+1)));
                if(arg[j]==NULL){
                    perror("malloc fail");
                    exit(1);
                }
                int h=0;
                for(;h<countCharInWord;h++){
                    arg[j][h] = theInput[h+indexInInput];
                }
                arg[j][h] = '\0';
            }
            //printf("arg[%d]=%s\n",j,arg[j]);
            indexInInput =indexInInput+countCharInWord+1;
            if(flag_have_two_into|| flag_have_error_2){
                indexInInput++;
            }
            while(theInput[indexInInput]==' '){//in case the user put more then 1 space
                indexInInput++;
            }
        }
        if(debug){
            for(int i=0;i<countParam;i++){
                printf("arg[%d] = %s\n",i,arg[i]);
            }
            if(flag_have_pipe){
                printf("index | =%d\n",index_pipe);
            }
            if(flag_have_one_into){
                printf("index > = %d\n",index_into);
            }
            if(flag_have_two_into){
                printf("index >> = %d\n",index_into);
            }
            if(flag_have_from){
                printf("index < = %d\n",index_from);
            }
            if(flag_have_error_2){
                printf("index 2> = %d\n",index_error_2);
            }
        }

        if(flag_have_pipe){// in case we have |
            //printf("in 2\n");
            int p[2];
            if((pipe(p))==-1){
                perror("pipe failde");
                exit(EXIT_FAILURE);
            }
            
            char** first = (char *)malloc(sizeof(char*)*(index_pipe+2));
            if(!first){printf("malloc error"); exit(1);}
            //if(debug){printf("size malloc 1: %d\n",index_pipe+2);}
            char** last;
            if(!flag_have_one_into && !flag_have_two_into){
                last = (char*)malloc(sizeof(char*)*(countParam-index_pipe));
            }
            else{
                last = (char*)malloc(sizeof(char*)*(index_into-(index_pipe)+1));
            }
            if(!last){printf("malloc error"); exit(1);}
            
            
            for(int i=0; i<(index_pipe+1);i++){
                first[i] = arg[i];
                //printf("first[%d] = %s\n",first[i]);
            }
            first[index_pipe+1] = NULL;
            //printf("first[%d] = %s\n",index_pipe+1,first[index_pipe+1]);
            
            if(!flag_have_one_into && !flag_have_two_into){
                for(int i=(index_pipe+1),j=0; i<(countParam);i++,j++){
                    last[j] = arg[i];
                    //printf("last[%d] = %s\n",j,last[j]);
                }
                last[countParam] = NULL;
                //printf("last[%d] = %s\n",countParam-index_pipe+1,last[countParam-index_pipe+1]);
            }
            else{
                for(int i=(index_pipe+1),j=0; i<(index_into+1);i++,j++){
                    last[j] = arg[i];
                    //printf("last[%d] = %s\n",j,last[j]);
                }
                last[index_into-index_pipe] = NULL;

                //printf("last[%d] = %s\n",countParam-index_pipe+1,last[countParam-index_pipe+1]);
            }
            
            // for(int i=0;i<countParam-(index_pipe+1);i++){
            //         printf("first[%d]=%s\n",i,first[i]);
            // }
            // for(int i=0;i<3;i++){
            //         printf("last[%d]=%s\n",i,last[i]);
            // }
            
            int status;
            pid_t fork_first, fork_last;
            fork_first = fork();
            if(fork_first==0){
                close(STDOUT_FILENO);  //closing stdout
                dup2(p[1],STDOUT_FILENO);
                close(p[0]);//close pipe
                close(p[1]);
                execvp(first[0],first);
            }
            fork_last = fork();
            if(fork_last ==0){
                //printf("in fork_last \n");
                if(flag_have_one_into || flag_have_two_into){
                    int file;
                    if(flag_have_one_into){//do the '>' =  write over, and don't creat new file if not exist
                        file = open(arg[index_into+1],O_WRONLY | O_CREAT, 0777);
                    }
                    else{// do the '>>' write ant the end of the file, if the file don't exist, creat new one
                        file = open(arg[index_into+1],O_WRONLY | O_APPEND | O_CREAT , 0777);
                    }
                    close(STDOUT_FILENO);
                    dup2(file,STDOUT_FILENO);
                    close(file);
                    //printf("in the if\n");
                }
                dup2(p[0],STDIN_FILENO);
                close(p[1]);//close pipe
                close(p[0]);
                execvp(last[0],last);
            }
            close(p[1]);//close pipe
            close(p[0]);
            wait(NULL);
            wait(NULL);
            free(last);
            free(first);
            
        }
        else if(flag_have_one_into || flag_have_two_into){// in case we have > or >> (and no |)
            
            char ** left = (char ** )malloc(sizeof(char*)*(index_into+2));
            //printf("size malloc = %d",index_into+2);
            if(!left){printf("malloce error");exit(1);}
            for(int i=0; i<(index_into+1);i++){
                left[i] = arg[i];
                //printf("left[%d]=%s\n",i,left[i]);
            }
            left[index_into+1] = NULL;
            if(fork()==0){
                int file;
                if(flag_have_one_into){//do the '>' =  write over, and don't creat new file if not exist
                    file = open(arg[index_into+1],O_WRONLY | O_CREAT, 0777);
                }
                else{// do the '>>' write ant the end of the file, if the file don't exist, creat new one
                    file = open(arg[index_into+1],O_WRONLY | O_APPEND | O_CREAT , 0777);
                }
                close(STDOUT_FILENO);
                dup2(file,STDOUT_FILENO);
                close(file);
                execvp(left[0],left);
            }
            else{
                wait(NULL);
                free(left);
            }
        }
        else if(flag_have_from){//in case we have <
            char ** left = (char ** )malloc(sizeof(char*)*(index_from+2));
            //printf("size malloc = %d",index_from+2);
            if(!left){printf("malloce error");exit(1);}
            for(int i=0; i<(index_from+1);i++){
                left[i] = arg[i];
                //printf("left[%d]=%s\n",i,left[i]);
            }
            left[index_from+1] = NULL;
            if(fork()==0){
                int file;
                file = open(arg[index_from+1],O_RDONLY);
                close(STDIN_FILENO);
                dup2(file,STDIN_FILENO);
                close(file);
                execvp(left[0],left);
            }
            else{
                wait(NULL);
                free(left);
            }
        }
        else if(flag_have_error_2){//in case we have 2>
            char ** left = (char ** )malloc(sizeof(char*)*(index_error_2+2));
            //printf("size malloc = %d",index_error_2+2);
            if(!left){printf("malloce error");exit(1);}
            for(int i=0; i<(index_error_2+1);i++){
                left[i] = arg[i];
                //printf("left[%d]=%s\n",i,left[i]);
            }
            left[index_error_2+1] = NULL;
            // for(int i=0; i<(index_error_2+2);i++){
            //     printf("left[%d]=%s\n",i,left[i]);
            // }
            if(fork()==0){
                int file;
                file = open(arg[index_error_2+1],O_WRONLY | O_APPEND | O_CREAT , 0777);
                //printf("left[%d]=%s\n",index_error_2+1,arg[index_error_2+1]);
                close(STDERR_FILENO);
                dup2(file,STDERR_FILENO);
                close(file);
                execvp(left[0],left);
            }
            else{
                wait(NULL);
                free(left);
            }
        }
        else{
            char ** toExec = (char ** )malloc(sizeof(char*)*(countParam+1));
            for(int i = 0; i<countParam+1;i++){
                toExec[i] = arg[i];
                //printf("toExec[%d] = %s",i,toExec[i]);
            }
            toExec[countParam] = NULL;
            int x = fork();
            if(x ==0){
                execvp(toExec[0],toExec);
                exit(0);
            }
            wait(NULL);
            free(toExec);
        }


        for(int i = 0; i<countParam;i++){
            free(arg[i]);
        }
        free(arg);
    }
    printf("Num of commands: %d\n",numOfCommand);
    printf("Total length of all commands: %d\n",sumCharInAllCommand);
    float average = sumCharInAllCommand;
    average = average/numOfCommand;
    printf("Average length of all commands: %f\n",average);
    printf("Num of command that include pipe: %d\n",numOfPipe);
    printf("Num of command that include rediraction: %d\n",numOfInto);
    printf("See you Next time !\n");
}
