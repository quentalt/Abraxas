#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <direct.h>

#define VERSIONS_DIR "wizardry/versions"
#define COMMITS_DIR "wizardry/commits"
#define LOG_FILE "wizardry/log.txt"
#define CONFIG_FILE "wizardry/config.txt"
#define USERS_FILE "wizardry/users.txt"
#define TAGS_FILE "wizardry/tags.txt"
#define FILE_HISTORY_DIR "wizardry/file_history"
#define MAX_FILENAME 256
#define MAX_USERS 100
#define MAX_COMMITS 1000

typedef struct User {
    char name[MAX_FILENAME];
    char role[MAX_FILENAME]; // e.g., "admin", "developer"
} User;

typedef struct Commit {
    char id[MAX_FILENAME];
    char message[256];
    char branch[MAX_FILENAME];
} Commit;

User users[MAX_USERS];
int user_count = 0;

Commit commits[MAX_COMMITS];
int commit_count = 0;

int compare_and_merge(const char *commit_from, const char *commit_to, const char *output_path) {
    FILE *file_from = fopen(commit_from, "r");
    FILE *file_to = fopen(commit_to, "r");
    FILE *output = fopen(output_path, "w");

    if (file_from == NULL || file_to == NULL || output == NULL) {
        perror("Error opening files for comparison");
        if (file_from) fclose(file_from);
        if (file_to) fclose(file_to);
        if (output) fclose(output);
        return -1;
    }

    char line_from[256], line_to[256];
    int conflict_found = 0;

    while (fgets(line_from, sizeof(line_from), file_from) && fgets(line_to, sizeof(line_to), file_to)) {
        if (strcmp(line_from, line_to) != 0) {
            fprintf(output, "CONFLICT:\n%s\n%s\n", line_from, line_to);
            conflict_found = 1;
        } else {
            fputs(line_from, output);
        }
    }

    while (fgets(line_to, sizeof(line_to), file_to)) {
        fputs(line_to, output);
    }

    fclose(file_from);
    fclose(file_to);
    fclose(output);
    return conflict_found;
}


void log_operation(const char *message) {
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    fprintf(file, "%s: %s\n", ctime(&now), message);
    fclose(file);
}

void create_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        _mkdir(path);
    }
}

void init_vcs() {
    create_dir("wizardry");
    create_dir(VERSIONS_DIR);
    create_dir(COMMITS_DIR);
    create_dir(FILE_HISTORY_DIR);

    log_operation("Initialized version control system.");
    printf("✨ Version control system initialized.\n");
}

void create_branch(const char *branch_name) {
    char branch_path[MAX_FILENAME];
    snprintf(branch_path, MAX_FILENAME, "%s/%s", VERSIONS_DIR, branch_name);

    create_dir(branch_path);
    log_operation("Created branch.");
    printf("🌿 Branch '%s' created.\n", branch_name);
}

void switch_branch(const char *branch_name) {
    char branch_path[MAX_FILENAME];
    snprintf(branch_path, MAX_FILENAME, "%s/%s", VERSIONS_DIR, branch_name);

    if (access(branch_path, F_OK) != -1) {
        FILE *file = fopen(CONFIG_FILE, "w");
        if (file == NULL) {
            perror("Error switching branch");
            return;
        }
        fprintf(file, "current_branch=%s\n", branch_name);
        fclose(file);
        log_operation("Switched branch.");
        printf("🔀 Switched to branch '%s'.\n", branch_name);
    } else {
        printf("❌ Branch '%s' does not exist.\n", branch_name);
    }
}

void delete_branch(const char *branch_name) {
    char branch_path[MAX_FILENAME];
    snprintf(branch_path, MAX_FILENAME, "%s/%s", VERSIONS_DIR, branch_name);

    if (rmdir(branch_path) == 0) {
        log_operation("Deleted branch.");
        printf("❌ Branch '%s' deleted.\n", branch_name);
    } else {
        perror("Error deleting branch");
    }
}

void add_commit(const char *id, const char *message, const char *branch) {
    if (commit_count < MAX_COMMITS) {
        strcpy(commits[commit_count].id, id);
        strcpy(commits[commit_count].message, message);
        strcpy(commits[commit_count].branch, branch);
        commit_count++;
    } else {
        printf("❌ Maximum number of commits reached.\n");
    }
}

void commit_changes(const char *branch_name, const char *message) {
    char commit_id[MAX_FILENAME];
    snprintf(commit_id, MAX_FILENAME, "%s_%ld", branch_name, time(NULL));

    char commit_path[MAX_FILENAME];
    snprintf(commit_path, MAX_FILENAME, "%s/%s", COMMITS_DIR, commit_id);

    FILE *file = fopen(commit_path, "w");
    if (file == NULL) {
        perror("Error committing changes");
        return;
    }

    fprintf(file, "Branch: %s\nMessage: %s\n", branch_name, message);
    fclose(file);

    add_commit(commit_id, message, branch_name);
    log_operation("Committed changes.");
    printf("💾 Changes committed to branch '%s' with ID '%s'.\n", branch_name, commit_id);
}

