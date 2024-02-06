#define _GNU_SOURCE
#include "Header.h"

/*
    1. Checks if tag exists & we cannot change tag.
    2. Gets username and useremail.
    3. Gets ID if it hasn't been given.
    4. Creats tag.
*/
void AddTag(char *tagname, char *message, char *ID, int can_change, char *REPOSITORY)
{
    // Checking existence.
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/tags/%s.txt", REPOSITORY, tagname);
    if (access(path, F_OK) == 0 && !can_change)
    {
        printf("Tag already " RED "exists!\n" FORMAT_RESET);
        return;
    }

    // Getting name and username.
    char username[MEDIUM_SIZE], useremail[MEDIUM_SIZE];
    char global_p[PATH_MAX], local_p[PATH_MAX];
    sprintf(global_p, "/home/saraghazavi/.neogitconfig/info.txt");
    sprintf(local_p, "%s/.neogit/config.txt", REPOSITORY);

    FILE *config;
    struct stat global_s, local_s;
    stat(global_p, &global_s);
    stat(local_p, &local_s);
    if (global_s.st_mtime > local_s.st_mtime)
        config = fopen(global_p, "r");
    else if (global_s.st_mtime < local_s.st_mtime)
        config = fopen(local_p, "r");

    fgets(username, sizeof(username), config);
    fgets(useremail, sizeof(useremail), config);
    fclose(config);

    strtok(username, "=");
    char *name = strtok(NULL, "\n");
    strtok(useremail, "=");
    char *email = strtok(NULL, "\n");

    if (name == NULL || email == NULL)
    {
        printf("You haven't " RED
               "set" FORMAT_RESET
               " config yet! " YELLOW "Please set and try again!\n" FORMAT_RESET);
        return;
    }

    // Getting ID if it hasn't been given.
    if (ID == NULL)
    {
        ID = (char *)malloc(SMALL_SIZE);
        char IDs_p[1024];
        sprintf(IDs_p, "%s/.neogit/current_IDs.txt", REPOSITORY);
        FILE *ID_f = fopen(IDs_p, "r");
        fgets(ID, sizeof(ID), ID_f);
        fgets(ID, sizeof(ID), ID_f);
        fgets(ID, sizeof(ID), ID_f);
        if (ID[strlen(ID) - 1] == '\n')
            ID[strlen(ID) - 1] = '\0';
        fclose(ID_f);
    }

    FILE *f = fopen(path, "w");
    if (message == NULL)
        fprintf(f, "AuthorName =%s\nAuthorEmail =%s\nCommitID =%s\nMessage =-\n", name, email, ID);
    else
        fprintf(f, "AuthorName =%s\nAuthorEmail =%s\nCommitID =%s\nMessage =%s\n", name, email, ID, message);
    fclose(f);
    printf("Tag created " GREEN "successfully!\n" FORMAT_RESET);
    return;
}

// List tags alphabetically.
void ListTags(char *REPOSITORY)
{
    printf(MAGENTA "Tags :\n" FORMAT_RESET);
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/tags", REPOSITORY);
    DIR *dir = opendir(path);
    struct dirent **entry;
    int n = scandir(path, &entry, NULL, Sorter);
    for (int i = 0; i < n; i++)
    {
        if (strcmp(entry[i]->d_name, ".") && strcmp(entry[i]->d_name, ".."))
            printf(BOLD "\t%s\n" FORMAT_RESET, entry[i]->d_name);
        free(entry[i]);
    }
    free(entry);
    closedir(dir);
    return;
}

