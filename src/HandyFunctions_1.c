#define _GNU_SOURCE
#include "Header.h"

// Includes all command names.
bool IsCommand(char *command)
{
    if (!strncmp(command, "neogit revert ", 14))
        return true;
    if (!strncmp(command, "neogit tag", 10))
        return true;
    if (!strcmp(command, "neogit tree"))
        return true;
    if (!strncmp(command, "neogit stash push", 17))
        return true;
    if (!strncmp(command, "neogit stash show ", 18))
        return true;
    if (!strncmp(command, "neogit stash branch ", 20))
        return true;
    if (!strcmp(command, "neogit stash drop"))
        return true;
    if (!strcmp(command, "neogit stash clear"))
        return true;
    if (!strcmp(command, "neogit stash pop"))
        return true;
    if (!strcmp(command, "neogit stash list"))
        return true;
    if (!strcmp(command, "neogit pre-commit hooks list"))
        return true;
    if (!strcmp(command, "neogit pre-commit applied hooks"))
        return true;
    if (!strncmp(command, "neogit pre-commit add hook ", 27))
        return true;
    if (!strncmp(command, "neogit pre-commit remove hook ", 30))
        return true;
    if (!strcmp(command, "neogit pre-commit"))
        return true;
    if (!strcmp(command, "neogit pre-commit -u"))
        return true;
    if (!strncmp(command, "neogit pre-commit -f ", 21))
        return true;
    if (!strncmp(command, "neogit grep -f ", 15))
        return true;
    if (!strncmp(command, "neogit diff -f ", 15))
        return true;
    if (!strncmp(command, "neogit diff -c ", 15))
        return true;
    if (!strncmp(command, "neogit merge -b ", 16))
        return true;
    if (!strncmp(command, "neogit config -global user.name ", 32))
        return true;
    if (!strncmp(command, "neogit config -global user.email ", 33))
        return true;
    if (!strncmp(command, "neogit config user.name ", 24))
        return true;
    if (!strncmp(command, "neogit config user.email ", 25))
        return true;
    if (!strncmp(command, "neogit config alias.", 20))
        return true;
    if (!strncmp(command, "neogit config -global alias.", 29))
        return true;
    if (!strcmp(command, "neogit init"))
        return true;
    if (!strncmp(command, "neogit add ", 11))
        return true;
    if (!strncmp(command, "neogit reset ", 13))
        return true;
    if (!strcmp(command, "neogit status"))
        return true;
    if (!strncmp(command, "neogit commit -m ", 17))
        return true;
    if (!strncmp(command, "neogit commit -s ", 17))
        return true;
    if (!strncmp(command, "neogit set -m ", 14))
        return true;
    if (!strncmp(command, "neogit replace -m ", 18))
        return true;
    if (!strncmp(command, "neogit remove -s ", 17))
        return true;
    if (!strncmp(command, "neogit log ", 11))
        return true;
    if (!strncmp(command, "neogit checkout ", 16))
        return true;
    if (!strncmp(command, "neogit branch", 13))
        return true;
    return false;
}

// Creating path directory
void DirectoryMaker(char *path)
{
    if (access(path, F_OK) != 0)
        if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
        {
            perror("mkdir");
            return;
        }
    return;
}

// Creates and runs a bash for cp command
void BashCPMaker(char *source, char *destination)
{
    FILE *f = fopen("/home/saraghazavi/do.sh", "w");
    fprintf(f, "cp -r %s %s", source, destination);
    fclose(f);
    chmod("/home/saraghazavi/do.sh", 0x777);
    system("/home/saraghazavi/do.sh");
    remove("/home/saraghazavi/do.sh");
}

// Creates and runs a bash for rm command
void BashRMMaker(char *path)
{
    FILE *f = fopen("/home/saraghazavi/do.sh", "w");
    fprintf(f, "rm -r %s", path);
    fclose(f);
    chmod("/home/saraghazavi/do.sh", 0x777);
    system("/home/saraghazavi/do.sh");
    remove("/home/saraghazavi/do.sh");
}

// Comparing mode of files for neogit status T mode
mode_t ModeComparator(char *file1, char *file2)
{
    struct stat stat1, stat2;
    stat(file1, &stat1);
    stat(file2, &stat2);
    return stat1.st_mode == stat2.st_mode;
}