void list_branches() {
    struct dirent *entry;
    DIR *dp = opendir(VERSIONS_DIR);
    if (dp == NULL) {
        perror("Error opening versions directory");
        return;
    }

    printf("🌿 Branches:\n");
    while ((entry = readdir(dp))) {
        if (entry->d_name[0] != '.') {
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dp);
    log_operation("Listed branches.");
}

void list_commits(const char *branch_name) {
    printf("📜 Commits for branch '%s':\n", branch_name);
    for (int i = 0; i < commit_count; i++) {
        if (strcmp(commits[i].branch, branch_name) == 0) {
            printf("ID: %s\nMessage: %s\n", commits[i].id, commits[i].message);
        }
    }
    log_operation("Listed commits.");
}

void merge_branches(const char *branch_from, const char *branch_to) {
    char commit_path_from[MAX_FILENAME], commit_path_to[MAX_FILENAME];
    struct dirent *entry;
    DIR *dp = opendir(COMMITS_DIR);
    if (dp == NULL) {
        perror("Error opening commits directory");
        return;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_name[0] != '.') {
            snprintf(commit_path_from, MAX_FILENAME, "%s/%s", COMMITS_DIR, entry->d_name);

            FILE *file = fopen(commit_path_from, "r");
            if (file == NULL) {
                perror("Error opening commit file");
                continue;
            }

            char line[256];
            int is_from_branch = 0;
            while (fgets(line, sizeof(line), file)) {
                if (strncmp(line, "Branch: ", 8) == 0 && strstr(line, branch_from)) {
                    is_from_branch = 1;
                    break;
                }
            }
            fclose(file);

            if (is_from_branch) {
                snprintf(commit_path_to, MAX_FILENAME, "%s/%s", VERSIONS_DIR, branch_to);
                FILE *file_from = fopen(commit_path_from, "r");
                FILE *file_to = fopen(commit_path_to, "r");

                if (file_from == NULL || file_to == NULL) {
                    perror("Error opening commit files for merge");
                    if (file_from) fclose(file_from);
                    if (file_to) fclose(file_to);
                    continue;
                }

                char merged_path[MAX_FILENAME];
                snprintf(merged_path, MAX_FILENAME, "%s/%s_%s", COMMITS_DIR, branch_to, entry->d_name);

                int conflict_found = compare_and_merge(commit_path_from, commit_path_to, merged_path);
                if (conflict_found) {
                    printf("❌ Conflict found in commit '%s'.\n", entry->d_name);
                } else {
                    printf("✅ Merged commit '%s' from branch '%s' to branch '%s'.\n", entry->d_name, branch_from, branch_to);
                }

                fclose(file_from);
                fclose(file_to);
            }
        }
    }

    closedir(dp);
    log_operation("Merged branches.");
}




void track_file(const char *filename, const char *commit_id) {
    char history_file[MAX_FILENAME];
    snprintf(history_file, MAX_FILENAME, "%s/%s_history.txt", FILE_HISTORY_DIR, filename);

    FILE *file = fopen(history_file, "a");
    if (file == NULL) {
        perror("Error tracking file");
        return;
    }

    fprintf(file, "Commit: %s\n", commit_id);
    fclose(file);
    log_operation("Tracked file changes.");
}

void show_file_history(const char *filename) {
    char history_file[MAX_FILENAME];
    snprintf(history_file, MAX_FILENAME, "%s/%s_history.txt", FILE_HISTORY_DIR, filename);

    FILE *file = fopen(history_file, "r");
    if (file == NULL) {
        perror("Error showing file history");
        return;
    }

    char line[256];
    printf("📜 History for file '%s':\n", filename);
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
    log_operation("Displayed file history.");
}


void create_tag(const char *tag_name, const char *commit_id) {
    FILE *file = fopen(TAGS_FILE, "a");
    if (file == NULL) {
        perror("Error creating tag");
        return;
    }

    fprintf(file, "%s %s\n", tag_name, commit_id);
    fclose(file);
    log_operation("Created tag.");
}

void list_tags() {
    FILE *file = fopen(TAGS_FILE, "r");
    if (file == NULL) {
        perror("Error listing tags");
        return;
    }

    char tag_name[MAX_FILENAME], commit_id[MAX_FILENAME];
    printf("🏷️ Tags:\n");
    while (fscanf(file, "%s %s", tag_name, commit_id) != EOF) {
        printf("%s -> %s\n", tag_name, commit_id);
    }

    fclose(file);
    log_operation("Listed tags.");
}