// Shows information of a tag. Similar to log.
void ShowTag(char *tagname, char *REPOSITORY)
{
    char path[PATH_MAX];
    sprintf(path, "%s/.neogit/tags/%s.txt", REPOSITORY, tagname);

    if (access(path, F_OK))
    {
        printf("Tag doesn't " RED
               "exist!\n" FORMAT_RESET);
        return;
    }

    printf("tag " MAGENTA "%s\n" FORMAT_RESET, tagname);
    FILE *f = fopen(path, "r");

    char line1[BUFF_SIZE];
    fgets(line1, sizeof(line1), f);
    strtok(line1, "=\n");
    char *name = strtok(NULL, "=\n");

    char line2[BUFF_SIZE];
    fgets(line2, sizeof(line2), f);
    strtok(line2, "=\n");
    char *email = strtok(NULL, "=\n");

    char line3[BUFF_SIZE];
    fgets(line3, sizeof(line3), f);
    strtok(line3, "=\n");
    char *ID = strtok(NULL, "=\n");

    char line4[BUFF_SIZE];
    fgets(line4, sizeof(line4), f);
    strtok(line4, "=\n");
    char *message = strtok(NULL, "=\n");
    fclose(f);

    struct stat folder;
    stat(path, &folder);
    time_t crt = folder.st_ctime;

    printf("commit " YELLOW "%s\n" FORMAT_RESET, ID);
    printf("Author: %s " DIM
           "<%s>" FORMAT_RESET
           "\n",
           name, email);
    printf("Date:\t" BLUE "%s" FORMAT_RESET, ctime(&crt));
    printf(BOLD "Message: %s\n\n" FORMAT_RESET, message);

    return;
}

/*
    1. Getting head and it's last commit ID.
    2. Because it works like stack, it renames all folder names in .stash to next their number.
    3. Creates 0 folder, copies working directory and clears working directory.
    4. Imports head's last commit to working directory.
    5. Changes commit ID pointer.
    6. Writes info.txt for stash.
*/
void StashPush(bool check, char *message, char *REPOSITORY)
{
    // Getting head.
    char h_p[PATH_MAX];
    sprintf(h_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(h_p, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    fclose(h_f);

    // Getting head's last commit ID.
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char line[BUFF_SIZE];
    char *ID;
    while (fgets(line, sizeof(line), b_f) != NULL)
    {
        strtok(line, "_");
        if (!strcmp(line, HEAD))
        {
            strtok(NULL, "_");
            ID = strtok(NULL, "\n");
            break;
        }
    }
    fclose(b_f);

    // Rename all folder names to next number.
    char stash[PATH_MAX];
    sprintf(stash, "%s/.neogit/.stash", REPOSITORY);
    DIR *dir = opendir(stash);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            char previous[PATH_MAX];
            sprintf(previous, "%s/%s", stash, entry->d_name);
            int number_name = 0;
            for (int i = 0; i < strlen(entry->d_name); i++)
            {
                number_name *= 10;
                number_name += (entry->d_name)[i] - '0';
            }
            number_name++;
            char next[PATH_MAX];
            sprintf(next, "%s/%d", stash, number_name);
            rename(previous, next);
        }
    }
    closedir(dir);

    // Copy working directory in folder 0 in stash and clear working directory.
    char new[PATH_MAX];
    sprintf(new, "%s/0", stash);
    mkdir(new, S_IRWXU | S_IRWXG | S_IRWXO);

    DIR *wd_dir = opendir(REPOSITORY);
    while ((entry = readdir(wd_dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            char path[PATH_MAX];
            sprintf(path, "%s/%s", REPOSITORY, entry->d_name);
            BashCPMaker(path, new);
            BashRMMaker(path);
        }
    }
    closedir(wd_dir);

    // Import head's last commit to working directory.
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, ID);
    DIR *cm_dir = opendir(commit);
    while ((entry = readdir(cm_dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, "info.txt"))
        {
            char path[PATH_MAX];
            sprintf(path, "%s/%s", commit, entry->d_name);
            BashCPMaker(path, REPOSITORY);
        }
    }
    closedir(cm_dir);

    // Change commit ID pointer.
    char IDs_p[PATH_MAX];
    sprintf(IDs_p, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(IDs_p, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE], curID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fgets(curID, sizeof(curID), ID_f);
    fclose(ID_f);

    ID_f = fopen(IDs_p, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%s\n", ID);
    fclose(ID_f);

    // Write info for stash.
    char info[PATH_MAX];
    sprintf(info, "%s/info.txt", new);
    FILE *f = fopen(info, "w");
    if (check)
        fprintf(f, "%s:%s:%s", HEAD, message, ID);
    else
        fprintf(f, "%s:-:%s", HEAD, ID);
    fclose(f);
    printf("Stash created " GREEN "successfully!\n" FORMAT_RESET);
    return;
}

// Shows all stash entries in stack form.
void StashList(char *REPOSITORY)
{
    printf("Stash entries:\n");
    char stash[PATH_MAX];
    sprintf(stash, "%s/.neogit/.stash", REPOSITORY);
    DIR *dir = opendir(stash);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            char info[PATH_MAX];
            sprintf(info, "%s/%s/info.txt", stash, entry->d_name);
            FILE *f = fopen(info, "r");
            char line[BUFF_SIZE];
            fgets(line, sizeof(line), f); // branch / message / id
            strtok(line, ":");
            printf(YELLOW "\t%s:" FORMAT_RESET " Branch: %s\n", entry->d_name, line);
            char *message = strtok(NULL, ":\n");
            printf(BOLD "\t\tMessage: %s\n" FORMAT_RESET, message);
        }
    }
    closedir(dir);
    return;
}

