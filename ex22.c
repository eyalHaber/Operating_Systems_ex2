// Eyal Haber 203786298

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include<sys/wait.h>
#include <time.h>

#define SIZE 1
#define size_t 50

// functions:
int execute_vp_io_redirect(char** command, int fd_input, char* output_redirect_path);
int execute_vp(char **command);
int dir_open(char* dir_name, char* dir_path, char* input_path, char* correct_output_path, int fd_csv);

// main:
int main(int argc, char* argv[]) {
    // open conf file:
    int fd_conf;       /* input file descriptor */
    fd_conf = open(argv[1], O_RDONLY);
    if (fd_conf < 0)   /* means file open did not take place */
    {
        perror("after open");   /* text explaining why */
        exit(-1);
    }

    // read dir path from conf file (line 1):
    int char_file1;          /* how many chars were actually red */
    char buf[SIZE];         /* input (output) buffer */

    char dir_path_temp[150];
    int i;
    i = 0;
    do
    {
        char_file1 = read(fd_conf, buf, SIZE);
        if (buf[0] != '\n') { // read until end of the line
            dir_path_temp[i] = buf[0];
            i++;
        }
    } while (buf[0] != '\n');//read until end of the line
    dir_path_temp[i] = '\0';
    char* dir_path = &dir_path_temp[0]; //final dir path

    //read input file path from conf (line 2):
    i = 0;
    char input_path_temp[150];
    do
    {
        char_file1 = read(fd_conf, buf, SIZE);
        if (buf[0] != '\n'){
            input_path_temp[i] = buf[0];
            i++;
        }

    } while (buf[0] != '\n');
    input_path_temp[i] = '\0';
    char* input_path = &input_path_temp[0]; //final input path

    //read correct output file from conf (line 3):
    i = 0;
    char output_path_temp[150];
    do
    {
        char_file1 = read(fd_conf,buf,SIZE);
        if (buf[0] != '\n')
        {
            output_path_temp[i] = buf[0];
            i++;
        }

    } while (buf[0] != '\n');
    output_path_temp[i] = '\0';
    char* correct_output_path = &output_path_temp[0]; //final output_path

    int fd_output;
    fd_output = open(correct_output_path, O_RDONLY);

    if (fd_output < 0) /* meaning file open did not take place */
    {
        write(1,"Error in: Open\n", size_t);   /* text explaining why */
        exit(-1);
    }

    int fd_csv; //open csv file
    if ((fd_csv = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
    {
        perror("Error in: "); /* open failed */
        exit(-1);
    }

    //open dir from conf
    DIR* dip;
    struct dirent* dit;

    if(argc < 2)
    {
        perror("Error in: ");
        return 0;
    }

    if((dip = opendir(dir_path)) == NULL)
    {
        write(1,"Not a valid directory\n", size_t);
        exit(-1);
    }

    while((dit = readdir(dip)) != NULL) //for every file
    {
        char* name = &dit->d_name[0]; //get file name

        //get file path
        char inner_dir_path_temp [150];
        strcpy(inner_dir_path_temp, dir_path);
        strcat(inner_dir_path_temp, "/");
        strcat(inner_dir_path_temp, name);
        char* inner_dir_path = &inner_dir_path_temp[0];

        if (dit->d_type == 4 && strcmp(dit->d_name, ".")&& strcmp(dit->d_name, ".."))
        {   //if the file is a dir
            dir_open(name,inner_dir_path, input_path,correct_output_path,fd_csv); //open dir
        }
    }

    if(close(fd_csv) == -1)
    {
        write(2, "Error in: \n", SIZE);
    }
    if(closedir(dip) == -1)
    {
        perror("Error in: ");
    }
    if(close(fd_conf) == -1)
    {
        perror("Error in: ");
    }
    if(close(fd_output) == -1)
    {
        perror("Error in: ");
    }
    return 0;
}

int execute_vp_io_redirect(char **command, int fd_input, char* output_redirect_path)
{ // execute_vp_io_redirect in foreground
    int stat, waited, ret_code;
    pid_t pid;
    pid = fork();

    if (pid == 0) {  /* Child */
        int fd_out; /* new file descriptor */
        int fd_error; /* new file descriptor */

        if ((fd_error = open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
        {
            perror("Error in: \n"); /* open failed */
            exit(1);
        }

        dup2(fd_error,2);

        if ((fd_out = open(output_redirect_path, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
        {
            write(2,"Error in: \n", SIZE);
            perror("Error in: \n"); /* open failed */
            exit(-1);
        }

        dup2(fd_out, 1);
        dup2(fd_input,0);

        ret_code = execvp(command[0],command);
        if (ret_code == -1)
        {
            write(fd_error, "exec failed redirect \n", SIZE);
            exit(-1);
        }

        if(close(fd_out) == -1)
        {
            perror("Error in: \n");
            return 0;
        }

    }
    if (pid > 0) {
        {  /* Parent */
            wait(&stat);   /* stat can tell what happened */
            int result = stat/256;
            return result;
        }
    }
    if (pid == -1) {
        perror("Error in: \n");  // fork() failed
    }
    return -1;
}

int execute_vp(char **command) { // execute_vp_io_redirect in foreground
    int stat, waited, ret_code;
    pid_t pid;
    pid = fork();

    if (pid == 0) {  /* Child */
        int fd_error; /* new file descriptor */

        if ((fd_error = open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
        {
            perror("open failed"); /* open failed */
            exit(1);
        }

        dup2(fd_error,2);
        ret_code = execve(command[0],command,NULL);
        if (ret_code == -1)
        {
            write(2, "Error in: fork \n", size_t);
            exit(1); // unsuccessful termination
        }
    }
    if (pid > 0) {
        {  /* Parent */
            wait(&stat);   /* stat can tell what happened */
            int result = stat/256;
            return result;
        }
    }
    if (pid == -1) {
        write(2, "fork failed\n", SIZE);
    }
    return -1;
}

int dir_open(char* dir_name,char* dir_path,char* input_path,char* correct_output_path, int fd_csv )
{ //executes, runs and compares all c files in dir

    DIR* dip;
    struct dirent* dit;
    if((dip = opendir(dir_path)) == NULL)
    {
        perror("opendir");
        return 0;
    }

    int has_file_flag = 0;

    while((dit = readdir(dip)) != NULL)
    {
        int fd_input;
        fd_input = open(input_path,O_RDONLY);

        if (fd_input < 0) /* means file open did not take place */
        {
            write(2,"Error in: \n", SIZE);
            perror("Error in: \n");   /* text explaining why */
            exit(-1);
        }
        //for every file in dir, search for .c files
        char* name = &dit->d_name[0];
        int len = strlen(name);
        if (name[len-1] == 'c' && name[len-2] == '.' ){ //if file ends with .c
            has_file_flag = 1;
            // set full file path (path/name.txt)
            char c2[150];
            strcpy(c2, dir_path);
            strcat(c2,"/");
            strcat(c2,name);
            char* file_path = &c2[0];
            //add .txt
            char temp[150];
            strcpy(temp, dir_path);
            strcat(temp,".txt");
            char* file_path_with_txt = &temp[0];
            //run file
            char* args[] = {"gcc",file_path, NULL }; //compile file
            int result;
            result = execute_vp_io_redirect(args, fd_input, file_path_with_txt);
            if (result == 0)
            { //if compilation succeed
                //measure time
                time_t start_t, end_t;
                double diff_t;
                time(&start_t);

                char* run[] = {"./a.out", "a.out", NULL }; //run file with provided input
                execute_vp_io_redirect(run, fd_input, file_path_with_txt);

                time(&end_t);
                diff_t = difftime(end_t, start_t);

                if (diff_t > 5)
                { //if time is above 5 seconds, write in csv
                    char line_temp[150];
                    strcpy(line_temp,dir_name);
                    strcat(line_temp,",20,TIMEOUT\n");
                    char* line = &line_temp[0];
                    if(write(fd_csv, line,strlen(line)) != strlen(line))
                    {
                        write(2, "Error in: csv\n", SIZE);
                        exit(0); // successful program termination
                    }
                    break;
                }

                //compare with ex21
                char* run2[] = {"./comp.out",file_path_with_txt, correct_output_path,NULL}; //run ex21 to compare
                result = execute_vp(run2); //get result from output and correct output

                //write result to csv
                char line_temp[150];
                strcpy(line_temp,dir_name);

                if (result == 1){ // identical files
                    strcat(line_temp,",100,EXCELLENT\n");
                }
                if (result == 2){ // different files
                    strcat(line_temp,",50,WRONG\n");
                }
                if (result == 3){ // similar files
                    strcat(line_temp,",75,SIMILAR\n");
                }

                char* line = &line_temp[0];
                if(write(fd_csv,line,strlen(line)) != strlen(line))
                {
                    write(2, "Error in: csv\n", SIZE);
                    exit(0); // successful program termination
                }
            }
            else
            { //if compilation failed
                char line_temp[150];
                strcpy(line_temp,dir_name);
                strcat(line_temp,",10,COMPILATION_ERROR\n");
                char* line = &line_temp[0];
                if(write(fd_csv,line,strlen(line)) != strlen(line))
                {
                    write(2, "Error in: csv\n", SIZE);
                    exit(0); // successful program termination
                }
            }
        }
    }
    if (has_file_flag == 0)
    { //if there is no c file in directory
        char line_temp[150];
        strcpy(line_temp,dir_name);
        strcat(line_temp,",0,NO_C_FILE\n");
        char* line = &line_temp[0];
        if(write(fd_csv,line,strlen(line)) != strlen(line))
        {
            write(2, "Error in: csv\n", SIZE);
            exit(0); // successful program termination
        }
    }

    if(closedir(dip) == -1)
    {
        perror("Error in: \n");
        return 0; // successful program termination
    }
    return 0;
}