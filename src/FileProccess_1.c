#define _GNU_SOURCE
#include "Header.h"

// Creating repository. Returns 1 if it's done successfully.
int Init()
{
    // Checking repository existence.
    char cur_dir[PATH_MAX];
    getcwd(cur_dir, sizeof(cur_dir));
    if (RepoFinder(cur_dir))
    {
        printf("Repository already " RED "exists!\n" FORMAT_RESET);
        return 0;
    }
    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit");
    mkdir(cur_dir, S_IRWXU | S_IRWXG | S_IRWXO);

    // Copying global config as defult config.
    strcat(cur_dir, "/config.txt");
    if (access("/home/saraghazavi/.neogitconfig/info.txt", F_OK))
    {
        DirectoryMaker("/home/saraghazavi/.neogitconfig");
        FILE *file = fopen("/home/saraghazavi/.neogitconfig/info.txt", "w");
        fputs("name =\nemail =\n", file);
        fclose(file);
    }
    BashCPMaker("/home/saraghazavi/.neogitconfig/info.txt", cur_dir);

    // Creating txt files.
    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit/branch.txt");
    FILE *f = fopen(cur_dir, "w");
    fputs("master_0_0\n", f);
    fclose(f);

    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit/head.txt");
    f = fopen(cur_dir, "w");
    fputs("master\n", f);
    fclose(f);

    getcwd(cur_dir, sizeof(cur_dir));
    strcat(cur_dir, "/.neogit/current_IDs.txt");
    f = fopen(cur_dir, "w");
    fputs("20000\n10000\n0\n", f);
    fclose(f);

    // Creating folders.
    mkdir(".neogit/.stash", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/alias", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/shortcuts", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/commits", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.staged", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/.records", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/tags", S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(".neogit/hooks", S_IRWXU | S_IRWXG | S_IRWXO);

    printf("Repository initialized " GREEN
           "successfully!\n" FORMAT_RESET);
    return 1;
}

// add -n, enters directories and shows S for staged files or U for unstaged files.
void ADDShow(char *REPOSITORY, char *path, int depth, int mainDepth, bool IsStaged)
{
    if (!depth)
        return;
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            // If you achieve folder, go deeper.
            strcat(path, "/");
            strcat(path, entry->d_name);
            printf("\n");
            for (int i = 0; i < mainDepth - depth + 1; i++)
                printf("\t");
            printf("Entering " BOLD YELLOW "%s : \n" FORMAT_RESET, entry->d_name);
            ADDShow(REPOSITORY, path, depth - 1, mainDepth, true);
        }

        else if (entry->d_type != DT_DIR)
        {
            // If you achieve file, check if it exists and not changed between working directory and staging area.
            for (int i = 0; i < mainDepth - depth + 1; i++)
                printf("\t");
            char working[PATH_MAX];
            sprintf(working, "%s/%s", path, entry->d_name);

            char staging[PATH_MAX];
            sprintf(staging, "%s/.neogit/.staged%s/%s", REPOSITORY, path + strlen(REPOSITORY), entry->d_name);
            printf("%s", entry->d_name);

            if (!IsChanged(staging, working))
            {
                printf(RED BOLD "\tU\n" FORMAT_RESET);
                IsStaged = false;
            }
            else
                printf(GREEN BOLD "\tS\n" FORMAT_RESET);
        }
    }
    for (int i = 0; i < mainDepth - depth; i++)
        printf("\t");
    if (IsStaged)
        printf("--> Directory is " BLUE "staged\n\n" FORMAT_RESET);
    else
        printf("--> Directory is " BLUE "not staged\n\n" FORMAT_RESET);
    while (path[strlen(path) - 1] != '/')
        path[strlen(path) - 1] = '\0';
    path[strlen(path) - 1] = '\0';
    closedir(dir);
    return;
}

/*
    First geting file path and checking existence.
    then creating the directory of file if it doesn't exist.
    then copy it from working directory to staging area.
*/
void Stager(char *relative, char *REPOSITORY)
{
    char path[PATH_MAX];
    getcwd(path, sizeof(path));
    strcat(path, "/");
    strcat(path, relative);
    if (access(path, F_OK) != 0)
    {
        printf("File/Directory " RED "not found!\n" FORMAT_RESET);
        return;
    }

    char staging[PATH_MAX];
    sprintf(staging, "%s/.neogit/.staged%s", REPOSITORY, path + strlen(REPOSITORY));

    while (staging[strlen(staging) - 1] != '/')
        staging[strlen(staging) - 1] = '\0';
    staging[strlen(staging) - 1] = '\0';
    DirectoryMaker(staging);

    BashCPMaker(path, staging);
    printf(ITALIC "File/Directory added " GREEN "successfully!\n" NOITALIC FORMAT_RESET);
    return;
}

/*
    First copying staging area to records.
    Then renaming.
*/
void Recorder(char *REPOSITORY)
{
    char staging[PATH_MAX];
    sprintf(staging, "%s/.neogit/.staged", REPOSITORY);
    char records[PATH_MAX];
    sprintf(records, "%s/.neogit/.records", REPOSITORY);
    BashCPMaker(staging, records);

    int ID = PassAddID(REPOSITORY);
    char name[PATH_MAX];
    sprintf(name, "%s/%d", records, ID);
    strcat(records, "/.staged");
    rename(records, name);
    CalculateAddID(REPOSITORY, '+');
}

