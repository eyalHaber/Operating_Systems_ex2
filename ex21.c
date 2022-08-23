// Eyal Haber 203786298

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NEWFILE (O_WRONLY | O_CREAT | O_TRUNC )
#define MODE600 (S_IRUSR | S_IWUSR)
#define SIZE 10

// functions:
void fileCopy(char* inPath, char* outPath);
int is_identical(int fd_file1,int fd_file2);
int is_similar(int fd_file1_no_spaces,int fd_file2_no_spaces);

// main:
int main(int argc,char* argv[])
{
    int fd_file1;                   /* input file1 descriptor */
    int fd_file2;                   /* input file2 descriptor */
    int fd_file1_no_spaces;         /* input file1_copy descriptor */
    int fd_file2_no_spaces;         /* input file2_copy descriptor */
    char new_file1_name[] = "file1_no_spaces.txt";
    char new_file2_name[] = "file2_no_spaces.txt";

    fileCopy(argv[1], new_file1_name);
    fileCopy(argv[2], new_file2_name);

    fd_file1 = open(argv[1],O_RDONLY);
    if (fd_file1 < 0) /* means file open did not take place */
    {
        perror("after open");   /* text explaining why */
        exit(-1);
    }

    fd_file2 = open(argv[2],O_RDONLY);
    if (fd_file2 < 0) /* means file open did not take place */
    {
        perror("after open ");   /* text explaining why */
        exit(-1);
    }
    fd_file1_no_spaces = open(new_file1_name,O_RDONLY);
    if (fd_file1_no_spaces < 0) /* means file open did not take place */
    {
        perror("after open ");   /* text explaining why */
        exit(-1);
    }

    fd_file2_no_spaces = open(new_file2_name,O_RDONLY);
    if (fd_file2_no_spaces < 0) /* means file open did not take place */
    {
        perror("after open ");   /* text explaining why */
        exit(-1);
    }
    int identical = is_identical(fd_file1,fd_file2);
    //int similar = is_similar(fd_file1_no_spaces,fd_file2_no_spaces);
    int a = is_similar(fd_file1_no_spaces,fd_file2_no_spaces); // option 1
    int b = is_similar(fd_file2_no_spaces,fd_file1_no_spaces); // option 2
    int similar = (a<b) ? a : b; // if one them is 0 take him

    close(fd_file1);		/* free allocated structures */
    close(fd_file2);		/* free allocated structures */
    close(fd_file1_no_spaces);		/* free allocated structures */
    close(fd_file2_no_spaces);		/* free allocated structures */
    remove(new_file1_name);     /* remove new file1 */
    remove(new_file2_name);     /* remove new file2 */

    if (identical == 1){
        return 1;
    }
    else if (similar == 1){
        return 3;
    }
    else if (similar == 0){
        return 2;
    }
}

// functions:
void fileCopy(char* inPath, char* outPath)
{
    char buf[SIZE];
    int in,out,count;

    if((in = open(inPath,O_RDONLY)) == -1)
    {
        perror("Error in\n");
        exit(-1);
    }
    if((out = open(outPath,NEWFILE, MODE600)) == -1)
    {
        perror("Error in\n");
        exit(-1);
    }
    while((count = read(in, buf, sizeof(buf))) > 0)
    {
        int spaces_count = 0;
        int i;
        for ( i = 0; i <count ; ++i) {
            if (buf[i]== ' ' ||buf[i]== '\n' ){
                spaces_count++;
            }

        }
        int final_len = count - spaces_count;
        char final[final_len];
        int j = 0;
        for ( i = 0; i <count ; ++i) {
            if (buf[i]!= ' ' && buf[i]!= '\n'){
                final[j] = buf[i];
                j++;
            }
        }

        if(write(out, final, final_len) != final_len)
        {
            perror("Error in\n");
            exit(-1);
        }
    }
    if(count==-1)
    {
        perror("Error in\n");
        exit(-1);
    }
    close(in);
    close(out);
}

int is_identical(int fd_file1,int fd_file2)
{
    int char_file1;          /* how many chars were actually red */
    int char_file2;          /* how many chars were actually written */
    char buf1[SIZE]; /* input (output) buffer */
    char buf2[SIZE]; /* input (output) buffer */
    int identical = 1;

    do
    {
        char_file1 = read(fd_file1,buf1,SIZE);
        char_file2 = read(fd_file2,buf2,SIZE);

        char final1_temp[char_file1+1];
        int i;
        for ( i = 0; i <char_file1 ; ++i) {
            final1_temp[i] = buf1[i];
        }
        final1_temp[i] = '\0';

        char final2_temp[char_file2+1];
        for ( i = 0; i <char_file2 ; ++i) {
            final2_temp[i] = buf2[i];
        }
        final2_temp[i] = '\0';
        char* final1 = &final1_temp[0];
        char* final2 = &final2_temp[0];

        if (strcmp(final1,final2) != 0){ // if the string chunks are not identical!
            identical = 0; // these two files are surly not identical!
        }

    }while ( (char_file1 == SIZE) && (char_file2 == SIZE));
    return identical;
}

int is_similar(int fd_file1_no_spaces,int fd_file2_no_spaces)
{
    int char_file3;          /* how many chars were actually red */
    int char_file4;          /* how many chars were actually red */
    char buf3[SIZE]; /* input (output) buffer */
    char buf4[SIZE]; /* input (output) buffer */
    int similar = 1;  // they are similar until proven wrong
    lseek(fd_file1_no_spaces, 0, SEEK_SET);
    lseek(fd_file2_no_spaces, 0, SEEK_SET);

    do
    {
        char_file3 = read(fd_file1_no_spaces, buf3, SIZE);
        char_file4 = read(fd_file2_no_spaces, buf4, SIZE);

        char final1_temp[char_file3+1];
        int i;
        for ( i = 0; i <char_file3 ; ++i) {
            final1_temp[i] = buf3[i];
        }
        final1_temp[i] = '\0';
        char* final1 = &final1_temp[0];

        char final2_temp[char_file4+1];
        for ( i = 0; i <char_file4 ; ++i) {
            final2_temp[i] = buf4[i];
        }
        final2_temp[i] = '\0';
        char* final2 = &final2_temp[0];

        if (strcmp(final1,final2) != 0){
            for ( i = 0; i < char_file3; ++i) {

                if ((int) final1[i] == (int) final2[i] || (int) final1[i] == (int) final2[i]+32 || (int) final2[i] == (int) final1[i]+32){
                }
                else  similar = 0;
            }
        }
    }while ((char_file3 == SIZE) && (char_file4 == SIZE));

    return similar;
}


