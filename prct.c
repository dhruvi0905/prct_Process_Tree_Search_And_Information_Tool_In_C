#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Standard library functions
#include <string.h>     // String handling functions
#include <dirent.h>     // Directory handling functions
#include <ctype.h>      // Character handling functions
#include <unistd.h>     // System calls like getpid, getppid
#include <sys/types.h>  // Data types for system calls
#include <sys/stat.h>   // File status functions
#include <signal.h>     // Signal handling functions

#define MAX_PROCESSES 1024  // Define the maximum number of processes

// Function to get the parent PID (PPID) of a given process
int get_ppid(int pid) {
    char path[50], line[256]; // Buffer to store file path and line content
    FILE *file; // File pointer
    int ppid = -1; // Initialize PPID to -1

    sprintf(path, "/proc/%d/status", pid); // Construct the file path
    file = fopen(path, "r"); // Open the status file
    if (!file) return -1; // Return -1 if file can't be opened

    while (fgets(line, sizeof(line), file)) { // Read file line by line
        if (strncmp(line, "PPid:", 5) == 0) { // Check for PPid field
            sscanf(line, "PPid: %d", &ppid);  // Extract the PPID value
            break; // Exit loop once PPID is found
        }
    }
    fclose(file); // Close the file
    return ppid; // Return the extracted PPID
}

// Function to check if a process belongs to the given root process tree
int is_in_tree(int root, int pid) {
    while (pid > 0) { // Traverse up the process hierarchy
        if (pid == root) return 1; // If PID matches root, return true
        pid = get_ppid(pid); // Move to the parent process
    }
    return 0; // Return false if not in the tree
}

// Function to list all grandchildren of a process
void list_grandchildren(int pid) {
    DIR *dir = opendir("/proc");  // Open the /proc directory
    struct dirent *entry; // Directory entry structure
    int child_pid;

    if (!dir) return; // Exit if directory can't be opened

    while ((entry = readdir(dir)) != NULL) { // Read each entry
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric entries
        child_pid = atoi(entry->d_name); // Convert to integer PID
        if (get_ppid(get_ppid(child_pid)) == pid) {  // Check for grandchild
            printf("%d\n", child_pid); // Print grandchild PID
        }
    }
    closedir(dir); // Close directory
}
// Function to recursively list all non-direct descendants
void list_non_direct_descendants(int pid) {
    DIR *dir = opendir("/proc"); // Open the /proc directory
    struct dirent *entry; // Directory entry structure
    int child_pid, ppid;

    if (!dir) return; // Exit if directory can't be opened

    while ((entry = readdir(dir)) != NULL) { // Read each entry
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric entries
        child_pid = atoi(entry->d_name); // Convert to integer PID
        ppid = get_ppid(child_pid);  // Get parent PID


        if (ppid > 0 && ppid != pid) { // Ensure valid PPID
            int grandparent_pid = get_ppid(ppid); // Get grandparent PID
            if (grandparent_pid == pid) {  // Check if grandparent matches root PID
                printf("%d\n", child_pid); // Print PID
                list_non_direct_descendants(child_pid); // Recursively check deeper levels 
            } else {
                int great_grandparent_pid = get_ppid(grandparent_pid); // Get great-grandparent PID
                if (great_grandparent_pid == pid) { // Check if it matches root PID
                    printf("%d\n", child_pid); // Print PID
                    list_non_direct_descendants(child_pid); // Continue recursive search
                }
            }
        }
    }
    closedir(dir); // Close directory
}

// Function to list direct children of a process
void list_children(int pid) {
    DIR *dir = opendir("/proc"); // Open the /proc directory
    struct dirent *entry; // Directory entry structure
    int child_pid;

    if (!dir) return;  // Exit if directory can't be opened

    while ((entry = readdir(dir)) != NULL) { // Read each entry
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric entries
        child_pid = atoi(entry->d_name); // Convert to integer PID
        if (get_ppid(child_pid) == pid) { // Check if it's a child
            printf("%d\n", child_pid); // Print child PID
        }
    }
    closedir(dir); // Close directory
}

// Function to check if a process is defunct (zombie)
int is_defunct(int pid) {
    char path[50], state; // Buffer for path and process state
    FILE *file; // File pointer

    sprintf(path, "/proc/%d/stat", pid); // Construct file path
    file = fopen(path, "r");  // Open file
    if (!file) return 0; // Return false if file can't be opened

    fscanf(file, "%*d %*s %c", &state); // Read process state
    fclose(file); // Close file
    return (state == 'Z');  // 'Z' indicates a zombie process
}
// Function to list and count defunct descendant processes
void count_defunct_descendants(int pid, int *count) {
    DIR *dir = opendir("/proc"); // Open the /proc directory
    struct dirent *entry; // Directory entry structure
    int child_pid;

    if (!dir) return; // Exit if directory can't be opened

    while ((entry = readdir(dir)) != NULL) { // Read each entry
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric entries
        child_pid = atoi(entry->d_name); // Convert to integer PID
        if (get_ppid(child_pid) == pid) { // Check if it's a child
            if (is_defunct(child_pid)) { // Check if defunct
                (*count)++; // Increment count
            }
            count_defunct_descendants(child_pid, count);  // Recursively check descendants
        }
    }
    closedir(dir); // Close directory
}

