#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <limits>

using namespace std;

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int priority;
};

struct Execution {
    int processId;      // -1 means CPU idle
    int startTime;
    int endTime;
};

struct RRProcess {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
};

// ------------------------------------------------------------
// Validate process information
// ------------------------------------------------------------
bool isValidProcess(const Process& p) {

    return p.id > 0 &&
           p.arrivalTime >= 0 &&
           p.burstTime > 0 &&
           p.priority > 0;
}

// ------------------------------------------------------------
// Display original input data
// ------------------------------------------------------------
void displayProcesses(const vector<Process>& processes) {

    cout << "\nOriginal Process Dataset\n";
    cout << "------------------------\n";

    cout << left
         << setw(10) << "PID"
         << setw(15) << "Arrival"
         << setw(12) << "Burst"
         << setw(12) << "Priority"
         << '\n';

    for (const auto& p : processes) {

        cout << left
             << setw(10) << p.id
             << setw(15) << p.arrivalTime
             << setw(12) << p.burstTime
             << setw(12) << p.priority
             << '\n';
    }
}

// ------------------------------------------------------------
// Display execution sequence
// ------------------------------------------------------------
void displaySequence(const vector<Execution>& execution) {

    cout << "\nExecution Sequence: ";

    for (size_t i = 0; i < execution.size(); i++) {

        if (execution[i].processId == -1) {
            cout << "IDLE";
        } else {
            cout << "P" << execution[i].processId;
        }

        if (i != execution.size() - 1) {
            cout << " -> ";
        }
    }

    cout << '\n';
}

// ------------------------------------------------------------
// Display detailed execution intervals
// ------------------------------------------------------------
void displayExecution(const vector<Execution>& execution) {

    cout << "\nExecution Intervals\n";
    cout << "-------------------\n";

    for (const auto& e : execution) {

        if (e.processId == -1) {

            cout << "CPU IDLE : ["
                 << e.startTime
                 << ", "
                 << e.endTime
                 << ")\n";

        } else {

            cout << "P" << e.processId
                 << "       : ["
                 << e.startTime
                 << ", "
                 << e.endTime
                 << ")\n";
        }
    }
}

// ============================================================
// NON-PREEMPTIVE PRIORITY SCHEDULING
// ============================================================
//
// Convention:
// Smaller priority number = higher priority.
//
// Tie-breaking:
// 1. Smaller priority number
// 2. Earlier arrival time
// 3. Smaller Process ID
// ============================================================

vector<Execution> priorityScheduling(
    const vector<Process>& originalProcesses) {

    // Preserve original input data.
    vector<Process> processes = originalProcesses;

    vector<bool> completed(processes.size(), false);

    vector<Execution> execution;

    int currentTime = 0;
    int completedCount = 0;

    while (completedCount < static_cast<int>(processes.size())) {

        int selectedIndex = -1;

        // Find highest-priority process among arrived processes.
        for (int i = 0;
             i < static_cast<int>(processes.size());
             i++) {

            if (completed[i]) {
                continue;
            }

            // Process hasn't arrived yet.
            if (processes[i].arrivalTime > currentTime) {
                continue;
            }

            if (selectedIndex == -1) {
                selectedIndex = i;
                continue;
            }

            const Process& current = processes[i];
            const Process& selected = processes[selectedIndex];

            // Smaller priority number = higher priority.
            if (current.priority < selected.priority) {

                selectedIndex = i;

            } else if (current.priority == selected.priority) {

                // Tie -> earlier arrival time.
                if (current.arrivalTime < selected.arrivalTime) {

                    selectedIndex = i;

                } else if (current.arrivalTime ==
                           selected.arrivalTime) {

                    // Tie -> smaller PID.
                    if (current.id < selected.id) {
                        selectedIndex = i;
                    }
                }
            }
        }

        // ----------------------------------------------------
        // No process is currently available.
        // CPU becomes idle until next process arrives.
        // ----------------------------------------------------
        if (selectedIndex == -1) {

            int nextArrival =
                numeric_limits<int>::max();

            for (int i = 0;
                 i < static_cast<int>(processes.size());
                 i++) {

                if (!completed[i]) {

                    nextArrival = min(
                        nextArrival,
                        processes[i].arrivalTime
                    );
                }
            }

            if (currentTime < nextArrival) {

                execution.push_back({
                    -1,
                    currentTime,
                    nextArrival
                });

                currentTime = nextArrival;
            }

            continue;
        }

        // ----------------------------------------------------
        // Execute selected process completely.
        // Priority scheduling here is NON-PREEMPTIVE.
        // ----------------------------------------------------
        Process& p = processes[selectedIndex];

        int startTime = currentTime;
        int endTime = currentTime + p.burstTime;

        execution.push_back({
            p.id,
            startTime,
            endTime
        });

        currentTime = endTime;

        completed[selectedIndex] = true;
        completedCount++;
    }

    return execution;
}