// Returns 1 if the repository exists in given address or any parent directories. Returns 0 if not.
int RepoFinder(char *path)
{
    if (!path || !(*path))
        return 0;
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL)
        return 0;

    while ((entry = readdir(dir)) != NULL)
        if (!strcmp(entry->d_name, ".neogit"))
            return 1;
    closedir(dir);
    path[strlen(path)] = '\0';
    char *slash = strrchr(path, '/');
    if (slash != NULL)
        *slash = '\0';
    return RepoFinder(path);
}

// Changing user name in config file
void ChangeName(char *info, char *name)
{
    FILE *fp_r = fopen(info, "r");
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), fp_r))
        if (strncmp(line, "email =", 7) == 0)
            break;
    fclose(fp_r);
    FILE *fp_w = fopen(info, "w");
    fprintf(fp_w, "name =%s\n%s", name, line);
    fclose(fp_w);
    printf("User name added successfully!\n");
}

// Changing user email in config file
void ChangeEmail(char *info, char *email)
{
    FILE *fp_r = fopen(info, "r");
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), fp_r))
        if (strncmp(line, "name =", 6) == 0)
            break;
    fclose(fp_r);
    FILE *fp_w = fopen(info, "w");
    fprintf(fp_w, "%semail =%s\n", line, email);
    fclose(fp_w);
    printf("User email added successfully!\n");
}

// Returns 1 if the given files are same, returns 0 if not.
int IsChanged(char *file1, char *file2)
{
    int char1, char2;
    FILE *fp_1 = fopen(file1, "rb");
    if (fp_1 == NULL)
        return 0;
    FILE *fp_2 = fopen(file2, "rb");
    if (fp_2 == NULL)
    {
        fclose(fp_1);
        return 0;
    }

    while ((char1 = fgetc(fp_1)) != EOF && (char2 = fgetc(fp_2)) != EOF)
        if (char1 != char2)
        {
            fclose(fp_1);
            fclose(fp_2);
            return 0;
        }
    char2 = fgetc(fp_2);
    if (char1 != char2)
    {
        fclose(fp_1);
        fclose(fp_2);
        return 0;
    }
    fclose(fp_1);
    fclose(fp_2);
    return 1;
}

// Recieving add ID string and change it to integer
int PassAddID(char *REPOSITORY)
{
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/current_IDs.txt", REPOSITORY);
    int IntForm = 0;
    char line[SMALL_SIZE];
    FILE *f = fopen(path, "r");
    fgets(line, sizeof(line), f);
    for (int i = 0; i < strlen(line) - 1; i++)
    {
        IntForm *= 10;
        IntForm += (line[i] - '0');
    }
    fclose(f);
    return IntForm;
}

// Recieving commit ID string and change it to integer
int PassCommitID(char *REPOSITORY)
{
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/current_IDs.txt", REPOSITORY);
    int IntForm = 0;
    char line[SMALL_SIZE];
    FILE *f = fopen(path, "r");
    fgets(line, sizeof(line), f);
    fgets(line, sizeof(line), f);
    for (int i = 0; i < strlen(line) - 1; i++)
    {
        IntForm *= 10;
        IntForm += (line[i] - '0');
    }
    fclose(f);
    return IntForm;
}

// Increasing or decreasing add ID depending on sign.
void CalculateAddID(char *REPOSITORY, char sign)
{
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *f = fopen(path, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE], curCommit[SMALL_SIZE];
    fgets(addID, sizeof(addID), f);
    fgets(commitID, sizeof(commitID), f);
    fgets(curCommit, sizeof(curCommit), f);
    fclose(f);

    int idx = strlen(addID) - 2;
    if (sign == '+')
    {
        while (addID[idx] == '9')
        {
            addID[idx] = '0';
            idx--;
        }
        addID[idx]++;
    }
    else
    {
        while (addID[idx] == '0')
        {
            addID[idx] = '9';
            idx--;
        }
        addID[idx]--;
    }
    f = fopen(path, "w");
    fputs(addID, f);
    fputs(commitID, f);
    fputs(curCommit, f);
    fclose(f);
    return;
}