// Function to list all descendants (recursive)
void list_descendants(int pid, int *count, int defunct_only, int orphan_only, int non_direct) {
    DIR *dir = opendir("/proc"); // Open the /proc directory to read process information
    struct dirent *entry; // Declare a structure to store directory entry information
    int child_pid, ppid; // Declare variables for child process ID and parent process ID

    if (!dir) return; // If the directory can't be opened, return

    // Loop through all entries in the /proc directory
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric directory names
        child_pid = atoi(entry->d_name); // Convert directory name to integer (PID)
        ppid = get_ppid(child_pid); // Get the parent process ID for the current child PID

        // If the parent process ID matches the given PID
        if (ppid == pid) {
            // Check if the process should be printed based on the given conditions
            if ((defunct_only && is_defunct(child_pid)) ||
                (orphan_only && get_ppid(child_pid) == 1) ||
                (!defunct_only && !orphan_only)) {
                printf("%d\n", child_pid); // Print the child PID
                (*count)++; // Increment the count of descendants
            }
            // If non-direct descendants are requested, recursively list them
            if (non_direct) {
                list_descendants(child_pid, count, defunct_only, orphan_only, 1);
            }
        }
    }
    closedir(dir); // Close the /proc directory
}

// Function to list siblings of a process
void list_siblings(int pid, int defunct_only) {
    int parent_pid = get_ppid(pid); // Get the parent process ID of the given PID
    if (parent_pid == -1) return; // If no parent process ID, return

    DIR *dir = opendir("/proc"); // Open the /proc directory to read process information
    struct dirent *entry; // Declare a structure to store directory entry information
    int child_pid; // Declare a variable for the child process ID

    if (!dir) return; // If the directory can't be opened, return

    // Loop through all entries in the /proc directory
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric directory names
        child_pid = atoi(entry->d_name); // Convert directory name to integer (PID)
        // Check if the child process has the same parent process ID and isn't the given PID
        if (get_ppid(child_pid) == parent_pid && child_pid != pid) {
            // If defunct processes are requested, print only defunct ones
            if (!defunct_only || is_defunct(child_pid)) {
                printf("%d\n", child_pid); // Print the sibling PID
            }
        }
    }
    closedir(dir); // Close the /proc directory
}

// Function to check if a process is a descendant of a given root process
int is_descendant(int root, int pid) {
    while (pid > 0) { // Loop until the PID is valid (greater than 0)
        if (pid == root) return 1; // If the PID matches the root process, return 1 (true)
        pid = get_ppid(pid); // Get the parent process ID of the current PID
    }
    return 0; // If no match is found, return 0 (false)
}

// Function to kill parent processes of zombie processes
void kill_parents_of_zombies(int root_pid) {
    DIR *dir = opendir("/proc"); // Open the /proc directory to read process information
    struct dirent *entry; // Declare a structure to store directory entry information
    int child_pid, parent_pid; // Declare variables for child process ID and parent process ID

    if (!dir) return; // If the directory can't be opened, return

    // Loop through all entries in the /proc directory
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric directory names
        child_pid = atoi(entry->d_name); // Convert directory name to integer (PID)

        // Check if the process is a zombie and a descendant of the root process
        if (is_defunct(child_pid) && is_in_tree(root_pid, child_pid)) {
            parent_pid = get_ppid(child_pid); // Get the parent process ID of the zombie process

            // Ensure the parent is within the root process tree
            if (parent_pid > 0 && is_in_tree(root_pid, parent_pid)) {
                printf("Killing parent process: %d\n", parent_pid); // Print the parent PID to be killed
                kill(parent_pid, SIGKILL); // Send SIGKILL to terminate the parent process
            }
        }
    }
    closedir(dir); // Close the /proc directory
}

// Function to send a signal to all descendants of a process
void send_signal_to_descendants(int pid, int signal) {
    DIR *dir = opendir("/proc"); // Open the /proc directory to read process information
    struct dirent *entry; // Declare a structure to store directory entry information
    int child_pid; // Declare a variable for the child process ID

    if (!dir) return; // If the directory can't be opened, return

    // Loop through all entries in the /proc directory
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue; // Skip non-numeric directory names
        child_pid = atoi(entry->d_name); // Convert directory name to integer (PID)

        // Ensure we only signal actual descendants
        if (is_in_tree(pid, child_pid) && get_ppid(child_pid) == pid) {
            send_signal_to_descendants(child_pid, signal); // Recursively send the signal to descendants
            printf("Sending signal %d to process %d\n", signal, child_pid); // Print which signal is being sent to which process
            kill(child_pid, signal); // Send the signal to the child process
        }
    }
    closedir(dir); // Close the /proc directory
}