void delete_tag(const char *tag_name) {
    FILE *file = fopen(TAGS_FILE, "r");
    if (file == NULL) {
        perror("Error deleting tag");
        return;
    }

    char temp_file[MAX_FILENAME];
    snprintf(temp_file, MAX_FILENAME, "%s_temp", TAGS_FILE);

    FILE *temp = fopen(temp_file, "w");
    if (temp == NULL) {
        perror("Error deleting tag");
        fclose(file);
        return;
    }

    char tag[MAX_FILENAME], commit_id[MAX_FILENAME];
    while (fscanf(file, "%s %s", tag, commit_id) != EOF) {
        if (strcmp(tag, tag_name) != 0) {
            fprintf(temp, "%s %s\n", tag, commit_id);
        }
    }

    fclose(file);
    fclose(temp);
    remove(TAGS_FILE);
    rename(temp_file, TAGS_FILE);
    log_operation("Deleted tag.");
    printf("❌ Tag '%s' deleted.\n", tag_name);
}

void add_user(const char *name, const char *role) {
    if (user_count < MAX_USERS) {
        strcpy(users[user_count].name, name);
        strcpy(users[user_count].role, role);
        user_count++;
    } else {
        printf("❌ Maximum number of users reached.\n");
    }
}

void list_users() {
    printf("👥 Users:\n");
    for (int i = 0; i < user_count; i++) {
        printf("%s (%s)\n", users[i].name, users[i].role);
    }
    log_operation("Listed users.");
}

void delete_user(const char *name) {
    char temp_file[MAX_FILENAME];
    snprintf(temp_file, MAX_FILENAME, "%s_temp", USERS_FILE);

    FILE *file = fopen(USERS_FILE, "r");
    if (file == NULL) {
        perror("Error deleting user");
        return;
    }

    FILE *temp = fopen(temp_file, "w");
    if (temp == NULL) {
        perror("Error deleting user");
        fclose(file);
        return;
    }

    char user_name[MAX_FILENAME], role[MAX_FILENAME];
    while (fscanf(file, "%s %s", user_name, role) != EOF) {
        if (strcmp(user_name, name) != 0) {
            fprintf(temp, "%s %s\n", user_name, role);
        }
    }

    fclose(file);
    fclose(temp);
    remove(USERS_FILE);
    rename(temp_file, USERS_FILE);
    log_operation("Deleted user.");
    printf("❌ User '%s' deleted.\n", name);
}

void show_help() {
    printf("🧙‍♂️ Wizardry VCS\n");
    printf("Commands:\n");
    printf("  abracadabra: Initialize version control system\n");
    printf("  branch <name>: Create a new branch\n");
    printf("  peekaboo <name>: Switch to a branch\n");
    printf("  alakazam <name>: Delete a branch\n");
    printf("  simsalabim <message>: Commit changes\n");
    printf("  branches: List all branches\n");
    printf("  commits: List all commits for the current branch\n");
    printf("  mergolet <from> <to>: Merge changes from one branch to another\n");
    printf("  alatrace <file>: Track changes to a file\n");
    printf("  parchment <file>: Show history of a file\n");
    printf("  tagolet <name>: Tag a commit\n");
    printf("  tag <name>: Create a tag for the current commit\n");
    printf("  tagolets: List all tags\n");
    printf("  delete-tag <name>: Delete a tag\n");
    printf("  volunteer <name> <role>: Add a user\n");
    printf("  volunteers: List all users\n");
    printf("  delete-user <name>: Delete a user\n");
    printf("  aidezmoi: Show help\n");
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", argv[0]);
        return 1;
    }

    FILE *config = fopen(CONFIG_FILE, "r");
    if (config == NULL) {
        init_vcs();
    } else {
        fclose(config);
    }

    FILE *users_file = fopen(USERS_FILE, "r");
    if (users_file == NULL) {
        FILE *file = fopen(USERS_FILE, "w");
        if (file == NULL) {
            perror("Error creating users file");
            return 1;
        }
        fclose(file);
    } else {
        char name[MAX_FILENAME], role[MAX_FILENAME];
        while (fscanf(users_file, "%s %s", name, role) != EOF) {
            add_user(name, role);
        }
        fclose(users_file);
    }

