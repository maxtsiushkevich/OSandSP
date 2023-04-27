#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

int compare_strings(const void* a, const void* b) // function for qsort
{
    return strcmp(*(const char**)a, *(const char**)b);
}

void print_environment() // print sorted environment
{
    int n = 0;
    while(environ[n] != NULL)
        n++;
    qsort(environ, n, sizeof(char*), compare_strings);
    printf("Environment variables:\n");
    for (int i = 0; i < n; i++) {
        printf("\t%s\n", environ[i]);
    }
    printf("\n");
}

void print_environment_from_file(char *filename) // print minimum environment from file(argv[1])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("File open error");
        exit(1);
    }
    char *env = malloc(sizeof(char) * 255);
    while (fgets(env, 255, file) != NULL)
    {
        env[strcspn(env, "\n")] = '\0';
        printf("\t%s = %s\n", env, getenv(env));
    }
    free(env);
    fclose(file);
}

char* get_child_name(int *counter) // generate child_XX
{
    char *name = malloc(sizeof(char) * 9);
    strcpy(name, "child_");

    if(*counter >= 0 && *counter < 10)
    {
        name[6] = '0';
        name[7] = '0' + (*counter);
        name[8] = '\0';
    }
    if(*counter >= 10 && *counter < 100)
    {
        name[6] = '0' + (*counter/10);
        name[7] = '0' + (*counter%10);
        name[8] = '\0';
    }
    (*counter)++;
    if (*counter == 100)
        *counter = 0;
    return name;
}

void get_minimum_environment(char *min_env[], char *filename) // return massive with variables from file(argv[1])
{
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("File open error");
        exit(1);
    }

    int i = 0;
    char *var = malloc(sizeof(char) * 255);
    while (fgets(var, 255, file) != NULL)
    {
        var[strcspn(var, "\n")] = '\0';
        sprintf(min_env[i], "%s=%s", var, getenv(var));
        i++;
    }
    fclose(file);
    free(var);
}

void create_process(char *child_path, char *arguments[], char *min_env[]) // create new process and run child program
{
    pid_t proc = fork();

    if (proc == -1)
        printf("Fork error");

    else if (proc > 0)
    {
        printf("Parent %d, parent of %d\n", getpid(), proc);
        wait(NULL);
    }
    else
    {
        printf("Child process %d\n", getpid());
        print_environment_from_file(arguments[1]);
        printf("\nRun child program:\n");
        execve(child_path, arguments, min_env);
        wait(NULL);
    }
}

int main(int argc, char *argv[], char *env[])
{
    setlocale(LC_COLLATE, "C");
    int counter = 0;
    char *child_path = NULL; // path to child program

    char *file_name = argv[1]; // text file with env vars

    print_environment();

    char *arguments[4] = {NULL, NULL, NULL, NULL };

    char **min_env = malloc(sizeof(char*) * 10);
    for (int i = 0; i < 9; i++)
        min_env[i] = malloc(sizeof(char) * 500);
    min_env[9] = NULL;

    get_minimum_environment(min_env, file_name); // get vars for new process

    printf("To create process press '+', '*' or '&'. For close press 'q'\n");
    while (1)
    {
        char c;
        rewind(stdin);
        scanf("%c", &c);
        rewind(stdin);

        switch(c)
        {
            case '+':
            {
                child_path = getenv("CHILD_PATH");
                arguments[0] = get_child_name(&counter);
                arguments[1] = file_name;
                arguments[2] = "+";
                create_process(child_path, arguments, min_env);
                break;
            }
            case '*':
            {
                for(int i = 0; env[i] != NULL; i++)
                {
                    char *str = strstr(env[i], "CHILD_PATH");
                    if (str != NULL)
                    {
                        child_path = str + 11;
                        break;
                    }
                }
                arguments[0] = get_child_name(&counter);
                arguments[1] = file_name;
                arguments[2] = "*";
                create_process(child_path, arguments, min_env);
                break;
            }
            case '&':
            {
                for (int i = 0; environ[i] != NULL; i++)
                {
                    char *str = strstr(environ[i], "CHILD_PATH");
                    if (str != NULL)
                    {
                        child_path = str + 11;
                        break;
                    }
                }
                arguments[0] = get_child_name(&counter);
                arguments[1] = file_name;
                arguments[2] = "&";
                create_process(child_path, arguments, min_env);
                break;
            }
            case 'q':
            {
                free(arguments[0]);
                for (int i = 0; i < 9; i++)
                    free(min_env[i]);
                free(min_env);
                exit(0);
            }
            default:
                break;
        }
    }
}