/*
    Adds undeleted files and folders to staging area.
    Removes deleted files and folders from staging area.
*/
void AddRedo(char *REPOSITORY, char *staging)
{
    DIR *dir = opendir(staging);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            char working[PATH_MAX];
            sprintf(working, "%s%s/%s", REPOSITORY, staging + strlen(REPOSITORY) + 16, entry->d_name);

            char through_staging[PATH_MAX];
            sprintf(through_staging, "%s/%s", staging, entry->d_name);

            // If the folder is not in working directory, it has been deleted.
            if (access(working, F_OK) != 0)
            {
                BashRMMaker(through_staging);
                return;
            }
            strcat(staging, "/");
            strcat(staging, entry->d_name);
            AddRedo(REPOSITORY, staging);
        }
        else if (entry->d_type != DT_DIR)
        {
            char working[PATH_MAX];
            sprintf(working, "%s%s/%s", REPOSITORY, staging + strlen(REPOSITORY) + 16, entry->d_name);

            char through_staging[PATH_MAX];
            sprintf(through_staging, "%s/%s", staging, entry->d_name);

            // If the file is not in working directory, it has been deleted.
            if (access(working, F_OK) != 0)
            {
                remove(through_staging);
                return;
            }
            BashCPMaker(working, through_staging);
        }
    }
    while (staging[strlen(staging) - 1] != '/')
        staging[strlen(staging) - 1] = '\0';
    staging[strlen(staging) - 1] = '\0';
    closedir(dir);
    return;
}

// Removes file or folder from staging area if it's staged.
void Reset(char *relative, char *REPOSITORY)
{
    char path[PATH_MAX];
    getcwd(path, sizeof(path));
    char staging[PATH_MAX];
    sprintf(staging, "%s/.neogit/.staged%s/%s", REPOSITORY, path + strlen(REPOSITORY), relative);
    if (access(staging, F_OK) != 0)
    {
        printf("File/Directory not " RED "staged!\n" FORMAT_RESET);
        return;
    }
    BashRMMaker(staging);
    printf(ITALIC "File unstaged successfully!" NOITALIC
                  "\n");
    return;
}

// Returning the last add command.
void ResetUndo(char *REPOSITORY)
{
    if (PassAddID == 20000)
    {
        printf("You haven't " RED
               "staged" FORMAT_RESET
               " anything yet!\n");
        return;
    }
    CalculateAddID(REPOSITORY, '-');
    int ID = PassAddID(REPOSITORY);

    // Removing the last add command.
    char records[PATH_MAX];
    sprintf(records, "%s/.neogit/.records/%d", REPOSITORY, ID);
    BashRMMaker(records);

    // Clearing staging area.
    char staging[PATH_MAX];
    sprintf(staging, "%s/.neogit/.staged/", REPOSITORY);
    BashRMMaker(staging);

    // Importing previous add to staging area.
    sprintf(records, "%s/.neogit/.records/%d", REPOSITORY, --ID);
    sprintf(staging, "%s/.neogit", REPOSITORY);
    BashCPMaker(staging, records);

    // Renaming to ".staged".
    sprintf(records, "%s/.neogit/%d", REPOSITORY, ID);
    sprintf(staging, "%s/.neogit/.staged", REPOSITORY);
    rename(records, staging);
    printf(ITALIC "Reset undone " GREEN "successfully!\n" NOITALIC FORMAT_RESET);
    return;
}

/*
    First we process the last commit to find deleted and modified files.
    Then we process working directory to find added files.
    Mode changes are being processed in commit checking.
*/
void Status(char *REPOSITORY)
{
    // Getting the ID of commit we are on to check if it's 0 or not.
    char IDs_path[PATH_MAX];
    sprintf(IDs_path, "%s/.neogit/current_IDs.txt", REPOSITORY);
    char ID[SMALL_SIZE];
    FILE *f = fopen(IDs_path, "r");
    fgets(ID, sizeof(ID), f);
    fgets(ID, sizeof(ID), f);
    fgets(ID, sizeof(ID), f);
    if (ID[strlen(ID) - 1] == '\n')
        ID[strlen(ID) - 1] = '\0';
    fclose(f);
    if (!strcmp(ID, "0"))
    {
        printf("You haven't " RED
               "committed" FORMAT_RESET
               " anything yet!\n");
        return;
    }

    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, ID);
    StatusCommit(REPOSITORY, commit);
    char path[PATH_MAX];
    strcpy(path, REPOSITORY);
    StatusWorking(REPOSITORY, path);
}

