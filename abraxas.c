#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_FILENAME 256
#define REPO_DIR "wizardry"
#define LOG_FILE "wizardry/log.txt"
#define BRANCHES_DIR "wizardry/branches"
#define COMMITS_DIR "wizardry/commits"

typedef struct Commit {
    char id[MAX_FILENAME];
    char message[256];
    char branch[MAX_FILENAME];
    time_t timestamp;
} Commit;

typedef struct Branch {
    char name[MAX_FILENAME];
    Commit *commits;
    int commit_count;
} Branch;


#define MAX_FILENAME 256
#define VERSIONS_DIR "wizardry"
#define LOG_FILE "wizardry/log.txt"
#include <direct.h>

#define MAX_FILENAME 256
#define VERSIONS_DIR "wizardry"
#define LOG_FILE "wizardry/log.txt"

// Function to create a directory if it doesn't exist
void create_dir(const char *dir_name) {
    struct stat st;
    if (stat(dir_name, &st) == -1) {
        if (_mkdir(dir_name) == -1) {
            perror("Error creating directory");
        }
    }
}

// Function to log operations
void log_operation(const char *operation) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        perror("Error opening log file");
        return;
    }
    time_t now;
    time(&now);
    fprintf(log, "[%s] %s\n", ctime(&now), operation);
    fclose(log);
}

// Initialize the version control system
void init_vcs() {
    create_dir(VERSIONS_DIR);
    log_operation("Initialized version control system.");
    printf("✨ Abracadabra! Version control system initialized.\n");
}

// Add a new version of the document
void add_version(const char *filename) {
    FILE *src, *dest;
    char dest_filename[MAX_FILENAME];
    int version = 1;
    struct dirent *entry;
    struct stat st;
    DIR *dp;

    dp = opendir(VERSIONS_DIR);
    if (dp != NULL) {
        while ((entry = readdir(dp))) {
            char entry_path[MAX_FILENAME];
            snprintf(entry_path, MAX_FILENAME, "%s/%s", VERSIONS_DIR, entry->d_name);
            if (stat(entry_path, &st) == 0 && S_ISREG(st.st_mode)) {
                version++;
            }
        }
        closedir(dp);
    }

    snprintf(dest_filename, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version);

    src = fopen(filename, "r");
    if (src == NULL) {
        perror("Error opening source file");
        return;
    }

    dest = fopen(dest_filename, "w");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(src);
        return;
    }

    char ch;
    while ((ch = fgetc(src)) != EOF) {
        fputc(ch, dest);
    }

    fclose(src);
    fclose(dest);
    log_operation("Added version.");
    printf("✨ Hocus Pocus! Version %d added.\n", version);
}

// List all versions
void list_versions() {
    struct dirent *entry;
    struct stat st;
    DIR *dp;

    dp = opendir(VERSIONS_DIR);
    if (dp == NULL) {
        perror("Error opening versions directory");
        return;
    }

    printf("🔍 Peekaboo! Available versions:\n");
    while ((entry = readdir(dp))) {
        char entry_path[MAX_FILENAME];
        snprintf(entry_path, MAX_FILENAME, "%s/%s", VERSIONS_DIR, entry->d_name);
        if (stat(entry_path, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("🔸 %s\n", entry->d_name);
        }
    }

    closedir(dp);
    log_operation("Listed versions.");
}

// View a specific version
void view_version(int version) {
    char filename[MAX_FILENAME];
    snprintf(filename, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    printf("📖 Eureka! Viewing version %d:\n", version);
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);
    log_operation("Viewed version.");
}

// Delete a specific version
void delete_version(int version) {
    char filename[MAX_FILENAME];
    snprintf(filename, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version);

    if (remove(filename) == 0) {
        log_operation("Deleted version.");
        printf("🔥 Abrakadabra! Version %d deleted.\n", version);
    } else {
        perror("Error deleting version");
    }
}

// Compare two versions
void compare_versions(int version1, int version2) {
    char filename1[MAX_FILENAME], filename2[MAX_FILENAME];
    snprintf(filename1, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version1);
    snprintf(filename2, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version2);

    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "r");

    if (file1 == NULL || file2 == NULL) {
        perror("Error opening files for comparison");
        if (file1) fclose(file1);
        if (file2) fclose(file2);
        return;
    }

    printf("🔍 Bibbidi Bobbidi Boo! Comparing versions %d and %d:\n", version1, version2);

    char ch1, ch2;
    int pos = 0;
    while ((ch1 = fgetc(file1)) != EOF && (ch2 = fgetc(file2)) != EOF) {
        pos++;
        if (ch1 != ch2) {
            printf("Difference at position %d: '%c' vs '%c'\n", pos, ch1, ch2);
        }
    }

    if (ch1 != EOF || ch2 != EOF) {
        printf("Files differ in length.\n");
    }

    fclose(file1);
    fclose(file2);
    log_operation("Compared versions.");
}