// Increasing or decreasing commit ID depending on sign.
void CalculateCommitID(char *REPOSITORY, char sign)
{
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *current_IDs = fopen(path, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE], curCommit[SMALL_SIZE];
    fgets(addID, sizeof(addID), current_IDs);
    fgets(commitID, sizeof(commitID), current_IDs);
    fgets(curCommit, sizeof(curCommit), current_IDs);
    fclose(current_IDs);

    int idx = strlen(commitID) - 2;
    if (sign == '+')
    {
        while (commitID[idx] == '9')
        {
            commitID[idx] = '0';
            idx--;
        }
        commitID[idx]++;
    }
    else
    {
        while (commitID[idx] == '0')
        {
            commitID[idx] = '9';
            idx--;
        }
        commitID[idx]--;
    }
    current_IDs = fopen(path, "w");
    fputs(addID, current_IDs);
    fputs(commitID, current_IDs);
    fputs(curCommit, current_IDs);
    fclose(current_IDs);
    return;
}

/*
    Takes a path and a time.
    Returns 1 if file is created before time.
    Returns -1 if file is created after time.
*/
int CompareFileTime(char *file, char *time)
{
    struct stat file_s;
    stat(file, &file_s);
    time_t file_t = file_s.st_ctime;
    struct tm inputTime;
    strptime(time, "%Y/%m/%d %H:%M:%S", &inputTime);
    time_t inputTimeValue = mktime(&inputTime);
    if (inputTimeValue < file_t)
        return -1;
    else if (inputTimeValue > file_t)
        return 1;
    return 0;
}

// Counts files in a path.
void FileCounter(char *path, int *counter)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            FileCounter(path, counter);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
            (*counter)++;
    }
    while (path[strlen(path) - 1] != '/')
        path[strlen(path) - 1] = '\0';
    path[strlen(path) - 1] = '\0';
    closedir(dir);
    return;
}

// Updates the ID of a particular branch in branch.txt.
void IDUpdate(char *REPOSITORY, char *branch, int ID)
{
    char target[MEDIUM_SIZE];
    sprintf(target, "%s_", branch);

    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *f = fopen(path, "r");
    char line[MEDIUM_SIZE][BUFF_SIZE];
    int idx = -1;
    while (fgets(line[++idx], sizeof(line[idx]), f))
        ;
    fclose(f);
    f = fopen(path, "w");
    for (int i = 0; i < idx; i++)
    {
        if (!strncmp(line[i], target, strlen(target)))
        {
            int tok = strlen(line[i]) - 1;
            while (line[i][tok] != '_')
                tok--;
            sprintf(line[i] + tok, "_%d\n", ID);
        }
        fputs(line[i], f);
    }
    fclose(f);
    return;
}

