#include "Header.h"
bool CanCommit = true;
// #define DEBUG
#ifdef DEBUG

int main()
{
    int argc = 4;
    char *argv[] = {"neogit", "pre-commit", "hooks", "list"};
    chdir("/home/saraghazavi/project");
#else
int main(int argc, char *argv[])
{
#endif
    Colors();
    if (argc < 2)
    {
        INVALID_CMD;
        return 0;
    }

    if (!strcmp(argv[1], "init"))
    {
        if (argc == 2)
        {
            if (Init())
                return 0;
        }
        else
        {
            INVALID_CMD;
            return 0;
        }
    }

    if (argc == 5 && !strcmp(argv[1], "config") && !strcmp(argv[2], "-global") && !strncmp(argv[3], "alias.", 6))
    {
        char *home = getenv("HOME");
        char alias[PATH_MAX];
        sprintf(alias, "%s/.neogitconfig/alias", home);
        DirectoryMaker(alias);

        sprintf(alias, "%s/.neogitconfig/alias/%s.txt", home, argv[3] + 6);

        if (IsCommand(argv[4]))
        {
            FILE *fp = fopen(alias, "w");
            fprintf(fp, "%s", argv[4]);
            fclose(fp);
            printf("Alias created " GREEN
                   "successfully!\n" FORMAT_RESET);
        }
        else
            printf("Not a " RED "valid" FORMAT_RESET " command to create alias!\n");
        return 0;
    }
    else if (argc == 5 && !strcmp(argv[1], "config") && !strcmp(argv[2], "-global"))
    {
        char *home = getenv("HOME");
        char config[PATH_MAX];
        sprintf(config, "%s/.neogitconfig", home);
        DirectoryMaker(config);

        sprintf(config, "%s/.neogitconfig/info.txt", home);
        if (access(config, F_OK) != 0)
        {
            FILE *fp = fopen(config, "w");
            fprintf(fp, "name =\nemail =\n");
            fclose(fp);
        }

        if (!strcmp(argv[3], "user.name"))
            ChangeName(config, argv[4]);
        else if (!strcmp(argv[3], "user.email"))
            ChangeEmail(config, argv[4]);
        else
            INVALID_CMD;
        return 0;
    }

    char REPOSITORY[PATH_MAX];
    getcwd(REPOSITORY, sizeof(REPOSITORY));
    CHECK_IF_REPO(RepoFinder(REPOSITORY));

    if (CheckAlias(argv[1], REPOSITORY))
        return 0;

    if (argc == 4 && !strcmp(argv[1], "config"))
    {
        if (!strncmp(argv[2], "alias.", 6))
        {
            strcat(REPOSITORY, "/.neogit/alias/");
            strcat(REPOSITORY, argv[2] + 6);
            strcat(REPOSITORY, ".txt");

            if (IsCommand(argv[3]))
            {
                FILE *fp = fopen(REPOSITORY, "w");
                fprintf(fp, "%s", argv[3]);
                fclose(fp);
                printf("Alias created " GREEN
                       "successfully!\n" FORMAT_RESET);
            }
            else
                printf("Not a " RED "valid" FORMAT_RESET " command to create alias!\n");
        }
        else
        {
            strcat(REPOSITORY, "/.neogit/config.txt");
            if (access(REPOSITORY, F_OK) != 0)
            {
                FILE *fp = fopen(REPOSITORY, "w");
                fprintf(fp, "name =\nemail =\n");
                fclose(fp);
            }
            if (!strcmp(argv[2], "user.name"))
                ChangeName(REPOSITORY, argv[3]);
            else if (!strcmp(argv[2], "user.email"))
                ChangeEmail(REPOSITORY, argv[3]);
            else
                INVALID_CMD;
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "add"))
    {
        if (argc > 3 && !strcmp(argv[2], "-f"))
        {
            for (int i = 3; i < argc; i++)
                Stager(argv[i], REPOSITORY);
            Recorder(REPOSITORY);
        }

        else if (argc > 3 && !strcmp(argv[2], "-n"))
        {
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++)
            {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            char path[PATH_MAX];
            getcwd(path, sizeof(path));
            char *slash = strrchr(path, '/');
            printf("Searching " BOLD YELLOW "%s :\n\n" FORMAT_RESET, slash + 1);
            ADDShow(REPOSITORY, path, depth, depth, true);
        }

        else if (argc == 3 && !strcmp(argv[2], "-redo"))
        {
            char staging[PATH_MAX];
            sprintf(staging, "%s/.neogit/.staged", REPOSITORY);
            AddRedo(REPOSITORY, staging);
            Recorder(REPOSITORY);
        }

        else
        {
            for (int i = 2; i < argc; i++)
                Stager(argv[i], REPOSITORY);
            Recorder(REPOSITORY);
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "reset"))
    {
        if (argc > 3 && !strcmp(argv[2], "-f"))
        {
            for (int i = 3; i < argc; i++)
                Reset(argv[i], REPOSITORY);
            Recorder(REPOSITORY);
        }

        else if (argc == 3 && !strcmp(argv[2], "-undo"))
            ResetUndo(REPOSITORY);

        else
        {
            for (int i = 2; i < argc; i++)
                Reset(argv[i], REPOSITORY);
            Recorder(REPOSITORY);
        }
    }

    else if (argc > 2 && !strcmp(argv[1], "commit"))
    {
        if (!strcmp(argv[2], "-m"))
        {
            if (argc > 4)
                printf("You message should be in " RED "quotations!\n" FORMAT_RESET);
            else if (argc < 4)
                printf("You commit should have a " RED
                       "message!\n" FORMAT_RESET);
            else if (strlen(argv[3]) > 72)
                printf("Your message can have a " RED "maximum" FORMAT_RESET " of 72 words!\n");
            else
                Commit(REPOSITORY, argv[3]);
        }

        else if (argc == 4 && !strcmp(argv[2], "-s"))
        {
            char path[PATH_MAX];
            sprintf(path, "%s/.neogit/shortcuts/%s.txt", REPOSITORY, argv[3]);
            FILE *f = fopen(path, "r");
            if (f == NULL)
                printf("Shortcut doesn't exist!\n");
            else
            {
                char message[MEDIUM_SIZE];
                fgets(message, sizeof(message), f);
                Commit(REPOSITORY, message);
                fclose(f);
            }
        }

        else
            INVALID_CMD;
    }

    else if (argc == 6 && !strcmp(argv[1], "set") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s"))
    {
        char sh_path[PATH_MAX];
        sprintf(sh_path, "%s/.neogit/shortcuts/%s.txt", REPOSITORY, argv[5]);
        FILE *sh_f = fopen(sh_path, "w");
        fputs(argv[3], sh_f);
        fclose(sh_f);
        printf("Shortcut message set " GREEN "successfully!\n" FORMAT_RESET);
    }

    else if (argc == 6 && !strcmp(argv[1], "replace") && !strcmp(argv[2], "-m") && !strcmp(argv[4], "-s"))
    {
        char sh_path[PATH_MAX];
        sprintf(sh_path, "%s/.neogit/shortcuts/%s.txt", REPOSITORY, argv[5]);
        if (access(sh_path, F_OK) != 0)
            printf("Shortcut " RED "doesn't exist!\n" FORMAT_RESET);
        else
        {
            FILE *sh_f = fopen(sh_path, "w");
            fputs(argv[3], sh_f);
            fclose(sh_f);
            printf("Shortcut message reset " GREEN "successfully!\n" FORMAT_RESET);
        }
    }

    else if (argc == 4 && !strcmp(argv[1], "remove") && !strcmp(argv[2], "-s"))
    {
        char sh_path[PATH_MAX];
        sprintf(sh_path, "%s/.neogit/shortcuts/%s.txt", REPOSITORY, argv[3]);
        if (access(sh_path, F_OK) != 0)
            printf("Shortcut doesn't exist!\n");
        else
        {
            remove(sh_path);
            printf("Shortcut message removed successfully!\n");
        }
    }

    else if (argc > 1 && !strcmp(argv[1], "log"))
    {
        if (argc == 2)
            LogSimple(REPOSITORY, PassCommitID(REPOSITORY) - 10000);

        else if (argc == 4 && !strcmp(argv[2], "-n"))
        {
            int depth = 0;
            for (int i = 0; i < strlen(argv[3]); i++)
            {
                depth *= 10;
                depth += (argv[3][i] - '0');
            }
            if (PassCommitID(REPOSITORY) - 9999 < depth)
                LogSimple(REPOSITORY, PassCommitID(REPOSITORY) - 10000);
            else
                LogSimple(REPOSITORY, depth);
        }

        else if (argc == 4 && !strcmp(argv[2], "-branch"))
            LogCondition(REPOSITORY, "Branch", argv[3]);

        else if (argc == 4 && !strcmp(argv[2], "-author"))
            LogCondition(REPOSITORY, "Username", argv[3]);

        else if (argc == 4 && !strcmp(argv[2], "-since"))
            LogTime(REPOSITORY, argv[3], '+');

        else if (argc == 4 && !strcmp(argv[2], "-before"))
            LogTime(REPOSITORY, argv[3], '-');

        else if (!strcmp(argv[2], "-search"))
            for (int i = 3; i < argc; i++)
                LogSearch(REPOSITORY, argv[i]);

        else
            INVALID_CMD;
    }

    else if (argc == 2 && !strcmp(argv[1], "status"))
        Status(REPOSITORY);

    else if (!strcmp(argv[1], "branch"))
    {
        if (argc == 2)
            BranchList(REPOSITORY);
        else if (argc == 3)
            AddBranch(argv[2], REPOSITORY);
        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "checkout"))
    {
        if (argc != 3)
            INVALID_CMD;
        else if (!strcmp(argv[2], "HEAD"))
            CheckoutHead(REPOSITORY);
        else if (!strncmp(argv[2], "HEAD-", 5))
            CheckoutHeadN(REPOSITORY, argv[2]);
        else
        {
            if (argv[2][0] <= '9' && argv[2][0] >= '0')
                CheckoutCommit(argv[2], REPOSITORY);
            else
                CheckoutBranch(argv[2], REPOSITORY);
        }
    }

    else if (!strcmp(argv[1], "tag"))
    {
        if (argc == 2)
            ListTags(REPOSITORY);

        else if (!strcmp(argv[2], "-a"))
        {
            /*
                Two forms of code can be done for -f.
                1. It's a permission to change an existing tag.
                2. It's a flag to be set on tag for future changes.
            */
            if (argc == 4)
                AddTag(argv[3], NULL, NULL, 0, REPOSITORY);

            else if (argc == 5 && !strcmp(argv[4], "-f"))
                AddTag(argv[3], NULL, NULL, 1, REPOSITORY);

            else if (argc == 6)
            {
                if (!strcmp(argv[4], "-m"))
                    AddTag(argv[3], argv[5], NULL, 0, REPOSITORY);
                else if (!strcmp(argv[4], "-c"))
                    AddTag(argv[3], NULL, argv[5], 0, REPOSITORY);
                else
                    INVALID_CMD;
            }

            else if (argc == 7)
            {
                if (!strcmp(argv[4], "-m") && !strcmp(argv[6], "-f"))
                    AddTag(argv[3], argv[5], NULL, 1, REPOSITORY);
                else if (!strcmp(argv[4], "-c") && !strcmp(argv[6], "-f"))
                    AddTag(argv[3], NULL, argv[5], 1, REPOSITORY);
                else
                    INVALID_CMD;
            }

            else if (argc == 8 && !strcmp(argv[4], "-m") && !strcmp(argv[6], "-c"))
                AddTag(argv[3], argv[5], argv[7], 0, REPOSITORY);

            else if (argc == 9 && !strcmp(argv[4], "-m") && !strcmp(argv[6], "-c") && !strcmp(argv[8], "-f"))
                AddTag(argv[3], argv[5], argv[7], 1, REPOSITORY);

            else
                INVALID_CMD;
        }

        else if (!strcmp(argv[2], "show"))
        {
            if (argc != 4)
                INVALID_CMD;
            else
                ShowTag(argv[3], REPOSITORY);
        }

        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "stash"))
    {
        if (argc < 3)
            INVALID_CMD;
        else if (!strcmp(argv[2], "push"))
        {
            if (argc == 5)
                StashPush(true, argv[4], REPOSITORY);
            else
                StashPush(false, NULL, REPOSITORY);
        }

        else if (!strcmp(argv[2], "list"))
        {
            if (argc == 3)
                StashList(REPOSITORY);
            else
                INVALID_CMD;
        }

        else if (!strcmp(argv[2], "show"))
        {
            if (argc != 4)
                INVALID_CMD;
            else
                StashShow(argv[3], REPOSITORY);
        }
        else if (!strcmp(argv[2], "pop"))
        {
            if (argc == 4)
                StashPop(argv[3], REPOSITORY);
            else if (argc == 3)
                StashPop("0", REPOSITORY);
            else
                INVALID_CMD;
        }
        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "pre-commit"))
    {
        if (argc == 2)
        {
            char staging[PATH_MAX];
            sprintf(staging, "%s/.neogit/.staged", REPOSITORY);
            DIR *dir = opendir(staging);
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type != DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                {
                    char path[PATH_MAX];
                    sprintf(path, "%s/.neogit/.staged/%s", REPOSITORY, entry->d_name);
                    RunHook(REPOSITORY, path, true);
                }
            }
            closedir(dir);
        }
        else if (!strcmp(argv[2], "hooks") && !strcmp(argv[3], "list"))
        {
            printf(YELLOW BOLD "List of available hooks:\n" FORMAT_RESET);
            printf("todo-check (for .c and .cpp files)\n");
            printf(DIM "\tIn .c and .cpp files there shouldn't be TODO comments.\n" FORMAT_RESET);
            printf("todo_check (for .txt files)\n");
            printf(DIM "\tIn .txt files there shouldn't be TODO word.\n" FORMAT_RESET);
            printf("eof_blank_space\n");
            printf(DIM "\tIn .txt files there shouldn't be whitespaces at the end of file.\n" FORMAT_RESET);
            printf("format_check\n");
            printf(DIM "\tFile Formats should be available\n" FORMAT_RESET);
            printf("balance_braces\n");
            printf(DIM "\tThere should be a closing for (,{,[\n" FORMAT_RESET);
            printf("file_size_check\n");
            printf(DIM "\tFile size shouldn't be larger than 5 MB\n" FORMAT_RESET);
            printf("character-limit\n");
            printf(DIM "\tFile characters number shouldn't be larger than 200000\n" FORMAT_RESET);
        }

        else if (!strcmp(argv[2], "applied") && !strcmp(argv[3], "hooks"))
        {
            printf(YELLOW BOLD "List of applied hooks:\n" FORMAT_RESET);
            char path[PATH_MAX];
            sprintf(path, "%s/.neogit/hooks", REPOSITORY);
            DIR *dir = opendir(path);
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                    printf("\t%s\n", entry->d_name);
            }
            closedir(dir);
        }

        else if (!strcmp(argv[2], "add") && !strcmp(argv[3], "hook"))
        {
            char path[PATH_MAX];
            sprintf(path, "%s/.neogit/hooks/%s.txt", REPOSITORY, argv[4]);
            FILE *f = fopen(path, "w");
            fprintf(f, "exist");
            fclose(f);
        }

        else if (!strcmp(argv[2], "remove") && !strcmp(argv[3], "hook"))
        {
            char path[PATH_MAX];
            sprintf(path, "%s/.neogit/applied_hook/%s.txt", REPOSITORY, argv[4]);
            remove(path);
        }

        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "grep") && !strcmp(argv[2], "-f") && !strcmp(argv[4], "-p"))
    {
        if (argc == 6)
        {
            char current[PATH_MAX];
            getcwd(current, sizeof(current));
            char file[PATH_MAX];
            sprintf(file, "%s/%s", current, argv[3]);
            Grep(file, argv[5], false);
        }

        else if (argc == 7)
        {
            char current[PATH_MAX];
            getcwd(current, sizeof(current));
            char file[PATH_MAX];
            sprintf(file, "%s/%s", current, argv[3]);
            Grep(file, argv[5], true);
        }

        else if (argc == 8)
        {
            char commit[PATH_MAX];
            sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, argv[7]);
            GrepCommit(commit, false, argv[5], argv[3]);
        }

        else if (argc == 9)
        {
            char commit[PATH_MAX];
            sprintf(commit, "%s/.neogit/commits/%s", REPOSITORY, argv[7]);
            GrepCommit(commit, true, argv[5], argv[3]);
        }

        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "diff"))
    {
        if (!strcmp(argv[2], "-f"))
        {
            char current[PATH_MAX];
            getcwd(current, sizeof(current));
            char file1[PATH_MAX];
            sprintf(file1, "%s/%s", current, argv[3]);
            char file2[PATH_MAX];
            sprintf(file2, "%s/%s", current, argv[4]);
            if (argc == 5)
                Diff(file1, file2, 1, 10000, 1, 10000);

            else if (argc == 7)
            {
                int begin = 0;
                char *tok = strtok(argv[6], "-");
                for (int i = 0; i < strlen(tok); i++)
                {
                    begin *= 10;
                    begin += (tok[i] - '0');
                }
                int end = 0;
                tok = strtok(NULL, "-");
                for (int i = 0; i < strlen(tok); i++)
                {
                    end *= 10;
                    end += (tok[i] - '0');
                }

                if (!strcmp(argv[5], "-line1"))
                    Diff(file1, file2, begin, end, 1, 10000);
                else if (!strcmp(argv[5], "-line2"))
                    Diff(file1, file2, 1, 10000, begin, end);
                else
                    INVALID_CMD;
            }

            else if (argc == 9)
            {
                if (!strcmp(argv[5], "-line1") && !strcmp(argv[7], "-line2"))
                {
                    int begin_1 = 0;
                    char *token = strtok(argv[6], "-");
                    for (int i = 0; i < strlen(token); i++)
                    {
                        begin_1 *= 10;
                        begin_1 += (token[i] - '0');
                    }
                    int end_1 = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++)
                    {
                        end_1 *= 10;
                        end_1 += (token[i] - '0');
                    }

                    int begin_2 = 0;
                    token = strtok(argv[8], "-");
                    for (int i = 0; i < strlen(token); i++)
                    {
                        begin_2 *= 10;
                        begin_2 += (token[i] - '0');
                    }
                    int end_2 = 0;
                    token = strtok(NULL, "-");
                    for (int i = 0; i < strlen(token); i++)
                    {
                        end_2 *= 10;
                        end_2 += (token[i] - '0');
                    }
                    Diff(file1, file2, begin_1, end_1, begin_2, end_2);
                }
                else
                    INVALID_CMD;
            }
            else
                INVALID_CMD;
        }

        else if (!strcmp(argv[2], "-c"))
        {
            if (argc == 5)
            {
                char commit_1[PATH_MAX], commit_2[PATH_MAX];
                sprintf(commit_1, "%s/.neogit/commits/%s", REPOSITORY, argv[3]);
                sprintf(commit_2, "%s/.neogit/commits/%s", REPOSITORY, argv[4]);
                if (access(commit_1, F_OK) != 0 || access(commit_2, F_OK) != 0)
                    printf("Invalid commit ID!\n");
                printf(BOLD "Files in commit %s that are not in commit %s :\n\n" FORMAT_RESET, argv[3], argv[4]);
                DiffOne(commit_1, commit_2);

                printf(BOLD "Files in commit %s that are not in commit %s :\n\n" FORMAT_RESET, argv[4], argv[3]);
                sprintf(commit_1, "%s/.neogit/commits/%s", REPOSITORY, argv[3]);
                sprintf(commit_2, "%s/.neogit/commits/%s", REPOSITORY, argv[4]);
                DiffOne(commit_2, commit_1);

                printf(BOLD "Comparing in commen files :\n\n" FORMAT_RESET);
                sprintf(commit_1, "%s/.neogit/commits/%s", REPOSITORY, argv[3]);
                sprintf(commit_2, "%s/.neogit/commits/%s", REPOSITORY, argv[4]);
                DiffTwo(commit_1, commit_2);
            }
            else
                INVALID_CMD;
        }
        else
            INVALID_CMD;
    }

    else if (!strcmp(argv[1], "merge"))
    {
        if (argc < 3)
            INVALID_CMD;
        else if (!strcmp(argv[2], "-b"))
        {
            if (argc != 5)
                INVALID_CMD;
            else
                Merge(argv[3], argv[4], REPOSITORY);
        }
    }

    else
        INVALID_CMD;
    return 0;
}