// For status command.
void StatusCommit(char *REPOSITORY, char *path)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            StatusCommit(REPOSITORY, path);
        }
        else if (entry->d_type != DT_DIR && strcmp(entry->d_name, "info.txt"))
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            char staging[PATH_MAX];
            sprintf(staging, "%s/.neogit/.staged%s", REPOSITORY, path + strlen(REPOSITORY) + 22);
            char working[PATH_MAX];
            sprintf(working, "%s%s", REPOSITORY, path + strlen(REPOSITORY) + 22);

            // If the file is available on working directory.
            if (access(working, F_OK) == 0)
            {
                if (!ModeComparator(working, path))
                    printf(ITALIC "%s\t\t" NOITALIC BOLD GREEN "T" FORMAT_RESET "\n", working + strlen(REPOSITORY));

                // We just want changed files.
                if (!IsChanged(path, working))
                {
                    if (access(staging, F_OK) == 0)
                        printf("%s\t\t" BOLD MAGENTA "+M" FORMAT_RESET "\n", working + strlen(REPOSITORY));
                    else
                        printf("%s\t\t" BOLD MAGENTA "-M" FORMAT_RESET "\n", working + strlen(REPOSITORY));
                }
            }
            // If it has been deleted from working directory.
            else
            {
                if (access(staging, F_OK) == 0)
                    printf("%s\t\t" BOLD RED "+D" FORMAT_RESET "\n", working + strlen(REPOSITORY));
                else
                    printf("%s\t\t" BOLD RED "-D" FORMAT_RESET "\n", working + strlen(REPOSITORY));
            }
            while (path[strlen(path) - 1] != '/')
                path[strlen(path) - 1] = '\0';
            path[strlen(path) - 1] = '\0';
        }
    }
    while (path[strlen(path) - 1] != '/')
        path[strlen(path) - 1] = '\0';
    path[strlen(path) - 1] = '\0';
    closedir(dir);
    return;
}

// For status command.
void StatusWorking(char *REPOSITORY, char *path)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            StatusWorking(REPOSITORY, path);
        }
        else if (entry->d_type != DT_DIR)
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            char staging[PATH_MAX];
            sprintf(staging, "%s/.neogit/.staged%s", REPOSITORY, path + strlen(REPOSITORY));
            char commits[PATH_MAX];
            int ID = PassCommitID(REPOSITORY) - 1;
            sprintf(commits, "%s/.neogit/commits/%d%s", REPOSITORY, ID, path + strlen(REPOSITORY));

            // If the file is not available on last commit (file has been added).
            if (access(commits, F_OK) != 0)
            {
                if (access(staging, F_OK) == 0)
                    printf("%s\t\t" BOLD YELLOW "+A" FORMAT_RESET "\n", path + strlen(REPOSITORY));
                else
                    printf("%s\t\t" BOLD YELLOW "-A" FORMAT_RESET "\n", path + strlen(REPOSITORY));
            }
            while (path[strlen(path) - 1] != '/')
                path[strlen(path) - 1] = '\0';
            path[strlen(path) - 1] = '\0';
        }
    }
    while (path[strlen(path) - 1] != '/')
        path[strlen(path) - 1] = '\0';
    path[strlen(path) - 1] = '\0';
    closedir(dir);
    return;
}

