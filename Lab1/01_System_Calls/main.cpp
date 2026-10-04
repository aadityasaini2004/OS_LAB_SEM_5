#include <iostream>
#include <unistd.h>      // fork(), getpid(), getppid(), execlp()
#include <sys/wait.h>    // waitpid()
#include <fcntl.h>       // open()
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>       // strerror()
#include <cerrno>        // errno

using namespace std;

int main() {

    cout << "========================================\n";
    cout << "   SYSTEM CALLS AND PROCESS CREATION\n";
    cout << "========================================\n";

    // --------------------------------------------------
    // 1. Create a child process
    // --------------------------------------------------
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "ERROR: Failed to create child process.\n";
        return 1;
    }

    // ==================================================
    // CHILD PROCESS
    // ==================================================
    if (pid == 0) {

        cout << "\n[CHILD PROCESS]\n";

        // 2. Display Process ID and Parent Process ID
        cout << "Child Process ID  : " << getpid() << '\n';
        cout << "Parent Process ID : " << getppid() << '\n';

        // --------------------------------------------------
        // 4. Execute a harmless Linux command
        // --------------------------------------------------
        cout << "\nExecuting harmless Linux command from child...\n";
        cout << "Command: echo \"Hello from child process!\"\n";

        execlp("echo", "echo", "Hello from child process!", nullptr);

        // execlp() returns only if an error occurs
        cerr << "ERROR: Failed to execute command: "
             << strerror(errno) << '\n';

        _exit(1);
    }

    // ==================================================
    // PARENT PROCESS
    // ==================================================
    else {

        cout << "\n[PARENT PROCESS]\n";

        cout << "Parent Process ID : " << getpid() << '\n';
        cout << "Created Child ID  : " << pid << '\n';

        // --------------------------------------------------
        // 3. Wait for child process to complete
        // --------------------------------------------------
        int status;

        if (waitpid(pid, &status, 0) == -1) {
            cerr << "ERROR: waitpid() failed: "
                 << strerror(errno) << '\n';
            return 1;
        }

        cout << "\nChild process has completed.\n";

        if (WIFEXITED(status)) {
            cout << "Child exit status : "
                 << WEXITSTATUS(status) << '\n';
        }

        // --------------------------------------------------
        // 5. Create, write, read and close a controlled file
        // --------------------------------------------------
        cout << "\n[FILE OPERATION]\n";

        const char* filename = "test_file.txt";
        const char* message =
            "This is a controlled test file for the OS lab.\n";

        // Create/Open file for writing
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd == -1) {
            cerr << "ERROR: Could not create file: "
                 << strerror(errno) << '\n';
        } else {

            cout << "File created: " << filename << '\n';

            // Write data
            ssize_t bytesWritten = write(
                fd,
                message,
                strlen(message)
            );

            if (bytesWritten == -1) {
                cerr << "ERROR: Write operation failed: "
                     << strerror(errno) << '\n';
            } else {
                cout << "Bytes written: "
                     << bytesWritten << '\n';
            }

            // Close file after writing
            close(fd);
            cout << "File closed after writing.\n";
        }

        // Re-open file for reading
        fd = open(filename, O_RDONLY);

        if (fd == -1) {
            cerr << "ERROR: Could not open file for reading: "
                 << strerror(errno) << '\n';
        } else {

            char buffer[256];

            ssize_t bytesRead = read(
                fd,
                buffer,
                sizeof(buffer) - 1
            );

            if (bytesRead == -1) {
                cerr << "ERROR: Read operation failed: "
                     << strerror(errno) << '\n';
            } else {

                buffer[bytesRead] = '\0';

                cout << "File contents:\n";
                cout << buffer;
            }

            // Close file after reading
            close(fd);
            cout << "File closed after reading.\n";
        }

        // --------------------------------------------------
        // 6. Inspect an approved Linux interface
        // --------------------------------------------------
        cout << "\n[DEVICE / PROC INTERFACE]\n";

        const char* procFile = "/proc/self/status";

        fd = open(procFile, O_RDONLY);

        if (fd == -1) {
            cerr << "ERROR: Could not open "
                 << procFile << ": "
                 << strerror(errno) << '\n';
        } else {

            cout << "Successfully opened read-only interface: "
                 << procFile << '\n';

            char buffer[512];

            ssize_t bytesRead = read(
                fd,
                buffer,
                sizeof(buffer) - 1
            );

            if (bytesRead == -1) {

                cerr << "ERROR: Could not read "
                     << procFile << ": "
                     << strerror(errno) << '\n';

            } else {

                buffer[bytesRead] = '\0';

                cout << "Sample data from /proc/self/status:\n";
                cout << buffer;
            }

            close(fd);
            cout << "Closed /proc interface.\n";
        }

        // --------------------------------------------------
        // 7. Demonstrate error handling
        // --------------------------------------------------
        cout << "\n[ERROR HANDLING]\n";

        const char* invalidFile = "does_not_exist.txt";

        fd = open(invalidFile, O_RDONLY);

        if (fd == -1) {

            cout << "Expected file operation failure.\n";
            cout << "Could not open '" << invalidFile << "'.\n";
            cout << "Error message: "
                 << strerror(errno) << '\n';

        } else {

            cout << "Unexpectedly opened invalid test file.\n";
            close(fd);
        }

        // --------------------------------------------------
        // 8. Final evidence
        // --------------------------------------------------
        cout << "\n========================================\n";
        cout << "           EVIDENCE SUMMARY\n";
        cout << "========================================\n";

        cout << "Parent PID       : " << getpid() << '\n';
        cout << "Child PID        : " << pid << '\n';
        cout << "File operation   : CREATE / WRITE / READ / CLOSE\n";
        cout << "Linux interface  : /proc/self/status\n";
        cout << "Error handling   : Invalid file path handled\n";
        cout << "Child completion : Confirmed using waitpid()\n";

        cout << "\nProgram completed successfully.\n";
    }

    return 0;
}