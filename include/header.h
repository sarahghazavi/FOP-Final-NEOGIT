#ifndef Header
#define Header

#define _GNU_SOURCE
#include "AnsiColors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h> //
#include <sys/types.h>
extern bool CanCommit;
#define BUFF_SIZE 1000
#define MEDIUM_SIZE 100
#define SMALL_SIZE 10

#define INVALID_CMD printf("Invalid Command!\n")
#define CHECK_IF_REPO(x)                                                       \
    if (!x)                                                                    \
    {                                                                          \
        printf("Not a neogit repository or any of the parent directories!\n"); \
        return 0;                                                              \
    }

// -----------------> File Proccessing Functions <-----------------
int Init();
void ADDShow(char *REPOSITORY, char *path, int depth, int mainDepth, bool IsStaged);
void Stager(char *relativeP, char *REPOSITORY);
void Recorder(char *REPOSITORY);
void AddRedo(char *REPOSITORY, char *staging);
void Reset(char *relative, char *REPOSITORY);
void ResetUndo(char *REPOSITORY);
void Status(char *REPOSITORY);
void StatusCommit(char *REPOSITORY, char *path);
void StatusWorking(char *REPOSITORY, char *path);
void Commit(char *REPOSITORY, char *message);
void CommitHelp(char *REPOSITORY, char *source, char *destination);
void LogSimple(char *REPOSITORY, int n);
void LogCondition(char *REPOSITORY, char *type, char *target);
void LogSearch(char *REPOSITORY, char *target);
void LogTime(char *REPOSITORY, char *time, char sign);
void CheckoutBranch(char *REPOSITORY, char *target);
void CheckoutCommit(char *REPOSITORY, char *target);
void CheckoutHead(char *REPOSITORY);
void CheckoutHeadN(char *REPOSITORY, char *target);
int WDCheckout(char *REPOSITORY, char *path, char *ID);

// -----------------> Handy Functions <-----------------
bool IsCommand(char *command);
void DirectoryMaker(char *path);
void BashCPMaker(char *source, char *destination);
void BashRMMaker(char *path);
mode_t ModeComparator(char *file1, char *file2);
int RepoFinder(char *path);
void ChangeName(char *info, char *name);   // check this
void ChangeEmail(char *info, char *email); // check this
int IsChanged(char *file1, char *file2);
int PassAddID(char *REPOSITORY);
int PassCommitID(char *REPOSITORY);
void CalculateAddID(char *REPOSITORY, char sign);
void CalculateCommitID(char *REPOSITORY, char sign);
int CompareFileTime(char *file, char *time);
void FileCounter(char *path, int *counter);
void IDUpdate(char *REPOSITORY, char *branch, int ID);
void AddBranch(char *name, char *REPOSITORY);
void BranchList(char *REPOSITORY);
int CheckAlias(char *entrance, char *REPOSITORY);
int Sorter(const struct dirent **a, const struct dirent **b);
int ValidLine(char *line);
bool Wildcard(char *wildcard, char *word);
int CompareLine(char *line_1, char *line_2, char *file_1, char *file_2, int which_1, int which_2);

// -----------------> Phase 2 Functions <-----------------
void AddTag(char *name, char *message, char *ID, int can_change, char *REPOSITORY);
void ListTags(char *REPOSITORY);
void ShowTag(char *name, char *REPOSITORY);
void StashPush(bool check, char *message, char *REPOSITORY);
void StashList(char *REPOSITORY);
void StashShow(char *idx, char *REPOSITORY);
int StashMove(char *main, char *go_in, char *compare, int mode);
void StashPop(char *index, char *REPOSITORY);
void Grep(char *path, char *target, bool show_num);
void GrepCommit(char *commit, bool show_num, char *target, char *file);
void DiffOne(char *path_1, char *path_2);
void DiffTwo(char *path_1, char *path_2);
int Diff(char *file1, char *file2, int begin_1, int end_1, int begin_2, int end_2);
void Merge(char *branch_1, char *branch_2, char *REPOSITORY);

// -----------------> Pre commit Functions <-----------------
void RunHook(char *REPOSITORY, char *file, bool print);
void TODO(const char *file, bool print);
void EOFBlankSpace(char *file, bool print);
void BalanceBraces(char *file, bool print);
void FormatCheck(char *file, bool print);
void FileSize(char *file, bool print);
void Character(char *file, bool print);

#endif