/*
    Moves toward stash and commit.
    While moving on stash, prints only-stash files and in-common file.
    While moving on commit, prints only-commit files.
*/
void StashShow(char *idx, char *REPOSITORY)
{
    char stash[PATH_MAX];
    sprintf(stash, "%s/.neogit/.stash/%s", REPOSITORY, idx);

    char info[PATH_MAX];
    sprintf(info, "%s/info.txt", stash);

    FILE *f = fopen(info, "r");
    char line[BUFF_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);
    strtok(line, ":");
    strtok(NULL, ":");
    char *ID = strtok(NULL, "\n");

    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, ID);

    char copy_stash[PATH_MAX];
    strcpy(copy_stash, stash);
    StashMove(stash, copy_stash, commit, 1);

    char copy_commit[PATH_MAX];
    strcpy(copy_commit, commit);
    StashMove(commit, copy_commit, stash, 0);

    return;
}

/*
    Moves through two pathes.
    Used for stash show.
    Used for merge branches.
    It also returns a value: true for same files, false for difference discovery.
*/
int StashMove(char *main, char *go_in, char *compare, int mode)
{
    bool check = true;

    DIR *dir = opendir(go_in);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(go_in, "/");
            strcat(go_in, entry->d_name);
            check = StashMove(main, go_in, compare, mode);

            while (go_in[strlen(go_in) - 1] != '/')
                go_in[strlen(go_in) - 1] = '\0';
            go_in[strlen(go_in) - 1] = '\0';
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            strcat(go_in, "/");
            strcat(go_in, entry->d_name);

            // Create filepath in TargetCompare directory.
            char target[PATH_MAX];
            sprintf(target, "%s%s", compare, go_in + strlen(main));

            // If mode = 1, it means moving on STASH. (For STASH command)
            // Check all files and if they exist in commit, check the difference.
            if (mode == 1)
            {
                // If it exists, check difference.
                if (access(target, F_OK) == 0)
                    Diff(go_in, target, 1, 10000, 1, 10000);
                // If it doesn't exist, print it.
                else
                {
                    printf("%s is only in Stash. It's " RED
                           "not" FORMAT_RESET " in Commit :\n",
                           entry->d_name);
                    FILE *f = fopen(go_in, "r");
                    char line[BUFF_SIZE];
                    while (fgets(line, sizeof(line), f) != NULL)
                        printf("\t%s", line);
                    fclose(f);
                    printf("\n\n");
                }
            }
            // If mode = 0, it means  moving on COMMITS. (For STASH command)
            // Ignore in common files because they've been proccessed.
            else if (mode == 0 && access(target, F_OK) != 0)
            {
                printf("%s is only in Commit. It's " RED "not" FORMAT_RESET " in Stash :\n", entry->d_name);
                FILE *f = fopen(go_in, "r");
                char line[BUFF_SIZE];
                while (fgets(line, sizeof(line), f) != NULL)
                    printf("\t%s", line);
                fclose(f);
                printf("\n\n");
            }
            // If mode = 2, it means we just want to check the difference between files in two directories.
            // Used in merge.
            else if (mode == 2)
            {
                if (access(target, F_OK) == 0)
                {
                    if (Diff(go_in, target, 1, 100000, 1, 100000) == 0)
                        check = false;
                }
            }
            while (go_in[strlen(go_in) - 1] != '/')
                go_in[strlen(go_in) - 1] = '\0';
            go_in[strlen(go_in) - 1] = '\0';
        }
    }
    closedir(dir);
    return check;
}