/*
    1. Checks if the branch already exists.
    2. Addes the branch. Branch's creation ID and last ID are head's last ID.
*/
void AddBranch(char *name, char *REPOSITORY)
{
    char head[PATH_MAX];
    sprintf(head, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(head, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    fclose(h_f);
    char branch[PATH_MAX];
    sprintf(branch, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(branch, "r");
    char tmp[MEDIUM_SIZE];
    while (fgets(tmp, sizeof(tmp), b_f))
    {
        strtok(tmp, "_");
        if (!strcmp(tmp, name))
        {
            printf("Branch already exists!\n");
            return;
        }
    }
    fclose(b_f);

    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    b_f = fopen(branch, "r");
    char *headID;
    while (fgets(tmp, sizeof(tmp), b_f))
    {
        strtok(tmp, "_");
        if (!strcmp(tmp, HEAD))
        {
            strtok(NULL, "_");
            headID = strtok(NULL, "\n");
            break;
        }
    }
    fclose(b_f);

    b_f = fopen(branch, "a");
    fprintf(b_f, "%s_%s_%s\n", name, headID, headID);
    fclose(b_f);
    printf("New branch " CYAN "%s" FORMAT_RESET " created " GREEN "successfully!\n" FORMAT_RESET, name);
    return;
}

// Prints all branches. Shows the head using "--> HEAD".
void BranchList(char *REPOSITORY)
{
    char h_p[PATH_MAX];
    sprintf(h_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(h_p, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    fclose(h_f);

    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char branch[MEDIUM_SIZE];
    while (fgets(branch, sizeof(branch), b_f))
    {
        strtok(branch, "_");
        printf("\t%s", branch);
        if (!strcmp(branch, HEAD))
            printf("\t-> " RED "HEAD" FORMAT_RESET);
        printf("\n");
    }
    return;
}

// Checks if command has been set to alias. Uses most recent one.
int CheckAlias(char *entrance, char *REPOSITORY)
{
    char *home = getenv("HOME");
    char g_p[PATH_MAX];
    sprintf(g_p, "%s/.neogitconfig/alias", home);
    DirectoryMaker(g_p);
    char command[MEDIUM_SIZE];
    sprintf(command, "%s.txt", entrance);

    DIR *g_dir = opendir(g_p);
    char g_final[PATH_MAX];
    struct dirent *entry;
    while ((entry = readdir(g_dir)) != NULL)
        if (!strcmp(entry->d_name, command))
            sprintf(g_final, "%s/%s", g_p, entry->d_name);
    closedir(g_dir);

    char l_p[PATH_MAX];
    sprintf(l_p, "%s/.neogit/alias", REPOSITORY);

    DIR *l_dir = opendir(l_p);
    char l_final[PATH_MAX];
    struct dirent *walking;
    while ((walking = readdir(l_dir)) != NULL)
        if (!strcmp(walking->d_name, command))
            sprintf(l_final, "%s/%s", l_p, walking->d_name);
    closedir(l_dir);

    FILE *f;
    if (g_final[0] != 0 && l_final[0] != 0)
    {
        struct stat g_s, l_s;
        stat(g_final, &g_s);
        stat(l_final, &l_s);
        if (g_s.st_mtime > l_s.st_mtime)
            f = fopen(g_final, "r");
        else if (g_s.st_mtime < l_s.st_mtime)
            f = fopen(l_final, "r");
    }
    else if (g_final[0] != 0)
        f = fopen(g_final, "r");
    else if (l_final[0] != 0)
        f = fopen(l_final, "r");
    else
        return 0;

    char line[BUFF_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);
    f = fopen("/home/saraghazavi/do.sh", "w");
    fprintf(f, "%s", line);
    fclose(f);
    chmod("/home/saraghazavi/do.sh", 0x777);
    system("/home/saraghazavi/do.sh");
    remove("/home/saraghazavi/do.sh");
    return 1;
}

// For sorting tags.
int Sorter(const struct dirent **a, const struct dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

// Checks if a line is valid or it's just \n or \t or space.
int ValidLine(char *line)
{
    for (int i = 0; i < strlen(line); i++)
        if (line[i] != '\n' && line[i] != '\t' && line[i] != ' ' && line[i] != '\r')
            return 1;
    return 0;
}

// Return true if the word matches wildcard
bool Wildcard(char *wildcard, char *word)
{
    while (*wildcard && *word)
    {
        if (*wildcard == *word || *wildcard == '?')
        {
            wildcard++;
            word++;
        }
        else if (*wildcard == '*')
        {
            while (*wildcard == '*')
                wildcard++;
            if (*wildcard == '\0')
                return true;
            while (*word && *word != *wildcard)
                word++;
        }
        else
            return false;
    }
    return (*wildcard == '\0' && *word == '\0');
}

// Compare two lines and print differences
int CompareLine(char *line_1, char *line_2, char *file_1, char *file_2, int which_1, int which_2)
{
    char *token;
    char **words_1 = NULL;
    int idx_1 = 0;
    token = strtok(line_1, " \n\t\r");
    while (token != NULL)
    {
        words_1 = (char **)realloc(words_1, (idx_1 + 1) * sizeof(char *));
        words_1[idx_1] = (char *)malloc(strlen(token) + 3);
        strcpy(words_1[idx_1], token);
        idx_1++;
        token = strtok(NULL, " \n\t\r");
    }
    char **words_2 = NULL;
    int idx_2 = 0;
    token = strtok(line_2, " \n\t\r");
    while (token != NULL)
    {
        words_2 = (char **)realloc(words_2, (idx_2 + 1) * sizeof(char *));
        words_2[idx_2] = (char *)malloc(strlen(token) + 3);
        strcpy(words_2[idx_2], token);
        idx_2++;
        token = strtok(NULL, " \n\t\r");
    }

    int minIDX = (idx_1 > idx_2) ? idx_2 : idx_1;
    int tmp_idx, ctr = 0;
    for (int i = 0; i < minIDX; i++)
        if (strcmp(words_1[i], words_2[i]))
        {
            ctr++;
            tmp_idx = i;
        }

    int difNum = (idx_1 > idx_2) ? idx_1 - idx_2 : idx_2 - idx_1;

    if (difNum == 0)
    {
        if (ctr == 0)
            return 0;
        else if (ctr == 1)
        {
            char tmp[50];
            sprintf(tmp, ">%s<", words_1[tmp_idx]);
            strncpy(words_1[tmp_idx], tmp, strlen(tmp));
            sprintf(tmp, ">%s<", words_2[tmp_idx]);
            strcpy(words_2[tmp_idx], tmp);

            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d\n" FORMAT_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++)
            {
                if (!strncmp(">", words_1[i], 1))
                    printf(RED "%s " FORMAT_RESET, words_1[i]);
                else
                    printf(CYAN "%s " FORMAT_RESET, words_1[i]);
            }
            printf("\n");
            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d" FORMAT_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++)
            {
                if (!strncmp(">", words_2[i], 1))
                    printf(RED "%s " FORMAT_RESET, words_2[i]);
                else
                    printf(BLUE "%s " FORMAT_RESET, words_2[i]);
            }
            printf("\n\n");
        }
        else
        {
            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d\n" FORMAT_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++)
                printf(CYAN "%s " FORMAT_RESET, words_1[i]);
            printf("\n");
            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d" FORMAT_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++)
                printf(BLUE "%s " FORMAT_RESET, words_2[i]);
            printf("\n\n");
        }
    }
    else if (difNum == 1)
    {
        if (ctr != 0)
        {
            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d\n" FORMAT_RESET,
                   file_1, which_1);
            printf("\t");
            for (int i = 0; i < idx_1; i++)
                printf(CYAN "%s " FORMAT_RESET, words_1[i]);
            printf("\n");
            printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                          "%d" FORMAT_RESET "\n\t",
                   file_2, which_2);
            for (int i = 0; i < idx_2; i++)
                printf(BLUE "%s " FORMAT_RESET, words_2[i]);
            printf("\n\n");
        }
        else
        {
            if (idx_1 > idx_2)
            {
                printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                              "%d\n" FORMAT_RESET,
                       file_1, which_1);
                printf("\t");
                for (int i = 0; i < idx_1 - 1; i++)
                    printf(CYAN "%s " FORMAT_RESET, words_1[i]);
                printf(RED ">%s< " FORMAT_RESET, words_1[idx_1 - 1]);
                printf("\n");
                printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                              "%d" FORMAT_RESET "\n\t",
                       file_2, which_2);
                for (int i = 0; i < idx_2; i++)
                    printf(BLUE "%s " FORMAT_RESET, words_2[i]);
                printf("\n\n");
            }
            else
            {
                printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                              "%d\n" FORMAT_RESET,
                       file_1, which_1);
                printf("\t");
                for (int i = 0; i < idx_1; i++)
                    printf(CYAN "%s " FORMAT_RESET, words_1[i]);
                printf("\n");
                printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                              "%d" FORMAT_RESET "\n\t",
                       file_2, which_2);
                for (int i = 0; i < idx_2 - 1; i++)
                    printf(BLUE "%s " FORMAT_RESET, words_2[i]);
                printf(RED ">%s< " FORMAT_RESET, words_2[idx_2 - 1]);
                printf("\n\n");
            }
        }
    }
    else
    {
        printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                      "%d\n" FORMAT_RESET,
               file_1, which_1);
        printf("\t");
        for (int i = 0; i < idx_1; i++)
            printf(CYAN "%s " FORMAT_RESET, words_1[i]);
        printf("\n");
        printf(YELLOW "%s" FORMAT_RESET " - line: " GREEN
                      "%d" FORMAT_RESET "\n\t",
               file_2, which_2);
        for (int i = 0; i < idx_2; i++)
            printf(BLUE "%s " FORMAT_RESET, words_2[i]);
        printf("\n\n");
    }

    for (int i = 0; i < idx_1; i++)
        free(words_1[i]);
    free(words_1);

    for (int i = 0; i < idx_2; i++)
        free(words_2[i]);
    free(words_2);
    return 1;
}
