#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <fstream>
using namespace std;

typedef struct Partition {
    int id;
    int size;
    int original_size;
    bool allocated;
    int allocated_process_id;
} Partition;

typedef struct Process {
    int id;
    int memory_required;
    bool allocated;
    int allocated_partition_id;
} Process;


// Fragmentation statistics
void CalculateFragmentation(const vector<Partition>& partitions) {
    int total_internal_frag = 0, total_external_frag = 0, free_space = 0;

    for (const auto& part : partitions) {
        if (part.allocated) {
            total_internal_frag += part.size;
        } else {
            free_space += part.size;
        }
    }

    total_external_frag = (free_space > 0) ? free_space : 0;

    cout << "\n================ FRAGMENTATION STATISTICS ============\n";
    cout << "Total Internal Fragmentation: " << total_internal_frag << " units\n";
    cout << "Total External Fragmentation: " << total_external_frag << " units\n";
    cout << "=====================================================\n";
}
// First Fit allocation algorithm
void FirstFit(vector<Partition>& partitions, vector<Process>& processes) {
    for (auto& proc : processes) {
        for (auto& part : partitions) {
            if (!part.allocated && proc.memory_required <= part.size) {
                proc.allocated = true;
                proc.allocated_partition_id = part.id;

                part.allocated = true;
                part.allocated_process_id = proc.id;
                part.size -= proc.memory_required;
                break;
            }
        }
    }
}

// Best Fit allocation algorithm
void BestFit(vector<Partition>& partitions, vector<Process>& processes) {
    for (auto& proc : processes) {
        int best_idx = -1;
        for (int i = 0; i < partitions.size(); i++) {
            if (!partitions[i].allocated && proc.memory_required <= partitions[i].size) {
                if (best_idx == -1 || partitions[i].size < partitions[best_idx].size) {
                    best_idx = i;
                }
            }
        }
        if (best_idx != -1) {
            proc.allocated = true;
            proc.allocated_partition_id = partitions[best_idx].id;

            partitions[best_idx].allocated = true;
            partitions[best_idx].allocated_process_id = proc.id;
            partitions[best_idx].size -= proc.memory_required;
        }
    }
}

// Worst Fit allocation algorithm
void WorstFit(vector<Partition>& partitions, vector<Process>& processes) {
    for (auto& proc : processes) {
        int worst_idx = -1;
        for (int i = 0; i < partitions.size(); i++) {
            if (!partitions[i].allocated && proc.memory_required <= partitions[i].size) {
                if (worst_idx == -1 || partitions[i].size > partitions[worst_idx].size) {
                    worst_idx = i;
                }
            }
        }
        if (worst_idx != -1) {
            proc.allocated = true;
            proc.allocated_partition_id = partitions[worst_idx].id;

            partitions[worst_idx].allocated = true;
            partitions[worst_idx].allocated_process_id = proc.id;
            partitions[worst_idx].size -= proc.memory_required;
        }
    }
}

// Next Fit allocation algorithm
void NextFit(vector<Partition>& partitions, vector<Process>& processes) {
    int last_allocated_idx = 0;
    for (auto& proc : processes) {
        for (int i = 0; i < partitions.size(); i++) {
            int idx = (last_allocated_idx + i) % partitions.size();
            if (!partitions[idx].allocated && proc.memory_required <= partitions[idx].size) {
                proc.allocated = true;
                proc.allocated_partition_id = partitions[idx].id;

                partitions[idx].allocated = true;
                partitions[idx].allocated_process_id = proc.id;
                partitions[idx].size -= proc.memory_required;

                last_allocated_idx = idx;
                break;
            }
        }
    }
}

// Deallocate a process by its ID
void DeallocateProcess(vector<Partition>& partitions, vector<Process>& processes, int process_id) {
    for (auto& part : partitions) {
        if (part.allocated && part.allocated_process_id == process_id) {
            part.allocated = false;
            part.size += processes[process_id - 1].memory_required;
            part.allocated_process_id = -1;

            processes[process_id - 1].allocated = false;
            processes[process_id - 1].allocated_partition_id = -1;

            cout << "Process " << process_id << " deallocated.\n";
            return;
        }
    }
    cout << "Process " << process_id << " not found in any partition.\n";
}

// Compact memory (merge all free partitions)
void CompactMemory(vector<Partition>& partitions) {
    int free_space = 0;

    for (auto& part : partitions) {
        if (!part.allocated) {
            free_space += part.size;
            part.size = 0;
        }
    }

    if (free_space > 0) {
        partitions.push_back({(int)partitions.size() + 1, free_space, free_space, false, -1});
    }

    partitions.erase(remove_if(partitions.begin(), partitions.end(),
                               [](const Partition& p) { return p.size == 0 && !p.allocated; }),
                     partitions.end());

    cout << "\nMemory Compacted. All free spaces merged into a single block.\n";
}