FILE *tags_file = fopen(TAGS_FILE, "r");

    if (tags_file == NULL) {
        FILE *file = fopen(TAGS_FILE, "w");
        if (file == NULL) {
            perror("Error creating tags file");
            return 1;
        }
        fclose(file);
    } else {
        fclose(tags_file);
    }

    FILE *commits_file = fopen(COMMITS_DIR, "r");
    if (commits_file == NULL) {
        FILE *file = fopen(COMMITS_DIR, "w");
        if (file == NULL) {
            perror("Error creating commits file");
            return 1;
        }
        fclose(file);
    } else {
        fclose(commits_file);
    }

    FILE *file_history = fopen(FILE_HISTORY_DIR, "r");
    if (file_history == NULL) {
        FILE *file = fopen(FILE_HISTORY_DIR, "w");
        if (file == NULL) {
            perror("Error creating file history directory");
            return 1;
        }
        fclose(file);
    } else {
        fclose(file_history);
    }

    if (strcmp(argv[1], "abracadabra") == 0) {
        init_vcs();
    } else if (strcmp(argv[1], "branch") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s branch <name>\n", argv[0]);
            return 1;
        }
        create_branch(argv[2]);
    } else if (strcmp(argv[1], "peekaboo") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s switch <name>\n", argv[0]);
            return 1;
        }
        switch_branch(argv[2]);
    } else if (strcmp(argv[1], "alakazam") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s delete <name>\n", argv[0]);
            return 1;
        }
        delete_branch(argv[2]);
    } else if (strcmp(argv[1], "simsalabim") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s commit <message>\n", argv[0]);
            return 1;
        }
        FILE *config = fopen(CONFIG_FILE, "r");
        if (config == NULL) {
            printf("❌ No branch selected.\n");
            return 1;
        }
        char line[256];
        char branch_name[MAX_FILENAME];
        while (fgets(line, sizeof(line), config)) {
            if (strncmp(line, "current_branch=", 15) == 0) {
                sscanf(line, "current_branch=%s", branch_name);
                break;
            }
        }
        fclose(config);
        commit_changes(branch_name, argv[2]);
    } else if (strcmp(argv[1], "branches") == 0) {
        list_branches();
    } else if (strcmp(argv[1], "commits") == 0) {
        FILE *config = fopen(CONFIG_FILE, "r");
        if (config == NULL) {
            printf("❌ No branch selected.\n");
            return 1;
        }
        char line[256];
        char branch_name[MAX_FILENAME];
        while (fgets(line, sizeof(line), config)) {
            if (strncmp(line, "current_branch=", 15) == 0) {
                sscanf(line, "current_branch=%s", branch_name);
                break;
            }
        }
        fclose(config);
        list_commits(branch_name);
    } else if (strcmp(argv[1], "mergolet") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s merge <from> <to>\n", argv[0]);
            return 1;
        }
        merge_branches(argv[2], argv[3]);
    } else if (strcmp(argv[1], "alatrace") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s track <file>\n", argv[0]);
            return 1;
        }
        FILE *config = fopen(CONFIG_FILE, "r");
        if (config == NULL) {
            printf("❌ No branch selected.\n");
            return 1;
        }
        char line[256];
        char branch_name[MAX_FILENAME];
        while (fgets(line, sizeof(line), config)) {
            if (strncmp(line, "current_branch=", 15) == 0) {
                sscanf(line, "current_branch=%s", branch_name);
                break;
            }
        }
        fclose(config);
        track_file(argv[2], branch_name);
    } else if (strcmp(argv[1], "parchment") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s history <file>\n", argv[0]);
            return 1;
        }
        show_file_history(argv[2]);
    } else if (strcmp(argv[1], "tagolet") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s tag <name>\n", argv[0]);
            return 1;
        }
        FILE *config = fopen(CONFIG_FILE, "r");
        if (config == NULL) {
            printf("❌ No branch selected.\n");
            return 1;
        }
    } else if (strcmp(argv[1], "tag") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s tag <name>\n", argv[0]);
            return 1;
        }
        FILE *config = fopen(CONFIG_FILE, "r");
        if (config == NULL) {
            printf("❌ No branch selected.\n");
            return 1;
        }
        char line[256];
        char branch_name[MAX_FILENAME];
        while (fgets(line, sizeof(line), config)) {
            if (strncmp(line, "current_branch=", 15) == 0) {
                sscanf(line, "current_branch=%s", branch_name);
                break;
            }
        }
        fclose(config);
        create_tag(argv[2], branch_name);
    }
    else if (strcmp(argv[1], "tagolets") == 0) {
        list_tags();
    } else if (strcmp(argv[1], "delete-tag") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s delete-tag <name>\n", argv[0]);
            return 1;
        }
        delete_tag(argv[2]);
    } else if (strcmp(argv[1], "volunteer") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s user <name> <role>\n", argv[0]);
            return 1;
        }
        add_user(argv[2], argv[3]);
    } else if (strcmp(argv[1], "volunteers") == 0) {
        list_users();
    } else if (strcmp(argv[1], "delete-user") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s delete-user <name>\n", argv[0]);
            return 1;
        }
        delete_user(argv[2]);
    } else if (strcmp(argv[1], "aidezmoi") == 0) {
        show_help();
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