// Function to get the PPID of a given PID
int get_parent_pid(int pid) {
    char path[256], line[256]; // Declare buffers for file path and lines
    FILE *fp; // Declare a pointer for the file
    int ppid = -1; // Initialize parent PID to -1 (indicating error if not found)

    snprintf(path, sizeof(path), "/proc/%d/status", pid); // Construct the file path for the given PID's status
    fp = fopen(path, "r"); // Open the file in read mode
    if (!fp) return -1; // If the file can't be opened, return -1

    // Read the file line by line
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "PPid:", 5) == 0) { // If the line starts with "PPid:"
            sscanf(line, "PPid: %d", &ppid); // Extract the parent PID from the line
            break; // Break the loop once the parent PID is found
        }
    }
    fclose(fp); // Close the file
    return ppid; // Return the parent process ID
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3) { // Check if enough arguments are provided
        fprintf(stderr, "Usage: %s [root_process] [process_id] [Option]\n", argv[0]); // Print usage instructions
        return 1; // Return an error code if insufficient arguments
    }

    int root_process = atoi(argv[1]); // Convert the root process ID from string to integer
    int process_id = atoi(argv[2]); // Convert the process ID from string to integer
    char *option = (argc == 4) ? argv[3] : NULL; // Get the option argument if provided

    // Validate process hierarchy
    if (!is_in_tree(root_process, process_id)) { // Check if the process belongs to the tree rooted at root_process
        printf("The process %d does not belong to the tree rooted at %d\n", process_id, root_process); // Print error message
        return 0; // Return if the process is not part of the tree
    }

    // If no option is provided, print PID and PPID
    if (!option) {
        printf("%d %d\n", process_id, get_ppid(process_id)); // Print the process ID and parent process ID
        return 0; // Return after printing the PID and PPID
    }

    // Handle different options
    int count = 0; // Initialize a count variable for descendants
    if (strcmp(option, "-id") == 0) { // If the option is -id
        list_children(process_id); // List the children of the process
    } else if (strcmp(option, "-dc") == 0) { // If the option is -dc
        count_defunct_descendants(process_id, &count); // Count the defunct descendants
        printf("%d\n", count); // Print the count of defunct descendants
    } else if (strcmp(option, "-ds") == 0) { // If the option is -ds
        list_non_direct_descendants(process_id); // List non-direct descendants
    } else if (strcmp(option, "-df") == 0) { // If the option is -df
        list_descendants(process_id, &count, 1, 0, 1); // List defunct descendants
    } else if (strcmp(option, "-op") == 0) { // If the option is -op
        list_descendants(process_id, &count, 0, 1, 1); // List orphan descendants
    } else if (strcmp(option, "-gc") == 0) { // If the option is -gc
        list_grandchildren(process_id); // List the grandchildren of the process
    } else if (strcmp(option, "-do") == 0) { // If the option is -do
        printf(is_defunct(process_id) ? "Defunct\n" : "Not Defunct\n"); // Check if the process is defunct
    } else if (strcmp(option, "-so") == 0) { // If the option is -so
        printf(get_ppid(process_id) == 1 ? "Orphan\n" : "Not Orphan\n"); // Check if the process is orphan
    } else if (strcmp(option, "-lg") == 0) { // If the option is -lg
        list_siblings(process_id, 0); // List siblings of the process
    } else if (strcmp(option, "-lz") == 0) { // If the option is -lz
        list_siblings(process_id, 1); // List zombie siblings of the process
    } else if (strcmp(option, "--pz") == 0) { // If the option is --pz
        kill_parents_of_zombies(process_id); // Kill parents of zombie processes
    } else if (strcmp(option, "-sk") == 0) { // If the option is -sk
        send_signal_to_descendants(process_id, SIGKILL); // Send SIGKILL to all descendants
    } else if (strcmp(option, "-st") == 0) { // If the option is -st
        send_signal_to_descendants(process_id, SIGSTOP); // Send SIGSTOP to all descendants
    } else if (strcmp(option, "-dt") == 0) { // If the option is -dt
        send_signal_to_descendants(process_id, SIGCONT); // Send SIGCONT to all descendants
    } else if (strcmp(option, "-rp") == 0) { // If the option is -rp
        kill(process_id, SIGKILL); // Kill the specified process
    } else { // If the option is invalid
        printf("Invalid option\n"); // Print an error message for invalid option
    }

    return 0; // Return 0 to indicate successful execution
}