/*
    1. Get head branch from head.txt, then get it's last commit ID from branch.txt.
        Use this commit ID and check the current commit to warn if commit is going to be done on a different BRANCH.
    2. Check staging area to warn if it's empty.
    3. Get the last set username and useremail.
    4. Copy staging area into commits and rename it.
    5. Clear staging area.
    6. Write info.txt for commit.
    7. Update head branch ID.
    8. Copy unchanged files from last commit.
    9. Print success message.
    10. Changing commit ID pointer.
*/
void Commit(char *REPOSITORY, char *message)
{
    if (CanCommit == false)
    {
        printf("You cannot commit beacuse of " RED "pre commit" FORMAT_RESET " rules!\n");
        return;
    }
    // Getting HEAD and it's last commit ID from branch.txt.
    char head[PATH_MAX];
    sprintf(head, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(head, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    fclose(h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    strcat(HEAD, "_");

    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char b_line[BUFF_SIZE];
    char *head_ID;
    while (1)
    {
        fgets(b_line, sizeof(b_line), b_f);
        if (!strncmp(b_line, HEAD, strlen(HEAD)))
        {
            strtok(b_line, "_");
            strtok(NULL, "_");
            head_ID = strtok(NULL, "\n");
            break;
        }
    }
    fclose(b_f);

    char IDs[PATH_MAX];
    sprintf(IDs, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(IDs, "r");
    char ID_line[MEDIUM_SIZE];
    fgets(ID_line, sizeof(ID_line), ID_f);
    fgets(ID_line, sizeof(ID_line), ID_f);
    fgets(ID_line, sizeof(ID_line), ID_f);
    fclose(ID_f);
    if (ID_line[strlen(ID_line) - 1] == '\n')
        ID_line[strlen(ID_line) - 1] = '\0';

    if (strcmp(head_ID, ID_line))
    {
        printf("You can " RED "only" FORMAT_RESET " commit changes on HEAD.\n");
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Checking staging area.
    char staging[PATH_MAX];
    strcpy(staging, REPOSITORY);
    strcat(staging, "/.neogit/.staged");
    int counter = 0;
    DIR *dir = opendir(staging);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            counter++;
    if (counter == 0 || counter == -1)
    {
        printf(RED "Nothing" FORMAT_RESET " has been staged since last commit!\n");
        closedir(dir);
        return;
    }
    closedir(dir);

    ////////////////////////////////////////////////////////////////////////////////////
    // Getting username and useremail.

    char username[MEDIUM_SIZE], useremail[MEDIUM_SIZE], branch[MEDIUM_SIZE];
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
               " config yet! " UNDERLINE "Please set and try again!\n" NOUNDERLINE);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Copy, rename and clear staging area.
    int ID = PassCommitID(REPOSITORY);
    CalculateCommitID(REPOSITORY, '+');

    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    BashCPMaker(staging, commit);
    BashRMMaker(staging);
    DirectoryMaker(staging);

    sprintf(staging, "%s/.neogit/commits/.staged", REPOSITORY);
    sprintf(commit, "%s/.neogit/commits/%d", REPOSITORY, ID);
    rename(staging, commit);

    ////////////////////////////////////////////////////////////////////////////////////
    // info.txt
    sprintf(head, "%s/.neogit/head.txt", REPOSITORY);
    h_f = fopen(head, "r");
    fgets(HEAD, sizeof(HEAD), h_f);
    fclose(h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';

    strcat(commit, "/info.txt");
    FILE *f = fopen(commit, "w");
    commit[strlen(commit) - 9] = '\0';
    fprintf(f, "Username =%s\nUseremail =%s\nBranch =%s\nMessage =%s\n", name, email, HEAD, message);
    fclose(f);

    ////////////////////////////////////////////////////////////////////////////////////
    IDUpdate(REPOSITORY, HEAD, ID);

    ////////////////////////////////////////////////////////////////////////////////////
    // Copy unchanged files.
    if (ID > 10000)
    {
        char destination[PATH_MAX];
        sprintf(destination, "%s/.neogit/commits/%d", REPOSITORY, ID);
        char source[PATH_MAX];
        sprintf(source, "%s/.neogit/commits/%d", REPOSITORY, ID - 1);
        CommitHelp(REPOSITORY, source, destination);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Success message.
    struct stat folder;
    stat(commit, &folder);
    time_t crt = folder.st_ctime;
    printf("Committed " GREEN "successfully" FORMAT_RESET " in time" BLUE " %s" FORMAT_RESET RED "ID = %d" FORMAT_RESET "\nMessage =" YELLOW " \"%s\"\n" FORMAT_RESET, ctime(&crt), ID, message);

    ////////////////////////////////////////////////////////////////////////////////////
    // Changing commit ID pointer
    ID_f = fopen(IDs, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fclose(ID_f);
    ID_f = fopen(IDs, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%d\n", ID);
    fclose(ID_f);
    return;
}

// For commit command.
void CommitHelp(char *REPOSITORY, char *source, char *destination)
{
    DIR *dir = opendir(source);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcat(source, "/");
            strcat(source, entry->d_name);
            strcat(destination, "/");
            strcat(destination, entry->d_name);
            DirectoryMaker(destination);
            CommitHelp(REPOSITORY, source, source);
        }
        else if (entry->d_type != DT_DIR)
        {
            char working[PATH_MAX];
            sprintf(working, "%s%s/%s", REPOSITORY, source + strlen(REPOSITORY) + 22, entry->d_name);

            char commit[PATH_MAX];
            sprintf(commit, "%s/%s", source, entry->d_name);

            // Copy the file if it hasn't been changed.
            if (IsChanged(commit, working))
            {
                strcat(destination, "/");
                strcat(destination, entry->d_name);
                BashCPMaker(commit, destination);
                while (destination[strlen(destination) - 1] != '/')
                    destination[strlen(destination) - 1] = '\0';
                destination[strlen(destination) - 1] = '\0';
            }
        }
    }
    while (source[strlen(source) - 1] != '/')
        source[strlen(source) - 1] = '\0';
    source[strlen(source) - 1] = '\0';
    while (destination[strlen(destination) - 1] != '/')
        destination[strlen(destination) - 1] = '\0';
    destination[strlen(destination) - 1] = '\0';
    closedir(dir);
    return;
}

// Simple log.
void LogSimple(char *REPOSITORY, int n)
{
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    int ID = PassCommitID(REPOSITORY) - 1;
    for (int i = ID; i >= ID - n + 1; i--)
    {
        char on_commit[PATH_MAX];
        sprintf(on_commit, "%s/%d", commit, i);

        char tmp[PATH_MAX];
        strcpy(tmp, on_commit);
        int counter = 0;
        FileCounter(tmp, &counter);
        struct stat folder;
        stat(on_commit, &folder);
        time_t crt = folder.st_ctime;

        strcat(on_commit, "/info.txt");
        FILE *f = fopen(on_commit, "r");
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
        char *branch = strtok(NULL, "=\n");

        char line4[BUFF_SIZE];
        fgets(line4, sizeof(line4), f);
        strtok(line4, "=\n");
        char *message = strtok(NULL, "=\n");
        fclose(f);

        printf("commit " RED "%d\n" FORMAT_RESET, i);
        printf("Author: %s " DIM
               "<%s>" FORMAT_RESET
               "\n",
               name, email);
        printf("Date:\t" BLUE "%s" FORMAT_RESET, ctime(&crt));
        printf("Branch --> " GREEN "%s\n" FORMAT_RESET, branch);
        printf("Number of commited files = " YELLOW "%d\n" FORMAT_RESET, counter);
        printf(BOLD "\t%s\n\n" FORMAT_RESET, message);
    }
}

// Log for branch or author. For branch, checks existence.
void LogCondition(char *REPOSITORY, char *type, char *target)
{
    if (!strcmp(type, "Branch"))
    {
        bool valid = true;
        char branch[PATH_MAX];
        sprintf(branch, "%s/.neogit/branch.txt", REPOSITORY);
        FILE *b_f = fopen(branch, "r");
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), b_f))
        {
            if (!strncmp(line, target, strlen(target)))
                valid = true;
        }
        fclose(b_f);
        if (valid == false)
        {
            printf("Branch " RED
                   "doesn't exist!\n" FORMAT_RESET);
            return;
        }
    }
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    int ID = PassCommitID(REPOSITORY) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char info_p[PATH_MAX];
        sprintf(info_p, "%s/%d/info.txt", commit, i);
        FILE *f = fopen(info_p, "r");
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), f) != NULL)
        {
            if (!strncmp(line, type, strlen(type)))
                if (!strncmp(line + strlen(type) + 2, target, strlen(target)))
                {
                    char on_commit[PATH_MAX];
                    sprintf(on_commit, "%s/%d", commit, i);

                    char tmp[PATH_MAX];
                    strcpy(tmp, on_commit);
                    int counter = 0;
                    FileCounter(tmp, &counter);
                    struct stat folder;
                    stat(on_commit, &folder);
                    time_t crt = folder.st_ctime;

                    FILE *use = fopen(info_p, "r");
                    char line1[BUFF_SIZE];
                    fgets(line1, sizeof(line1), use);
                    strtok(line1, "=\n");
                    char *name = strtok(NULL, "=\n");

                    char line2[BUFF_SIZE];
                    fgets(line2, sizeof(line2), use);
                    strtok(line2, "=\n");
                    char *email = strtok(NULL, "=\n");

                    char line3[BUFF_SIZE];
                    fgets(line3, sizeof(line3), use);
                    strtok(line3, "=\n");
                    char *branch = strtok(NULL, "=\n");

                    char line4[BUFF_SIZE];
                    fgets(line4, sizeof(line4), use);
                    strtok(line4, "=\n");
                    char *message = strtok(NULL, "=\n");

                    fclose(use);

                    printf("commit " RED "%d\n" FORMAT_RESET, i);
                    printf("Author: %s " DIM
                           "<%s>" FORMAT_RESET
                           "\n",
                           name, email);
                    printf("Date:\t" BLUE "%s" FORMAT_RESET, ctime(&crt));
                    printf("Branch --> " GREEN "%s\n" FORMAT_RESET, branch);
                    printf("Number of commited files = " YELLOW "%d\n" FORMAT_RESET, counter);
                    printf(BOLD "\t%s\n\n" FORMAT_RESET, message);
                    break;
                }
        }
        fclose(f);
    }
}