// Function to visualize memory partitions
void VisualizeMemory(const vector<Partition>& partitions) {
    cout << "\n================ MEMORY VISUALIZATION ================\n";

    // Top border
    cout << "+";
    for (const auto& part : partitions) {
        cout << string(13, '-') << "+";
    }
    cout << endl;

    // Partition allocation status
    cout << "|";
    for (const auto& part : partitions) {
        if (part.allocated) {
            cout << setw(12) << left << ("P" + to_string(part.allocated_process_id)) << "|";
        } else {
            cout << setw(12) << left << "Free" << "|";
        }
    }
    cout << endl;

    // Partition sizes
    cout << "|";
    for (const auto& part : partitions) {
        cout << setw(12) << left << (to_string(part.size) + "u") << "|";
    }
    cout << endl;

    // Bottom border
    cout << "+";
    for (const auto& part : partitions) {
        cout << string(13, '-') << "+";
    }
    cout << endl;

    cout << "=====================================================\n";
}

// Function to print the current memory status
void PrintStatus(const vector<Partition>& partitions, const vector<Process>& processes) {
    cout << "\n================ CURRENT MEMORY STATUS ================\n";

    cout << left << setw(15) << "Partition ID"
         << setw(15) << "Size"
         << setw(15) << "Allocated"
         << setw(15) << "Process ID" << endl;
    cout << string(60, '-') << endl;

    for (const auto& part : partitions) {
        cout << left << setw(15) << part.id
             << setw(15) << part.size
             << setw(15) << (part.allocated ? "Yes" : "No")
             << setw(15) << (part.allocated ? to_string(part.allocated_process_id) : "-") << endl;
    }

    cout << "\n================ PROCESS ALLOCATION STATUS ============\n";

    cout << left << setw(15) << "Process ID"
         << setw(20) << "Memory Required"
         << setw(15) << "Allocated"
         << setw(15) << "Partition ID" << endl;
    cout << string(65, '-') << endl;

    for (const auto& proc : processes) {
        cout << left << setw(15) << proc.id
             << setw(20) << proc.memory_required
             << setw(15) << (proc.allocated ? "Yes" : "No")
             << setw(15) << (proc.allocated ? to_string(proc.allocated_partition_id) : "-") << endl;
    }

    cout << string(60, '=') << "\n";
}

// Add the visualization call to all relevant operations in the menu.
int main() {
    int num_partitions, num_processes;
    cout << "Enter number of partitions: ";
    cin >> num_partitions;

    vector<Partition> partitions(num_partitions);
    for (int i = 0; i < num_partitions; i++) {
        cout << "Enter size of partition " << i + 1 << ": ";
        cin >> partitions[i].size;
        partitions[i].id = i + 1;
        partitions[i].original_size = partitions[i].size;
        partitions[i].allocated = false;
        partitions[i].allocated_process_id = -1;
    }

    cout << "Enter number of processes: ";
    cin >> num_processes;

    vector<Process> processes(num_processes);
    for (int i = 0; i < num_processes; i++) {
        cout << "Enter memory required for process " << i + 1 << ": ";
        cin >> processes[i].memory_required;
        processes[i].id = i + 1;
        processes[i].allocated = false;
        processes[i].allocated_partition_id = -1;
    }

    char choice;
    do {
        cout << "\n==================== MENU ====================\n";
        cout << "1. Allocate Memory (Choose Algorithm)\n";
        cout << "2. Deallocate Process\n";
        cout << "3. Compact Memory\n";
        cout << "4. Show Fragmentation Statistics\n";
        cout << "5. Print Memory Status\n";
        cout << "6. Visualize Memory\n";
        cout << "7. Exit\n";
        cout << "=============================================\n";
        cout << "Enter your choice: ";
        cin >> choice;
        switch (choice) {
            case '1': {
                char algo_choice;
                cout << "Choose Allocation Algorithm: (F)irst Fit, (B)est Fit, (W)orst Fit, (N)ext Fit: ";
                cin >> algo_choice;
                switch (algo_choice) {
                    case 'F': FirstFit(partitions, processes); break;
                    case 'B': BestFit(partitions, processes); break;
                    case 'W': WorstFit(partitions, processes); break;
                    case 'N': NextFit(partitions, processes); break;
                    default: cout << "Invalid algorithm choice!\n";
                }
                break;
            }
            case '2': {
                int process_id;
                cout << "Enter Process ID to deallocate: ";
                cin >> process_id;
                DeallocateProcess(partitions, processes, process_id);
                break;
            }
            case '3':
                CompactMemory(partitions);
                break;
            case '4':
                CalculateFragmentation(partitions);
                break;
            case '5':
                PrintStatus(partitions, processes);
                break;
            case '6':
                VisualizeMemory(partitions);
                break;
            case '7':
                cout << "Exiting program.\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    } while (choice != '7');

    return 0;
}