/*
    1. If there is no conflict between stash and working directory, imports the stash into working directory.
    2. Removes the particular stash.
    3. Rename all stashes after that stash.
*/
void StashPop(char *index, char *REPOSITORY)
{
    char stash[PATH_MAX];
    sprintf(stash, "%s/.neogit/.stash/%s", REPOSITORY, index);
    char copy_stash[PATH_MAX];
    strcpy(copy_stash, stash);
    // If there is no conflict between working directory and stash, clear working directory and import stash.
    if (StashMove(stash, copy_stash, REPOSITORY, 2))
    {
        printf("%s stash has been imported to working directory!\n", index);
        DIR *wd_dir = opendir(REPOSITORY);
        struct dirent *entry;
        while ((entry = readdir(wd_dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", REPOSITORY, entry->d_name);
                BashRMMaker(path);
            }
        }
        closedir(wd_dir);
        DIR *st_dir = opendir(stash);
        while ((entry = readdir(st_dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, "info.txt"))
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", stash, entry->d_name);
                BashCPMaker(path, REPOSITORY);
            }
        }
        closedir(st_dir);

        // Removing stash.
        BashRMMaker(stash);

        // Renaming stashes that have bigger number than the removed stash.
        sprintf(stash, "%s/.neogit/.stash", REPOSITORY);
        int idx = 0;
        for (int i = 0; i < strlen(index); i++)
        {
            idx *= 10;
            idx += index[i] - '0';
        }
        DIR *dir = opendir(stash);
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            {
                char next[PATH_MAX];
                sprintf(next, "%s/%s", stash, entry->d_name);
                int number_name = 0;
                for (int i = 0; i < strlen(entry->d_name); i++)
                {
                    number_name *= 10;
                    number_name += (entry->d_name)[i] - '0';
                }
                // Stop when you are at target index
                if (number_name < idx)
                    continue;
                number_name--;
                char previous[PATH_MAX];
                sprintf(previous, "%s/%d", stash, number_name);
                rename(next, previous);
            }
        }
        closedir(dir);
    }
    else
    {
        printf("You can't pop stash entry. " RED
               "CONFLICT ERROR!\n" FORMAT_RESET);
        printf("\tPlease resolve conflicts and " BLUE "try again!\n" FORMAT_RESET);
    }

    return;
}

/*
    1. Opens file path.
    2. Reads the file line by line.
    3. Tokenizes line, and then look for the target word. (Also has wildcard)
    4. Puts > before found word and prints.
*/
void Grep(char *path, char *target, bool show_num)
{
    char *name = strrchr(path, '/');
    if (name != NULL)
        name++;
    else
        name = path;
    printf("Found in " YELLOW "%s :\n" FORMAT_RESET, name);

    FILE *f = fopen(path, "r");
    int ln = 1;
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), f))
    {
        bool check = false;
        char *token;
        char **words = NULL;
        int idx = 0;
        token = strtok(line, " \n\t\r");
        while (token != NULL)
        {
            words = (char **)realloc(words, (idx + 1) * sizeof(char *));
            words[idx] = (char *)malloc(strlen(token) + 2);
            strcpy(words[idx], token);
            idx++;
            token = strtok(NULL, " \n\t\r");
        }
        for (int i = 0; i < idx; i++)
        {
            if (strstr(target, "*"))
            {
                if (Wildcard(target, words[i]))
                {
                    char tmp[MEDIUM_SIZE];
                    sprintf(tmp, ">%s", words[i]);
                    strcpy(words[i], tmp);
                    check = true;
                }
            }
            else
            {
                if (!strcmp(target, words[i]))
                {
                    char tmp[50];
                    sprintf(tmp, ">%s", words[i]);
                    strcpy(words[i], tmp);
                    check = true;
                }
            }
        }
        if (check)
        {
            if (show_num)
                printf("%d:", ln);
            printf("\t");
            for (int i = 0; i < idx; i++)
            {
                if (!strncmp(words[i], ">", 1))
                    printf(MAGENTA "%s " FORMAT_RESET, words[i] + 1);
                else
                    printf("%s ", words[i]);
            }
            printf("\n");
        }
        for (int i = 0; i < idx; i++)
            free(words[i]);
        free(words);
        ln++;
    }
}

