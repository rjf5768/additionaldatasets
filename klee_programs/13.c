#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_PROCESSES 10

// Process structure
struct process {
    int id;
    int arrival_time;
    int burst_time;
    int priority;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int is_completed;
};

// Initialize process
void init_process(struct process* p, int id, int arrival, int burst, int priority) {
    p->id = id;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->priority = priority;
    p->completion_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->is_completed = 0;
}

// Find highest priority process that has arrived
int find_highest_priority_process(struct process* processes, int num_processes, int current_time) {
    int highest_priority = -1;
    int selected_process = -1;
    
    for (int i = 0; i < num_processes; i++) {
        if (!processes[i].is_completed && processes[i].arrival_time <= current_time) {
            if (selected_process == -1 || processes[i].priority < highest_priority) {
                highest_priority = processes[i].priority;
                selected_process = i;
            }
        }
    }
    
    return selected_process;
}

// Non-preemptive priority scheduling
void priority_scheduling(struct process* processes, int num_processes) {
    int current_time = 0;
    int completed_processes = 0;
    
    // Find minimum arrival time
    int min_arrival = processes[0].arrival_time;
    for (int i = 1; i < num_processes; i++) {
        if (processes[i].arrival_time < min_arrival) {
            min_arrival = processes[i].arrival_time;
        }
    }
    current_time = min_arrival;
    
    while (completed_processes < num_processes) {
        int selected = find_highest_priority_process(processes, num_processes, current_time);
        
        if (selected == -1) {
            // No process available, advance time
            current_time++;
            continue;
        }
        
        // Execute the selected process
        processes[selected].completion_time = current_time + processes[selected].burst_time;
        processes[selected].turnaround_time = processes[selected].completion_time - processes[selected].arrival_time;
        processes[selected].waiting_time = processes[selected].turnaround_time - processes[selected].burst_time;
        processes[selected].is_completed = 1;
        
        current_time = processes[selected].completion_time;
        completed_processes++;
    }
}

// Calculate average waiting time
double calculate_average_waiting_time(struct process* processes, int num_processes) {
    double total_waiting_time = 0.0;
    for (int i = 0; i < num_processes; i++) {
        total_waiting_time += processes[i].waiting_time;
    }
    return total_waiting_time / num_processes;
}

// Calculate average turnaround time
double calculate_average_turnaround_time(struct process* processes, int num_processes) {
    double total_turnaround_time = 0.0;
    for (int i = 0; i < num_processes; i++) {
        total_turnaround_time += processes[i].turnaround_time;
    }
    return total_turnaround_time / num_processes;
}

// Check if all processes are completed
int all_processes_completed(struct process* processes, int num_processes) {
    for (int i = 0; i < num_processes; i++) {
        if (!processes[i].is_completed) {
            return 0;
        }
    }
    return 1;
}

// Verify scheduling constraints
int verify_scheduling_constraints(struct process* processes, int num_processes) {
    for (int i = 0; i < num_processes; i++) {
        // Completion time should be >= arrival time + burst time
        if (processes[i].completion_time < processes[i].arrival_time + processes[i].burst_time) {
            return 0;
        }
        
        // Waiting time should be >= 0
        if (processes[i].waiting_time < 0) {
            return 0;
        }
        
        // Turnaround time should be >= burst time
        if (processes[i].turnaround_time < processes[i].burst_time) {
            return 0;
        }
    }
    return 1;
}

// KLEE test function
int main() {
    // Symbolic parameters
    int num_processes;
    klee_make_symbolic(&num_processes, sizeof(num_processes), "num_processes");
    
    // Constrain parameters
    klee_assume(num_processes >= 2 && num_processes <= MAX_PROCESSES);
    
    // Allocate process array
    struct process* processes = malloc(num_processes * sizeof(struct process));
    
    // Initialize processes with symbolic data
    for (int i = 0; i < num_processes; i++) {
        int arrival_time, burst_time, priority;
        
        klee_make_symbolic(&arrival_time, sizeof(arrival_time), "arrival_time");
        klee_make_symbolic(&burst_time, sizeof(burst_time), "burst_time");
        klee_make_symbolic(&priority, sizeof(priority), "priority");
        
        // Constrain process parameters
        klee_assume(arrival_time >= 0 && arrival_time <= 20);
        klee_assume(burst_time >= 1 && burst_time <= 10);
        klee_assume(priority >= 1 && priority <= 10);
        
        init_process(&processes[i], i, arrival_time, burst_time, priority);
    }
    
    // Run priority scheduling
    priority_scheduling(processes, num_processes);
    
    // Verify scheduling results
    klee_assert(all_processes_completed(processes, num_processes));
    klee_assert(verify_scheduling_constraints(processes, num_processes));
    
    // Test individual process properties
    for (int i = 0; i < num_processes; i++) {
        // Process should be completed
        klee_assert(processes[i].is_completed == 1);
        
        // Completion time should be reasonable
        klee_assert(processes[i].completion_time >= processes[i].arrival_time);
        klee_assert(processes[i].completion_time <= 100); // Reasonable upper bound
        
        // Turnaround time calculation
        int expected_turnaround = processes[i].completion_time - processes[i].arrival_time;
        klee_assert(processes[i].turnaround_time == expected_turnaround);
        
        // Waiting time calculation
        int expected_waiting = processes[i].turnaround_time - processes[i].burst_time;
        klee_assert(processes[i].waiting_time == expected_waiting);
    }
    
    // Calculate and verify averages
    double avg_waiting = calculate_average_waiting_time(processes, num_processes);
    double avg_turnaround = calculate_average_turnaround_time(processes, num_processes);
    
    klee_assert(avg_waiting >= 0.0);
    klee_assert(avg_turnaround >= 0.0);
    
    // Test priority ordering (simplified check)
    // Find the process with highest priority (lowest priority number)
    int highest_priority_process = 0;
    for (int i = 1; i < num_processes; i++) {
        if (processes[i].priority < processes[highest_priority_process].priority) {
            highest_priority_process = i;
        }
    }
    
    // The highest priority process should complete before or at the same time as others
    // (This is a simplified check - in reality, arrival times matter)
    for (int i = 0; i < num_processes; i++) {
        if (i != highest_priority_process && 
            processes[i].arrival_time <= processes[highest_priority_process].arrival_time) {
            // If both processes arrive at the same time or earlier, 
            // higher priority should complete first
            if (processes[i].arrival_time == processes[highest_priority_process].arrival_time) {
                klee_assert(processes[highest_priority_process].completion_time <= processes[i].completion_time);
            }
        }
    }
    
    // Test with symbolic query
    int query_process_id;
    klee_make_symbolic(&query_process_id, sizeof(query_process_id), "query_process_id");
    klee_assume(query_process_id >= 0 && query_process_id < num_processes);
    
    // Verify query process properties
    klee_assert(processes[query_process_id].is_completed == 1);
    klee_assert(processes[query_process_id].waiting_time >= 0);
    klee_assert(processes[query_process_id].turnaround_time >= processes[query_process_id].burst_time);
    
    free(processes);
    return 0;
} 