// Log for a certain word in message.
void LogSearch(char *REPOSITORY, char *target)
{
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    int ID = PassCommitID(REPOSITORY) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char info_p[PATH_MAX];
        sprintf(info_p, "%s/%d/info.txt", commit, i);
        FILE *f = fopen(info_p, "r");
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), f) != NULL)
        {
            if (!strncmp(line, "Message", 7))
            {
                char *token = strtok(line, " =\n");
                while (token)
                {
                    if (!strcmp(token, target))
                    {
                        char on_commit[PATH_MAX];
                        sprintf(on_commit, "%s/%d", commit, i);

                        char tmp[PATH_MAX];
                        strcpy(tmp, on_commit);
                        int counter = 0;
                        FileCounter(tmp, &counter);
                        struct stat folder;
                        stat(on_commit, &folder);
                        time_t crt = folder.st_ctime;

                        FILE *use = fopen(info_p, "r");
                        char line1[BUFF_SIZE];
                        fgets(line1, sizeof(line1), use);
                        strtok(line1, "=\n");
                        char *name = strtok(NULL, "=\n");

                        char line2[BUFF_SIZE];
                        fgets(line2, sizeof(line2), use);
                        strtok(line2, "=\n");
                        char *email = strtok(NULL, "=\n");

                        char line3[BUFF_SIZE];
                        fgets(line3, sizeof(line3), use);
                        strtok(line3, "=\n");
                        char *branch = strtok(NULL, "=\n");

                        char line4[BUFF_SIZE];
                        fgets(line4, sizeof(line4), use);
                        strtok(line4, "=\n");
                        char *message = strtok(NULL, "=\n");

                        fclose(use);

                        printf("commit " RED "%d\n" FORMAT_RESET, i);
                        printf("Author: %s " DIM
                               "<%s>" FORMAT_RESET
                               "\n",
                               name, email);
                        printf("Date:\t" BLUE "%s" FORMAT_RESET, ctime(&crt));
                        printf("Branch --> " GREEN "%s\n" FORMAT_RESET, branch);
                        printf("Number of commited files = " YELLOW "%d\n" FORMAT_RESET, counter);
                        printf(BOLD "\t%s\n\n" FORMAT_RESET, message);

                        break;
                    }
                    token = strtok(NULL, " =\n");
                }
            }
        }
        fclose(f);
    }
}