// Restore a specific version
void restore_version(int version, const char *current_filename) {
    char version_filename[MAX_FILENAME];
    snprintf(version_filename, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version);

    FILE *src = fopen(version_filename, "r");
    FILE *dest = fopen(current_filename, "w");

    if (src == NULL || dest == NULL) {
        perror("Error restoring version");
        if (src) fclose(src);
        if (dest) fclose(dest);
        return;
    }

    char ch;
    while ((ch = fgetc(src)) != EOF) {
        fputc(ch, dest);
    }

    fclose(src);
    fclose(dest);
    log_operation("Restored version.");
    printf("🔄 Hocus Pocus! Version %d restored to %s.\n", version, current_filename);
}

// Merge two versions
void merge_versions(int version1, int version2, const char *merged_filename) {
    char filename1[MAX_FILENAME], filename2[MAX_FILENAME];
    snprintf(filename1, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version1);
    snprintf(filename2, MAX_FILENAME, "%s/spell_%d.txt", VERSIONS_DIR, version2);

    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "r");
    FILE *merged = fopen(merged_filename, "w");

    if (file1 == NULL || file2 == NULL || merged == NULL) {
        perror("Error opening files for merging");
        if (file1) fclose(file1);
        if (file2) fclose(file2);
        if (merged) fclose(merged);
        return;
    }

    char ch;
    while ((ch = fgetc(file1)) != EOF) {
        fputc(ch, merged);
    }
    while ((ch = fgetc(file2)) != EOF) {
        fputc(ch, merged);
    }

    fclose(file1);
    fclose(file2);
    fclose(merged);
    log_operation("Merged versions.");
    printf("🌀 Bibbidi Bobbidi Boo! Versions %d and %d merged into %s.\n", version1, version2, merged_filename);
}

// Print help for commands
void print_help() {
    printf("📝 Magical Commands:\n");
    printf("  abracadabra       - Initialize the version control system\n");
    printf("  hocuspocus <file> - Add a new version of the document\n");
    printf("  peekaboo          - List all versions\n");
    printf("  eureka <ver>      - View a specific version\n");
    printf("  abrakadabra <ver> - Delete a specific version\n");
    printf("  bibbidi <ver1> <ver2> - Compare two versions\n");
    printf("  alakazam <ver> <file> - Restore a specific version to a file\n");
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "abracadabra") == 0) {
        init_vcs();
    } else if (strcmp(argv[1], "hocuspocus") == 0) {
        if (argc < 3) {
            printf("🔮 Missing filename.\n");
            return 1;
        }
        add_version(argv[2]);
    } else if (strcmp(argv[1], "peekaboo") == 0) {
        list_versions();
    } else if (strcmp(argv[1], "eureka") == 0) {
        if (argc < 3) {
            printf("🔮 Missing version number.\n");
            return 1;
        }
        view_version(atoi(argv[2]));
    } else if (strcmp(argv[1], "abrakadabra") == 0) {
        if (argc < 3) {
            printf("🔮 Missing version number.\n");
            return 1;
        }
        delete_version(atoi(argv[2]));
    } else if (strcmp(argv[1], "bibbidi") == 0) {
        if (argc < 4) {
            printf("🔮 Missing version numbers.\n");
            return 1;
        }
        compare_versions(atoi(argv[2]), atoi(argv[3]));
    } else if (strcmp(argv[1], "alakazam") == 0) {
        if (argc < 4) {
            printf("🔮 Missing version number or filename.\n");
            return 1;
        }
        restore_version(atoi(argv[2]), argv[3]);
    } else if (strcmp(argv[1], "merge") == 0) {
        if (argc < 5) {
            printf("🔮 Missing version numbers or filename.\n");
            return 1;
        }
        merge_versions(atoi(argv[2]), atoi(argv[3]), argv[4]);
    } else {
        printf("🔮 Unknown command.\n");
    }
    return 0;
}