// DFS on commit ID and goes for simple Grep.
void GrepCommit(char *commit, bool show_num, char *target, char *file)
{
    DIR *dir = opendir(commit);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(commit, "/");
            strcat(commit, entry->d_name);
            GrepCommit(commit, show_num, target, file);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            if (!strcmp(entry->d_name, file))
            {
                char tmp[PATH_MAX];
                sprintf(tmp, "%s/%s", commit, entry->d_name);
                Grep(tmp, target, show_num);
            }
        }
    }
    while (commit[strlen(commit) - 1] != '/')
        commit[strlen(commit) - 1] = '\0';
    commit[strlen(commit) - 1] = '\0';
    closedir(dir);
    return;
}

// Moves through first commit and prints all files which doesn't exist in second commit.
void DiffOne(char *path_1, char *path_2)
{
    DIR *dir = opendir(path_1);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(path_1, "/");
            strcat(path_1, entry->d_name);
            strcat(path_2, "/");
            strcat(path_2, entry->d_name);
            DiffOne(path_1, path_2);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            char file_1[PATH_MAX], file_2[PATH_MAX];
            sprintf(file_1, "%s/%s", path_1, entry->d_name);
            sprintf(file_2, "%s/%s", path_2, entry->d_name);

            if (access(file_2, F_OK) != 0)
            {
                char *name_1 = strrchr(file_1, '/');
                if (name_1 != NULL)
                    name_1++;
                else
                    name_1 = file_1;
                printf(YELLOW "%s :\n" FORMAT_RESET, name_1);
                FILE *f = fopen(file_1, "r");
                char line[1024];
                while (fgets(line, sizeof(line), f) != NULL)
                    printf("%s", line);
                printf("\n\n");
                fclose(f);
            }
        }
    }
    while (path_1[strlen(path_1) - 1] != '/')
        path_1[strlen(path_1) - 1] = '\0';
    path_1[strlen(path_1) - 1] = '\0';
    while (path_2[strlen(path_2) - 1] != '/')
        path_2[strlen(path_2) - 1] = '\0';
    path_2[strlen(path_2) - 1] = '\0';
    closedir(dir);
    return;
}

// Moves through first commit and does DIFF on all files which exist in second commit.
void DiffTwo(char *path_1, char *path_2)
{
    DIR *dir = opendir(path_1);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(path_1, "/");
            strcat(path_1, entry->d_name);
            strcat(path_2, "/");
            strcat(path_2, entry->d_name);
            DiffTwo(path_1, path_2);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            char file_1[PATH_MAX], file_2[PATH_MAX];
            sprintf(file_1, "%s/%s", path_1, entry->d_name);
            sprintf(file_2, "%s/%s", path_2, entry->d_name);
            if (access(file_2, F_OK) == 0)
                Diff(file_1, file_2, 1, 10000, 1, 10000);
        }
    }
    while (path_1[strlen(path_1) - 1] != '/')
        path_1[strlen(path_1) - 1] = '\0';
    path_1[strlen(path_1) - 1] = '\0';
    while (path_2[strlen(path_2) - 1] != '/')
        path_2[strlen(path_2) - 1] = '\0';
    path_2[strlen(path_2) - 1] = '\0';
    closedir(dir);
    return;
}

