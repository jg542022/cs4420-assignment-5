#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 20

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
} Task;

static Task tasks[MAX_TASKS];
static int task_count;
static int ready_queue[MAX_TASKS];
static int ready_count;
static int next_arrival_index;
static int arrival_order[MAX_TASKS];

static void read_tasks(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) { perror(path); exit(1); }
    fscanf(file, "%d", &task_count);
    for (int i = 0; i < task_count; i++) {
        Task *task = &tasks[i];
        fscanf(file, "%d %d %d", &task->pid, &task->arrival_time, &task->burst_time);
        task->remaining_time = task->burst_time;
        task->start_time = -1;
        arrival_order[i] = i;
    }
    fclose(file);
    for (int i = 1; i < task_count; i++) {
        for (int j = i; j > 0 && tasks[arrival_order[j]].arrival_time < tasks[arrival_order[j - 1]].arrival_time; j--) {
            int swap = arrival_order[j];
            arrival_order[j] = arrival_order[j - 1];
            arrival_order[j - 1] = swap;
        }
    }
}

static void enqueue(int task_index) {
    ready_queue[ready_count++] = task_index;
}

static int dequeue_at(int position) {
    int task_index = ready_queue[position];
    for (int i = position; i < ready_count - 1; i++) ready_queue[i] = ready_queue[i + 1];
    ready_count--;
    return task_index;
}

static void admit_arrivals(int now) {
    while (next_arrival_index < task_count && tasks[arrival_order[next_arrival_index]].arrival_time <= now) {
        enqueue(arrival_order[next_arrival_index++]);
    }
}

static int position_of_shortest_job(void) {
    int best = 0;
    for (int i = 1; i < ready_count; i++) {
        if (tasks[ready_queue[i]].burst_time < tasks[ready_queue[best]].burst_time) best = i;
    }
    return best;
}

static void simulate(const char *algorithm, int quantum) {
    int now = 0;
    int finished = 0;
    while (finished < task_count) {
        admit_arrivals(now);
        if (ready_count == 0) {
            printf("t=%d: idle\n", now);
            now++;
            continue;
        }
        int position = strcmp(algorithm, "SJF") == 0 ? position_of_shortest_job() : 0;
        int task_index = dequeue_at(position);
        Task *task = &tasks[task_index];
        int slice = task->remaining_time;
        if (strcmp(algorithm, "RR") == 0 && quantum < slice) slice = quantum;
        if (task->start_time < 0) task->start_time = now;
        printf("t=%d-%d: run pid %d\n", now, now + slice, task->pid);
        now += slice;
        task->remaining_time -= slice;
        admit_arrivals(now);
        if (task->remaining_time == 0) {
            task->end_time = now;
            finished++;
            printf("t=%d: pid %d finished\n", now, task->pid);
        } else {
            enqueue(task_index);
        }
    }
}

static void print_statistics(void) {
    printf("\nPID Arrival Start End Running Waiting\n");
    double total_waiting = 0;
    for (int i = 0; i < task_count; i++) {
        Task *task = &tasks[i];
        int waiting = task->end_time - task->arrival_time - task->burst_time;
        total_waiting += waiting;
        printf("%3d %7d %5d %3d %7d %7d\n", task->pid, task->arrival_time, task->start_time,
               task->end_time, task->burst_time, waiting);
    }
    printf("Average Waiting Time: %g\n", total_waiting / task_count);
}

int main(int argc, char **argv) {
    if (argc < 3 || (strcmp(argv[2], "RR") == 0 && argc < 4)) {
        fprintf(stderr, "Usage: proj2 input_file [FCFS|RR|SJF] [time_quantum]\n");
        return 1;
    }
    read_tasks(argv[1]);
    int quantum = argc > 3 ? atoi(argv[3]) : 0;
    simulate(argv[2], quantum);
    print_statistics();
    return 0;
}
