#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>

extern char **environ;

int main(int argc, char *argv[], char *env[])
{
    printf("Child program %s: pid = %d, ppid = %d\n", argv[0], getpid(), getppid());

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("File open error");
        exit(1);
    }

    char *tmp = malloc(sizeof(char) * 255);

    switch (argv[2][0])
    {
        case '+':
        {
            printf("Child environment:\n");
            while (fgets(tmp, 255, file) != NULL)
            {
                tmp[strcspn(tmp, "\n")] = '\0';
                printf("\t%s = %s\n", tmp, getenv(tmp));
            }
            break;
        }
        case '*':
        {
            //считываем из файла название и ищем значение в env
            while (fgets(tmp, 255, file) != NULL)
            {
                tmp[strcspn(tmp, "\n")] = '\0';
                for (int i = 0; env[i] != NULL; i++)
                {
                    char *str = strstr(env[i], tmp);
                    if (str != NULL)
                    {
                        printf("\t%s=%s\n", env[i], str+11);
                        break;
                    }
                }
            }
            break;
        }
        case '&':
        {
            //  считываем из файла название и ищем значение в environ
            while (fgets(tmp, 255, file) != NULL)
            {
                tmp[strcspn(tmp, "\n")] = '\0';
                for (int i = 0; environ[i] != NULL; i++)
                {
                    char *str = strstr(environ[i], tmp);
                    if (str != NULL)
                    {
                        printf("\t%s=%s\n", environ[i], str+11);
                        break;
                    }
                }
            }
            break;
        }
    }

    free(tmp);
    fclose(file);
}