/*
    1. Calculates each file's lines to make sure end numbers are correct.
    2. Reads and ignors lines untill it reaches begin.
    3. Starts reading each VALID line and passing to CompareLine function.
    4. Marks a value for merge function.
*/
int Diff(char *file1, char *file2, int begin_1, int end_1, int begin_2, int end_2)
{
    FILE *fptr_1 = fopen(file1, "r");
    FILE *fptr_2 = fopen(file2, "r");

    if (fptr_1 == NULL || fptr_2 == NULL)
    {
        perror("Error opening files");
        return -1;
    }

    char *name_1 = strrchr(file1, '/');
    char *name_2 = strrchr(file2, '/');
    if (name_1 != NULL)
        name_1++;
    else
        name_1 = file1;
    if (name_2 != NULL)
        name_2++;
    else
        name_2 = file2;

    char line_1[BUFF_SIZE], line_2[BUFF_SIZE];
    int total_lines_1 = 0, total_lines_2 = 0;

    while (fgets(line_1, sizeof(line_1), fptr_1))
        if (ValidLine(line_1))
            total_lines_1++;

    while (fgets(line_2, sizeof(line_2), fptr_2))
        if (ValidLine(line_2))
            total_lines_2++;

    if (end_1 > total_lines_1)
        end_1 = total_lines_1;
    if (end_2 > total_lines_2)
        end_2 = total_lines_2;

    rewind(fptr_1);
    rewind(fptr_2);

    for (int i = 1; i < begin_1; i++)
        do
            fgets(line_1, sizeof(line_1), fptr_1);
        while (!ValidLine(line_1));
    for (int i = 1; i < begin_2; i++)
        do
            fgets(line_2, sizeof(line_2), fptr_2);
        while (!ValidLine(line_2));

    int len_1 = end_1 - begin_1;
    int len_2 = end_2 - begin_2;
    bool is_diff = false;

    if (len_2 > len_1)
    {
        for (int i = begin_1; i <= end_1; i++)
        {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!ValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!ValidLine(line_2));
            CompareLine(line_1, line_2, name_1, name_2, i, i - begin_1 + begin_2);
        }
        printf("\nLines in second file that does " RED "NOT" FORMAT_RESET " exist in first one:\n");
        while (fgets(line_2, sizeof(line_2), fptr_2))
            if (ValidLine(line_2))
            {
                printf(BLUE "%d:\t%s" FORMAT_RESET, end_2 - 1, line_2);
                end_2++;
            }
        is_diff = true;
    }
    else if (len_2 < len_1)
    {
        for (int i = begin_2; i <= end_2; i++)
        {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!ValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!ValidLine(line_2));
            CompareLine(line_1, line_2, name_1, name_2, i - begin_2 + begin_1, i);
        }
        printf("\nLines in first file that does " RED "NOT" FORMAT_RESET " exist in second one:\n");
        while (fgets(line_1, sizeof(line_1), fptr_1))
            if (ValidLine(line_1))
            {
                printf(BLUE "%d:\t%s" FORMAT_RESET, end_1 - 1, line_1);
                end_1++;
            }
        is_diff = true;
    }
    else
    {
        for (int i = begin_1; i <= end_1; i++)
        {
            do
                fgets(line_1, sizeof(line_1), fptr_1);
            while (!ValidLine(line_1));
            do
                fgets(line_2, sizeof(line_2), fptr_2);
            while (!ValidLine(line_2));
            if (CompareLine(line_1, line_2, name_1, name_2, i, i - begin_1 + begin_2))
                is_diff = true;
        }
    }
    return is_diff;
}