// Log for since or before.
void LogTime(char *REPOSITORY, char *time, char sign)
{
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    int ID = PassCommitID(REPOSITORY) - 1;
    for (int i = ID; i >= 10000; i--)
    {
        char on_commit[PATH_MAX];
        sprintf(on_commit, "%s/%d", commit, i);

        if ((sign == '-' && CompareFileTime(on_commit, time) == 1) || (sign == '+' && CompareFileTime(on_commit, time) == -1))
        {
            char tmp[PATH_MAX];
            strcpy(tmp, on_commit);
            int counter = 0;
            FileCounter(tmp, &counter);
            struct stat folder;
            stat(on_commit, &folder);
            time_t crt = folder.st_ctime;
            strcat(on_commit, "/info.txt");
            FILE *f = fopen(on_commit, "r");
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
            char *branch = strtok(NULL, "=\n");

            char line4[BUFF_SIZE];
            fgets(line4, sizeof(line4), f);
            strtok(line4, "=\n");
            char *message = strtok(NULL, "=\n");
            fclose(f);

            printf("commit " RED "%d\n" FORMAT_RESET, i);
            printf("Author: %s " DIM
                   "<%s>" FORMAT_RESET
                   "\n",
                   name, email);
            printf("Date:\t" BLUE "%s" FORMAT_RESET, ctime(&crt));
            printf("Branch --> " GREEN "%s\n" FORMAT_RESET, branch);
            printf("Number of commited files = " YELLOW "%d\n" FORMAT_RESET, counter);
            printf(BOLD "\t%s\n\n" FORMAT_RESET, message);
        }
    }
}