// ============================================================
// ROUND ROBIN SCHEDULING
// ============================================================
//
// Ready queue: FIFO
// Time quantum: user input
//
// When a process uses its quantum:
//   - newly arrived processes are added to queue
//   - current process goes to the back if unfinished
// ============================================================

vector<Execution> roundRobin(
    const vector<Process>& originalProcesses,
    int timeQuantum) {

    // --------------------------------------------------------
    // Copy and sort only the working data.
    // Original data remains unchanged.
    // --------------------------------------------------------
    vector<RRProcess> processes;

    for (const auto& p : originalProcesses) {

        processes.push_back({
            p.id,
            p.arrivalTime,
            p.burstTime,
            p.burstTime
        });
    }

    sort(processes.begin(), processes.end(),
         [](const RRProcess& a, const RRProcess& b) {

             if (a.arrivalTime != b.arrivalTime) {
                 return a.arrivalTime < b.arrivalTime;
             }

             return a.id < b.id;
         });

    queue<int> readyQueue;

    vector<Execution> execution;

    int currentTime = 0;
    int nextProcess = 0;
    int completedCount = 0;

    while (completedCount <
           static_cast<int>(processes.size())) {

        // ----------------------------------------------------
        // Add all processes that have arrived.
        // ----------------------------------------------------
        while (nextProcess <
               static_cast<int>(processes.size()) &&
               processes[nextProcess].arrivalTime <= currentTime) {

            readyQueue.push(nextProcess);
            nextProcess++;
        }

        // ----------------------------------------------------
        // No process is ready.
        // CPU remains idle until next arrival.
        // ----------------------------------------------------
        if (readyQueue.empty()) {

            if (nextProcess <
                static_cast<int>(processes.size())) {

                int nextArrival =
                    processes[nextProcess].arrivalTime;

                if (currentTime < nextArrival) {

                    execution.push_back({
                        -1,
                        currentTime,
                        nextArrival
                    });

                    currentTime = nextArrival;
                }

                continue;
            }
        }

        // Get process from front of FIFO queue.
        int index = readyQueue.front();
        readyQueue.pop();

        RRProcess& p = processes[index];

        int startTime = currentTime;

        int executionTime =
            min(timeQuantum, p.remainingTime);

        currentTime += executionTime;

        p.remainingTime -= executionTime;

        // Record EVERY Round Robin execution slice.
        execution.push_back({
            p.id,
            startTime,
            currentTime
        });

        // ----------------------------------------------------
        // Add processes that arrived while this process ran.
        // ----------------------------------------------------
        while (nextProcess <
               static_cast<int>(processes.size()) &&
               processes[nextProcess].arrivalTime <= currentTime) {

            readyQueue.push(nextProcess);
            nextProcess++;
        }

        // ----------------------------------------------------
        // If process still has work, place it at back.
        // ----------------------------------------------------
        if (p.remainingTime > 0) {

            readyQueue.push(index);

        } else {

            completedCount++;
        }
    }

    return execution;
}

// ============================================================
// MAIN
// ============================================================

