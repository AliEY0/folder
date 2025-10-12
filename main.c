#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <stdbool.h>
#include <sys/wait.h>



struct termios original;
char **read_folder(int *files){
    DIR *folder;
    struct dirent *entry;
    char **out = NULL;
    *files = 0;
    folder = opendir(".");
    if(folder == NULL)
    {
        perror("Unable to read directory");
        return NULL;
    }

    while( (entry=readdir(folder)) )
    {
        out = realloc(out, sizeof(char*) * (*files + 1));
        out[*files]= strdup(entry->d_name);
        *files += 1;
        //printf("File %3d: %s\n", *files,entry->d_name);
    }

    closedir(folder);
    return out;
}

void open_vim(char *path){
    pid_t pid = fork(); 
    if(pid == 0) {
        char *args[] = {"/usr/bin/vi", path, NULL};
        execv("/usr/bin/vi", args);
        perror("execv failed"); 
        _exit(1);
    } else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}

void print_folder(char **folder, int index,int lengte, bool enter){
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
    for(int i = 0; i < lengte; i++){
        if(i == index){
            printf("> ");
        }else{
            printf("  ");
        }
        printf("%s\n",folder[i]);
    }
    if(enter){
        printf("Filename: %s\n",folder[index]);
        open_vim(folder[index]);
    }
}


void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original);
    atexit(disable_raw_mode);

    struct termios raw = original;
    raw.c_lflag &= ~(ECHO | ICANON);  
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void get_input(){
    char **folder;
    int lengte = 0;
    folder = read_folder(&lengte);

    int index = 0;
    print_folder(folder, index, lengte,false);
    enable_raw_mode();
    char a;
    while (read(0, &a, 1) != 'q') {
        bool enter = false;
        if(a == 'd'){
            index = (index + 1) % lengte;
        }else if(a == 'f'){
            index -= 1;
            if(index == -1) index = lengte - 1;
        }
        else if(a == '\n'){
            enter = true;
        }

        print_folder(folder, index, lengte,enter);   
    }
    free(folder);
    
}



int main(){
    get_input();
    disable_raw_mode();
    return 0;
}











