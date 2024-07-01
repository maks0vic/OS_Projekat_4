#include <stdatomic.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>


#define MAX_WORD_LEN 64 //najveca dozvoljena duzina reci, uz \0
#define LETTERS 26 //broj slova u abecedi i broj dece u trie

extern void scanner_init(); //poziva se jednom na pocetku rada sistema
extern void *scanner_work(void *_args); //funkcija scanner niti

typedef struct search_result //rezultat pretrage
{
	int result_count; //duzina niza
	char **words; //niz stringova, svaki string duzine MAX_WORD_LEN
} search_result;

extern void trie_init(); //poziva se jednom na pocetku rada sistema
extern void trie_add_word(char *word); //operacija za dodavanje reci
extern search_result *trie_get_words(char *prefix); //operacija za pretragu
extern void trie_free_result(search_result *result); //rezultat se dinamicki alocira pri pretrazi, tako da treba da postoji funkcija za oslobadjanje tog rezultata

typedef struct trie_node //cvor unutar trie strukture
{
	char c; //slovo ovog cvora
	int term; //flag za kraj reci
	int subwords; //broj reci u podstablu, ne racunajuci sebe
	struct trie_node *parent; //pokazivac ka roditelju
	struct trie_node *children[LETTERS]; //deca
} trie_node;

typedef struct scanned_file //datoteka koju je scanner vec skenirao
{
	char file_name[256]; //naziv datoteke
	time_t mod_time; //vreme poslednje modifikacije datoteke
} scanned_file;


volatile int work = 1;

void* scanner_work(void *_args) {
    // printf("Thread start\n");
    char *dirPath = (char*)_args;

    scanned_file files[50] = {};
    int fileNumber = 0;
    // seconds = time(NULL);

    DIR *directory;
    FILE *file;
    struct dirent *entry;
    char word[100]; 

    // printf("dirPath:|%s|", dirPath);

    struct stat file_stat;

    while(work)
    {    
        directory = opendir(dirPath);

        if (directory == NULL) {
            printf("Unable to open directory");
            return 0;
        }
        // printf("Prosao dir");
        while ((entry = readdir(directory)) != NULL) {
            // Print the name of each file in the directory
            // printf("filename:%s len=%lu\n", entry->d_name, strlen(entry->d_name));
            
            if (!strcmp (entry->d_name, "."))
                continue;
            if (!strcmp (entry->d_name, ".."))    
                continue;

            char* fullPath = strdup(strcat(strcat(strdup(dirPath), "/"), entry->d_name));

            // int load = 1;
            // for (int i=0; i<fileNumber; i++) {
            //     if(strcmp(files[i].file_name, fullPath) == 0) {
            //         // printf("Poklapanje %s\n", fullPath);
            //         if (stat(fullPath, &file_stat) == 0) {
            //             // Get the last modification time
            //             time_t last_modified_time = file_stat.st_mtime;
            //             // printf("%lu %lu\n", files[i].mod_time, last_modified_time);
            //             if(files[i].mod_time > last_modified_time) {
            //                 load = 0;
            //             }
            //         }
            //     }
            // }

            // if (!load) continue;

            printf("Opening:%s\n",fullPath);
            file = fopen(fullPath, "r");
            
            if (file == NULL) {
                printf("Unable to open the file.\n");
                return 0;
            }

            scanned_file f = {};
            f.mod_time = time(NULL);
            strcpy(f.file_name, fullPath);
            

            for (int i=0; i<fileNumber; i++) {
                if(strcmp(files[i].file_name, f.file_name) == 0) {
                    // printf("Poklapanje %s\n", fullPath);
                    if (stat(f.file_name, &file_stat) == 0) {
                        // Get the last modification time
                        time_t last_modified_time = file_stat.st_mtime;
                        // printf("%lu %lu\n", files[i].mod_time, last_modified_time);
                        files[fileNumber] = f;
                        fileNumber++;
                    }
                }
            }
            
            
            printf("scanned file%d:%s %lu\n",fileNumber, f.file_name, f.mod_time;
            

            while (fscanf(file, "%s", word) == 1) {
                printf("%s\n", word);
            }
            fclose(file);
            
            
        }
        closedir(directory);        
        // printf("Close dir\n");
        // printf("going to sleep\n");
        sleep(5);
        // printf("tik\n");s
    }
    printf("Thread finish\n");
    return 0;
}



int main(int argc, char *argv[])
{
    int threadCount = 0;
    int NMAX = 10;
    char path[50];

    pthread_t scanThread[NMAX];

    char input[50];
    while (1) 
    {
        
        // printf("While start\n");
        // printf("Enter a command:");
        fgets(input, 50, stdin);
        input[strcspn(input, "\n")] = 0;


        // char* split = strtok (input," ");

        // printf("Command =|%s|\n", input);
        if (strcmp(input, "dir") == 0) {
            pthread_create(&scanThread[threadCount], 
            NULL, scanner_work, &input);
            threadCount++;
        }

        if (strcmp(input, "_stop_") == 0) 
        {
            work = 0;
            printf("Stopping everything!\n");
            printf("Thread Count = %d\n", threadCount);
            for (unsigned int i = 0; i < threadCount; i++) {
                printf("Shutting down thread: %d\n", i);
                pthread_join(scanThread[i], NULL);
            }
            return 0;
        }

        char *ptr = strtok(input, " ");

        if (strcmp(ptr, "_add_") == 0) 
        {
            
            ptr = strtok(NULL, " ");
            char add[50] = ""; 
            strcpy(add, ptr);
            // printf("|%s|\n", add);


            pthread_create(&scanThread[threadCount], 
            NULL, scanner_work, &add);
            threadCount++;
        }

        
        // printf("While end\n");
    }
    printf("Unexpected end\n");
	return 0;
}