int main() {

    cout << "===============================================\n";
    cout << "       PRIORITY AND ROUND ROBIN SCHEDULING\n";
    cout << "===============================================\n";

    int n;

    // --------------------------------------------------------
    // 1. Accept number of processes
    // --------------------------------------------------------
    cout << "\nEnter number of processes: ";
    cin >> n;

    if (cin.fail() || n <= 0) {

        cerr << "ERROR: Number of processes must be "
             << "a positive integer.\n";

        return 1;
    }

    vector<Process> processes;

    // --------------------------------------------------------
    // Accept process details
    // --------------------------------------------------------
    for (int i = 0; i < n; i++) {

        Process p;

        cout << "\nProcess " << i + 1 << '\n';

        cout << "Enter Process ID: ";
        cin >> p.id;

        cout << "Enter Arrival Time: ";
        cin >> p.arrivalTime;

        cout << "Enter Burst Time: ";
        cin >> p.burstTime;

        cout << "Enter Priority: ";
        cin >> p.priority;

        // Check numeric input.
        if (cin.fail()) {

            cerr << "ERROR: Invalid input. "
                 << "Please enter integers only.\n";

            return 1;
        }

        // Validate all process values.
        if (!isValidProcess(p)) {

            cerr << "ERROR: Invalid process data.\n";
            cerr << "Rules:\n";
            cerr << "- Process ID must be > 0\n";
            cerr << "- Arrival Time must be >= 0\n";
            cerr << "- Burst Time must be > 0\n";
            cerr << "- Priority must be > 0\n";

            return 1;
        }

        // Check duplicate PID.
        for (const auto& existing : processes) {

            if (existing.id == p.id) {

                cerr << "ERROR: Duplicate Process ID: "
                     << p.id << '\n';

                return 1;
            }
        }

        processes.push_back(p);
    }

    // --------------------------------------------------------
    // Accept and validate Round Robin quantum
    // --------------------------------------------------------
    int timeQuantum;

    cout << "\nEnter Round Robin Time Quantum: ";
    cin >> timeQuantum;

    if (cin.fail() || timeQuantum <= 0) {

        cerr << "ERROR: Time quantum must be "
             << "a positive integer.\n";

        return 1;
    }

    // --------------------------------------------------------
    // Display input
    // --------------------------------------------------------
    cout << "\n===============================================\n";
    cout << "                 INPUT DATA\n";
    cout << "===============================================\n";

    displayProcesses(processes);

    cout << "\nPriority Convention:\n";
    cout << "Smaller priority number = Higher priority\n";

    cout << "Priority tie-breaking:\n";
    cout << "Arrival Time -> Process ID\n";

    cout << "\nRound Robin Time Quantum: "
         << timeQuantum << '\n';

    cout << "Round Robin Ready Queue: FIFO\n";

    // ========================================================
    // PRIORITY SCHEDULING
    // ========================================================

    cout << "\n===============================================\n";
    cout << "          NON-PREEMPTIVE PRIORITY\n";
    cout << "===============================================\n";

    vector<Execution> priorityResult =
        priorityScheduling(processes);

    displaySequence(priorityResult);
    displayExecution(priorityResult);

    // ========================================================
    // ROUND ROBIN
    // ========================================================

    cout << "\n===============================================\n";
    cout << "             ROUND ROBIN\n";
    cout << "===============================================\n";

    vector<Execution> rrResult =
        roundRobin(processes, timeQuantum);

    displaySequence(rrResult);
    displayExecution(rrResult);

    // ========================================================
    // FINAL EVIDENCE
    // ========================================================

    cout << "\n===============================================\n";
    cout << "                 EVIDENCE\n";
    cout << "===============================================\n";

    cout << "Priority convention : "
         << "Lower number = Higher priority\n";

    cout << "Priority scheduling : "
         << "Non-preemptive\n";

    cout << "RR ready queue       : "
         << "FIFO\n";

    cout << "RR time quantum      : "
         << timeQuantum << '\n';

    cout << "RR execution slices  : "
         << "Recorded\n";

    cout << "CPU idle intervals   : "
         << "Displayed where applicable\n";

    cout << "Original data        : "
         << "Preserved\n";

    cout << "\nProgram completed successfully.\n";

    return 0;
}