/*
    1. Gets branchs' last commit ID.
    2. Creats commit pathes and checks if there are conflicts.
    3. Gets username and useremail, prepares new commit ID.
    4. Copies from both commits into the newone.
    5. Updates last commit ID for both branches, writes info.txt ann changes commit ID pointer.

*/
void Merge(char *branch_1, char *branch_2, char *REPOSITORY)
{
    // Getting each branches last commit ID
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char b1_ID[MEDIUM_SIZE], b2_ID[MEDIUM_SIZE];
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), b_f) != NULL)
    {
        strtok(line, "_");
        if (!strcmp(line, branch_1))
        {
            strtok(NULL, "_");
            strcpy(b1_ID, strtok(NULL, "\n\r"));
        }
        else if (!strcmp(line, branch_2))
        {
            strtok(NULL, "_");
            strcpy(b2_ID, strtok(NULL, "\n\r"));
        }
    }
    fclose(b_f);

    // Creating commit pathes
    char b1_p[PATH_MAX], b2_p[PATH_MAX];
    sprintf(b1_p, "%s/.neogit/commits/%s", REPOSITORY, b1_ID);
    sprintf(b2_p, "%s/.neogit/commits/%s", REPOSITORY, b2_ID);

    // Check if there is conflict
    char tmp_1[PATH_MAX];
    strcpy(tmp_1, b1_p);
    if (StashMove(b1_p, tmp_1, b2_p, 2))
    {
        // Getting username and useremail
        char username[MEDIUM_SIZE], useremail[MEDIUM_SIZE];
        char global_p[PATH_MAX], local_p[PATH_MAX];
        sprintf(global_p, "/home/saraghazavi/.neogitconfig/info.txt");
        sprintf(local_p, "%s/.neogit/config.txt", REPOSITORY);

        FILE *config;
        struct stat global_s, local_s;
        stat(global_p, &global_s);
        stat(local_p, &local_s);
        if (global_s.st_mtime > local_s.st_mtime)
            config = fopen(global_p, "r");
        else if (global_s.st_mtime < local_s.st_mtime)
            config = fopen(local_p, "r");

        fgets(username, sizeof(username), config);
        fgets(useremail, sizeof(useremail), config);
        fclose(config);

        strtok(username, "=");
        char *name = strtok(NULL, "\n");
        strtok(useremail, "=");
        char *email = strtok(NULL, "\n");

        // Preparing new commit ID
        int ID = PassCommitID(REPOSITORY);
        CalculateCommitID(REPOSITORY, '+');

        char commit[PATH_MAX];
        sprintf(commit, "%s/.neogit/commits/%d", REPOSITORY, ID);
        DirectoryMaker(commit);

        // Copying from both commits into the newone
        DIR *dir = opendir(b1_p);
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, "info.txt"))
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", b1_p, entry->d_name);
                BashCPMaker(path, commit);
            }
        }
        dir = opendir(b2_p);
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, "info.txt"))
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", b2_p, entry->d_name);
                BashCPMaker(path, commit);
            }
        }

        strcat(commit, "/info.txt");
        FILE *f = fopen(commit, "w");
        commit[strlen(commit) - 9] = '\0';

        // Updating last commit ID for both branches
        IDUpdate(REPOSITORY, branch_1, ID);
        IDUpdate(REPOSITORY, branch_2, ID);

        // Writing info.txt
        char message[MEDIUM_SIZE];
        sprintf(message, "Merge of %s and %s", branch_1, branch_2);
        fprintf(f, "Username =%s\nUseremail =%s\nBranch =%s\nMessage =%s\n", name, email, branch_1, message);
        fclose(f);

        // Success message
        struct stat folder;
        stat(commit, &folder);
        time_t crt = folder.st_ctime;
        printf("Committed " GREEN "successfully" FORMAT_RESET " in time" BLUE " %s" FORMAT_RESET RED "ID = %d" FORMAT_RESET "\nMessage =" YELLOW " \"%s\"\n" FORMAT_RESET, ctime(&crt), ID, message);

        // Changing commit ID pointer
        char IDs_p[PATH_MAX];
        sprintf(IDs_p, "%s/.neogit/current_IDs.txt", REPOSITORY);
        FILE *ID_f = fopen(IDs_p, "r");
        char addID[SMALL_SIZE], commitID[SMALL_SIZE];
        fgets(addID, sizeof(addID), ID_f);
        fgets(commitID, sizeof(commitID), ID_f);
        fclose(ID_f);
        ID_f = fopen(IDs_p, "w");
        fputs(addID, ID_f);
        fputs(commitID, ID_f);
        fprintf(ID_f, "%d\n", ID);
        fclose(ID_f);
    }
    else
    {
        printf("You can't merge branches. " RED
               "CONFLICT ERROR!\n" FORMAT_RESET);
        printf("\tPlease resolve conflicts and " BLUE "try again!\n" FORMAT_RESET);
    }
    return;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// -----------------> Pre commit Functions <-----------------//
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void RunHook(char *REPOSITORY, char *file, bool print)
{
    char hook[PATH_MAX];
    sprintf(hook, "%s/.neogit/applied_hook", REPOSITORY);
    DIR *dir = opendir(hook);
    struct dirent *entry;
    printf("%s:\n", file);
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            if (!strcmp(entry->d_name, "todo-check.txt"))
                TODO(file, print);
            if (!strcmp(entry->d_name, "eof-blank-space.txt"))
                EOFBlankSpace(file, print);
            if (!strcmp(entry->d_name, "balance-braces.txt"))
                BalanceBraces(file, print);
            if (!strcmp(entry->d_name, "format-check.txt"))
                FormatCheck(file, print);
            if (!strcmp(entry->d_name, "file-size-check.txt"))
                FileSize(file, print);
            if (!strcmp(entry->d_name, "character-limit.txt"))
                Character(file, print);
        }
    }
}