/*
    1. Gets current commit ID.
    2. Checks working directory for unstaged/untracked changes to warn.
    3. Changes head and gets the last ID of target branch.
    4. Clears working directory.
    5. Importing commit to working directory.
    6. Changing commit ID pointer.
*/
void CheckoutBranch(char *REPOSITORY, char *target)
{
    // Getting current commit ID.
    char IDs[PATH_MAX];
    sprintf(IDs, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(IDs, "r");
    char ID[SMALL_SIZE];
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    if (ID[strlen(ID) - 1] == '\n')
        ID[strlen(ID) - 1] = '\0';
    fclose(ID_f);

    char path[PATH_MAX];
    strcpy(path, REPOSITORY);
    bool IsSame = WDCheckout(REPOSITORY, path, ID);

    // Checking for unrecorded changes.
    if (!IsSame)
    {
        printf("There are " CYAN "Modified/Untracked" FORMAT_RESET " files in working directory. You " RED "CAN'T CHECKOUT" FORMAT_RESET ". " UNDERLINE "Commit or Stash and try again!\n" NOUNDERLINE);
        return;
    }

    // Changing head.
    char h_p[PATH_MAX];
    sprintf(h_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(h_p, "w");
    fprintf(h_f, "%s\n", target);
    fclose(h_f);
    printf("You've checked out on " YELLOW "%s" FORMAT_RESET GREEN "successfully!\n" FORMAT_RESET, target);

    // Getting the last commit ID of branch.
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char b_line[MEDIUM_SIZE];
    while (fgets(b_line, sizeof(b_line), b_f))
    {
        strtok(b_line, "_");
        if (!strcmp(b_line, target))
        {
            strtok(NULL, "_");
            target = strtok(NULL, "\n");
            break;
        }
    }
    fclose(b_f);

    // Cleaning working directory.
    DIR *wd_dir = opendir(REPOSITORY);
    struct dirent *walking;
    while ((walking = readdir(wd_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", REPOSITORY, walking->d_name);
            BashRMMaker(tmp);
        }
    }
    closedir(wd_dir);

    // Importing commit to working directory.
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, target);
    if (commit[strlen(commit) - 1] == '\n')
        commit[strlen(commit) - 1] = '\0';
    DIR *cm_dir = opendir(commit);
    while ((walking = readdir(cm_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", commit, walking->d_name);
            BashCPMaker(tmp, REPOSITORY);
        }
    }
    closedir(cm_dir);

    // Changing commit ID pointer.
    ID_f = fopen(IDs, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fclose(ID_f);
    ID_f = fopen(IDs, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%s\n", target);
    fclose(ID_f);
    return;
}

// Same. Compares target commit branch with head to warn.
void CheckoutCommit(char *REPOSITORY, char *target)
{
    // Getting current commit ID.
    char IDs[PATH_MAX];
    sprintf(IDs, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(IDs, "r");
    char ID[SMALL_SIZE];
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    if (ID[strlen(ID) - 1] == '\n')
        ID[strlen(ID) - 1] = '\0';
    fclose(ID_f);

    char path[PATH_MAX];
    strcpy(path, REPOSITORY);
    bool IsSame = WDCheckout(REPOSITORY, path, ID);

    // Checking for unrecorded changes.
    if (!IsSame)
    {
        printf("There are " CYAN "Modified/Untracked" FORMAT_RESET " files in working directory. You " RED "CAN'T CHECKOUT" FORMAT_RESET ". " UNDERLINE "Commit or Stash and try again!\n" NOUNDERLINE);
        return;
    }

    // Checking commit's branch to be equal to head.
    char info_p[PATH_MAX];
    sprintf(info_p, "%s/.neogit/commits/%s/info.txt", REPOSITORY, target);
    FILE *i_f = fopen(info_p, "r");
    char b_line[MEDIUM_SIZE];
    fgets(b_line, sizeof(b_line), i_f);
    fgets(b_line, sizeof(b_line), i_f);
    fgets(b_line, sizeof(b_line), i_f);
    fclose(i_f);
    strtok(b_line, "=");
    char *cur_b = strtok(NULL, "\n");
    char head_p[PATH_MAX];
    sprintf(head_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(head_p, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    if (strcmp(HEAD, cur_b))
    {
        printf("You should be on the " CYAN "branch of this commit ID" FORMAT_RESET
               " to checkout!\n");
        return;
    }
    printf("You've checked out on " YELLOW "%s" FORMAT_RESET GREEN "successfully!\n" FORMAT_RESET, target);

    // Getting the last commit ID of branch.
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char line[MEDIUM_SIZE];
    while (fgets(line, sizeof(line), b_f))
    {
        strtok(line, "_");
        if (!strcmp(line, target))
        {
            strtok(NULL, "_");
            target = strtok(NULL, "\n");
            break;
        }
    }
    fclose(b_f);

    // Cleaning working directory.
    DIR *wd_dir = opendir(REPOSITORY);
    struct dirent *walking;
    while ((walking = readdir(wd_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", REPOSITORY, walking->d_name);
            BashRMMaker(tmp);
        }
    }
    closedir(wd_dir);

    // Importing commit to working directory.
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, target);
    if (commit[strlen(commit) - 1] == '\n')
        commit[strlen(commit) - 1] = '\0';
    DIR *cm_dir = opendir(commit);
    while ((walking = readdir(cm_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", commit, walking->d_name);
            BashCPMaker(tmp, REPOSITORY);
        }
    }
    closedir(cm_dir);

    // Changing commit ID pointer.
    ID_f = fopen(IDs, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fclose(ID_f);
    ID_f = fopen(IDs, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%s\n", target);
    fclose(ID_f);
    return;
}

// Same. Finds last ID of head to checkout.
void CheckoutHead(char *REPOSITORY)
{
    // Getting head name.
    char h_p[PATH_MAX];
    sprintf(h_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(h_p, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    fclose(h_f);

    // Getting current commit ID.
    char ID_p[PATH_MAX];
    sprintf(ID_p, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(ID_p, "r");
    char ID[SMALL_SIZE];
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    if (ID[strlen(ID) - 1] == '\n')
        ID[strlen(ID) - 1] = '\0';
    fclose(ID_f);
    char path[PATH_MAX];
    strcpy(path, REPOSITORY);
    bool IsSame = WDCheckout(REPOSITORY, path, ID);

    // Checking for unrecorded changes.
    if (!IsSame)
    {
        printf("There are " CYAN "Modified/Untracked" FORMAT_RESET " files in working directory. You " RED "CAN'T CHECKOUT" FORMAT_RESET ". " UNDERLINE "Commit or Stash and try again!\n" NOUNDERLINE);
        return;
    }

    // Getting the last commit ID of head.
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char line[MEDIUM_SIZE];
    while (fgets(line, sizeof(line), b_f))
    {
        strtok(line, "_");
        if (!strcmp(line, HEAD))
        {
            strtok(NULL, "_");
            strcpy(ID, strtok(NULL, "\n"));
            break;
        }
    }
    fclose(b_f);
    printf("You've checked out on " YELLOW "HEAD" FORMAT_RESET GREEN "successfully!\n" FORMAT_RESET);

    // Cleaning working directory.
    DIR *wd_dir = opendir(REPOSITORY);
    struct dirent *walking;
    while ((walking = readdir(wd_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", REPOSITORY, walking->d_name);
            BashRMMaker(tmp);
        }
    }
    closedir(wd_dir);

    // Importing commit to working directory.
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, ID);
    DIR *cm_dir = opendir(commit);
    while ((walking = readdir(cm_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", commit, walking->d_name);
            BashCPMaker(tmp, REPOSITORY);
        }
    }
    closedir(cm_dir);

    // Changing commit ID pointer.
    ID_f = fopen(ID_p, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fclose(ID_f);
    ID_f = fopen(ID_p, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%s\n", ID);
    fclose(ID_f);
    return;
}

// Same. Finds last n times ID of head to checkout.
void CheckoutHeadN(char *REPOSITORY, char *target)
{
    // Getting head name.
    char h_p[PATH_MAX];
    sprintf(h_p, "%s/.neogit/head.txt", REPOSITORY);
    FILE *h_f = fopen(h_p, "r");
    char HEAD[MEDIUM_SIZE];
    fgets(HEAD, sizeof(HEAD), h_f);
    if (HEAD[strlen(HEAD) - 1] == '\n')
        HEAD[strlen(HEAD) - 1] = '\0';
    fclose(h_f);

    // Getting current commit ID.
    char ID_p[PATH_MAX];
    sprintf(ID_p, "%s/.neogit/current_IDs.txt", REPOSITORY);
    FILE *ID_f = fopen(ID_p, "r");
    char ID[SMALL_SIZE];
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    fgets(ID, sizeof(ID), ID_f);
    if (ID[strlen(ID) - 1] == '\n')
        ID[strlen(ID) - 1] = '\0';
    fclose(ID_f);
    char path[PATH_MAX];
    strcpy(path, REPOSITORY);
    bool IsSame = WDCheckout(REPOSITORY, path, ID);

    // Checking for unrecorded changes.
    if (!IsSame)
    {
        printf("There are " CYAN "Modified/Untracked" FORMAT_RESET " files in working directory. You " RED "CAN'T CHECKOUT" FORMAT_RESET ". " UNDERLINE "Commit or Stash and try again!\n" NOUNDERLINE);
        return;
    }

    // Getting the last commit ID of head.
    char b_p[PATH_MAX];
    sprintf(b_p, "%s/.neogit/branch.txt", REPOSITORY);
    FILE *b_f = fopen(b_p, "r");
    char line[MEDIUM_SIZE];
    while (fgets(line, sizeof(line), b_f))
    {
        strtok(line, "_");
        if (!strcmp(line, HEAD))
        {
            strtok(NULL, "_");
            strcpy(ID, strtok(NULL, "\n"));
            break;
        }
    }
    fclose(b_f);

    // Finding out which commit ID to checkout.
    int back = 0;
    for (int i = 5; i < strlen(target); i++)
    {
        back *= 10;
        back += target[i] - '0';
    }
    printf("You've checked out on " YELLOW "%d" FORMAT_RESET "commits before HEAD " GREEN "successfully!\n" FORMAT_RESET, back);
    char commit[PATH_MAX];
    sprintf(commit, "%s/.neogit/commits", REPOSITORY);
    int comID = PassCommitID(REPOSITORY) - 1, i;
    for (i = comID; i >= 10000; i--)
    {
        char info[PATH_MAX];
        sprintf(info, "%s/%d/info.txt", commit, i);
        FILE *i_p = fopen(info, "r");
        char line[BUFF_SIZE];
        fgets(line, sizeof(line), i_p);
        fgets(line, sizeof(line), i_p);
        fgets(line, sizeof(line), i_p);
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        if (!strcmp(line + 8, HEAD))
            back--;
        fclose(i_p);
        if (back == -1)
            break;
    }
    sprintf(ID, "%d", i);

    // Cleaning working directory.
    DIR *wd_dir = opendir(REPOSITORY);
    struct dirent *walking;
    while ((walking = readdir(wd_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, ".neogit"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", REPOSITORY, walking->d_name);
            BashRMMaker(tmp);
        }
    }
    closedir(wd_dir);

    // Importing commit to working directory.
    sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, ID);
    DIR *cm_dir = opendir(commit);
    while ((walking = readdir(cm_dir)) != NULL)
    {
        if (strcmp(walking->d_name, ".") && strcmp(walking->d_name, "..") && strcmp(walking->d_name, "info.txt"))
        {
            char tmp[PATH_MAX];
            sprintf(tmp, "%s/%s", commit, walking->d_name);
            BashCPMaker(tmp, REPOSITORY);
        }
    }
    closedir(cm_dir);

    // Changing commit ID pointer.
    ID_f = fopen(ID_p, "r");
    char addID[SMALL_SIZE], commitID[SMALL_SIZE];
    fgets(addID, sizeof(addID), ID_f);
    fgets(commitID, sizeof(commitID), ID_f);
    fclose(ID_f);
    ID_f = fopen(ID_p, "w");
    fputs(addID, ID_f);
    fputs(commitID, ID_f);
    fprintf(ID_f, "%s\n", ID);
    fclose(ID_f);
    return;
}

/*
    Walks through working directory and a commit ID.
    Returns 1 if there is a difference.
*/
int WDCheckout(char *REPOSITORY, char *path, char *ID)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".neogit"))
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            if (WDCheckout(REPOSITORY, path, ID) == 0)
                return 0;
        }
        else if (entry->d_type != DT_DIR)
        {
            strcat(path, "/");
            strcat(path, entry->d_name);
            char commit[PATH_MAX];
            sprintf(commit, "%s/.neogit/commits/%s%s", REPOSITORY, ID, path + strlen(REPOSITORY));

            // If file exists in target commit.
            if (access(commit, F_OK) == 0)
            {
                if (!IsChanged(commit, path))
                {
                    closedir(dir);
                    return 0;
                }
            }
            // If it doesn't exist.
            else
            {
                closedir(dir);
                return 0;
            }
            while (path[strlen(path) - 1] != '/')
                path[strlen(path) - 1] = '\0';
            path[strlen(path) - 1] = '\0';
        }
    }
    while (path[strlen(path) - 1] != '/')
        path[strlen(path) - 1] = '\0';
    path[strlen(path) - 1] = '\0';
    closedir(dir);
    return 1;
}