void TODO(const char *file, bool print)
{
    if (!strstr(file, ".c") && !strstr(file, ".cpp") && !strstr(file, ".txt"))
    {
        if (print)
        {
            printf("\"todo_check\".................................................................................." YELLOW "SKIPPED\n" FORMAT_RESET);
            return;
        }
    }
    FILE *f = fopen(file, "r");
    bool todo = false;
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        if (!strncmp(line, "//", 2))
            if (strstr(line, "TODO") != NULL)
            {
                todo = true;
                break;
            }
    }
    fclose(f);

    if (todo)
    {
        if (print)
            printf("\"todo_check\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
    else
    {
        if (print)
            printf("\"todo_check\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
}

void EOFBlankSpace(char *file, bool print)
{
    if (!strstr(file, ".c") && !strstr(file, ".cpp") && !strstr(file, ".txt"))
    {
        if (print)
        {
            printf("\"eof-blank-space\".................................................................................." YELLOW "SKIPPED\n" FORMAT_RESET);
            return;
        }
    }
    FILE *f = fopen(file, "r");
    fseek(f, -1, SEEK_END);
    int last = fgetc(f);
    fclose(f);
    if (last == '\n' || last == ' ' || last == '\t')
    {
        if (print)
            printf("\"eof-blank-space\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
    else
    {
        if (print)
            printf("\"eof-blank-space\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
}

void BalanceBraces(char *file, bool print)
{
    if (!strstr(file, ".c") && !strstr(file, ".cpp") && !strstr(file, ".txt"))
    {
        if (print)
        {
            printf("\"balance-braces\".................................................................................." YELLOW "SKIPPED\n" FORMAT_RESET);
            return;
        }
    }
    FILE *f = fopen(file, "r");
    int o_brace = 0, c_brace = 0;
    int o_crosh = 0, c_crosh = 0;
    int o_paran = 0, c_paran = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF)
    {
        if (ch == '{')
            o_crosh++;
        else if (ch == '}')
            c_crosh++;
        else if (ch == '(')
            o_paran++;
        else if (ch == ')')
            c_paran++;
        else if (ch == '[')
            o_brace++;
        else if (ch == ']')
            c_brace++;
    }
    fclose(f);
    if (o_brace == c_brace && o_crosh == c_crosh && o_paran == c_paran)
    {
        if (print)
            printf("\"balance-braces\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
    else
    {
        if (print)
            printf("\"balance-braces\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
}

void FormatCheck(char *file, bool print)
{
    if (!strstr(file, ".txt") && !strstr(file, ".mp4") && !strstr(file, ".c") && !strstr(file, ".cpp") && !strstr(file, ".png") && !strstr(file, ".wav") && !strstr(file, ".mp3"))
    {
        if (print)
            printf("\"format-check\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
    else
    {
        if (print)
            printf("\"format-check\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
}

void FileSize(char *file, bool print)
{
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    if (size < 5 * 1024 * 1024)
    {
        if (print)
            printf("\"file-size-check\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
    else
    {
        if (print)
            printf("\"file-size-check\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
}

void Character(char *file, bool print)
{
    if (!strstr(file, ".c") && !strstr(file, ".cpp") && !strstr(file, ".txt"))
    {
        if (print)
        {
            printf("\"character-limit\".................................................................................." YELLOW "SKIPPED\n" FORMAT_RESET);
            return;
        }
    }
    FILE *f = fopen(file, "r");
    int ctr = 0;
    int c;
    while ((c = fgetc(f)) != EOF)
    {
        ctr++;
    }
    fclose(f);
    if (ctr > 20000)
    {
        if (print)
            printf("\"character-limit\".................................................................................." RED "FAILED\n" FORMAT_RESET);
        CanCommit = false;
        return;
    }
    else
    {
        if (print)
            printf("\"character-limit\".................................................................................." GREEN "PASSED\n" FORMAT_RESET);
        return;
    }
}
