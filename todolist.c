// To-Do List App with Advanced Features in C
// Features: Undo, Bulk Delete, Due Dates, Priority, Sorting, Archive, Search, Stats, Fancy Output, Reminders

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GREEN "\033[1;32m"
#define RED   "\033[1;31m"
#define YELLOW "\033[1;33m"
#define BLUE  "\033[1;34m"
#define RESET "\033[0m"

struct Task {
    int id;
    char description[100];
    int completed;
    char dueDate[20]; // Format: YYYY-MM-DD
    int priority;     // 1 = Low, 2 = Medium, 3 = High
    int archived;     // 1 = archived
    struct Task *next;
};

struct Task *head = NULL;
int nextId = 1;

const char *priorityToStr(int p) {
    switch (p) {
        case 1: return "LOW";
        case 2: return "MEDIUM";
        case 3: return "HIGH";
        default: return "UNKNOWN";
    }
}

// Utility to compare dates (returns <0, 0, >0)
int compareDate(const char *d1, const char *d2) {
    return strcmp(d1, d2);
}

void addTask() {
    struct Task *newTask = malloc(sizeof(struct Task));
    getchar();
    printf("Enter task description: ");
    fgets(newTask->description, 100, stdin);
    newTask->description[strcspn(newTask->description, "\n")] = 0;

    printf("Enter due date (YYYY-MM-DD): ");
    fgets(newTask->dueDate, 20, stdin);
    newTask->dueDate[strcspn(newTask->dueDate, "\n")] = 0;

    printf("Enter priority (1=Low, 2=Med, 3=High): ");
    scanf("%d", &newTask->priority);
    getchar();

    newTask->id = nextId++;
    newTask->completed = 0;
    newTask->archived = 0;
    newTask->next = NULL;

    if (!head) head = newTask;
    else {
        struct Task *temp = head;
        while (temp->next) temp = temp->next;
        temp->next = newTask;
    }
    printf(GREEN "Task added successfully!\n" RESET);
}

void viewTasks(int filter) {
    struct Task *temp = head;
    int count = 0;
    printf(BLUE "\n--- TO-DO LIST ---\n" RESET);
    printf("%-4s %-50s %-10s %-12s %-10s\n", "ID", "Task", "Status", "Due Date", "Priority");
    while (temp) {
        if ((filter == 0 || (filter == 1 && !temp->completed) ||
             (filter == 2 && temp->completed) || (filter == 3 && temp->archived))) {
            const char *status = temp->completed ? "Done" : "Pending";
            const char *color = temp->completed ? GREEN : RED;
            printf("%-4d %-50s %s%-10s%s %-12s %-10s\n",
                   temp->id, temp->description,
                   color, status, RESET,
                   temp->dueDate,
                   priorityToStr(temp->priority));
            count++;
        }
        temp = temp->next;
    }
    if (count == 0) printf("No tasks to display.\n");
}

void markCompleted() {
    int id;
    printf("Enter ID to mark complete: ");
    scanf("%d", &id);
    struct Task *temp = head;
    while (temp) {
        if (temp->id == id && !temp->completed) {
            temp->completed = 1;
            printf(GREEN "Marked as done.\n" RESET);
            return;
        }
        temp = temp->next;
    }
    printf("Task not found or already done.\n");
}

void undoCompleted() {
    int id;
    printf("Enter ID to undo: ");
    scanf("%d", &id);
    struct Task *temp = head;
    while (temp) {
        if (temp->id == id && temp->completed) {
            temp->completed = 0;
            printf(YELLOW "Marked as pending again.\n" RESET);
            return;
        }
        temp = temp->next;
    }
    printf("Task not found or not completed.\n");
}

void deleteCompleted() {
    struct Task *temp = head, *prev = NULL;
    while (temp) {
        if (temp->completed && !temp->archived) {
            if (prev) prev->next = temp->next;
            else head = temp->next;
            struct Task *del = temp;
            temp = temp->next;
            free(del);
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
    printf(RED "All completed tasks deleted.\n" RESET);
}

void archiveCompleted() {
    struct Task *temp = head;
    while (temp) {
        if (temp->completed) temp->archived = 1;
        temp = temp->next;
    }
    printf("Completed tasks archived.\n");
}

void showStats() {
    int total = 0, done = 0, pending = 0;
    struct Task *temp = head;
    while (temp) {
        if (!temp->archived) {
            total++;
            if (temp->completed) done++;
            else pending++;
        }
        temp = temp->next;
    }
    printf("\nTotal: %d | Done: %d | Pending: %d\n", total, done, pending);
}

void searchTask() {
    char keyword[100];
    getchar();
    printf("Enter keyword to search: ");
    fgets(keyword, 100, stdin);
    keyword[strcspn(keyword, "\n")] = 0;
    struct Task *temp = head;
    while (temp) {
        if (strstr(temp->description, keyword)) {
            printf("%d - %s\n", temp->id, temp->description);
        }
        temp = temp->next;
    }
}

void checkReminders() {
    time_t t = time(NULL);
    struct tm *today = localtime(&t);
    char current[20];
    strftime(current, sizeof(current), "%Y-%m-%d", today);

    struct Task *temp = head;
    while (temp) {
        if (!temp->completed && compareDate(temp->dueDate, current) < 0) {
            printf(RED "Reminder: Task '%s' is past due!\n" RESET, temp->description);
        }
        temp = temp->next;
    }
}

void saveTasks() {
    FILE *f = fopen("tasks.txt", "w");
    struct Task *temp = head;
    while (temp) {
        fprintf(f, "%d|%d|%s|%s|%d|%d\n",
                temp->id, temp->completed, temp->description,
                temp->dueDate, temp->priority, temp->archived);
        temp = temp->next;
    }
    fclose(f);
}

void loadTasks() {
    FILE *f = fopen("tasks.txt", "r");
    if (!f) return;
    struct Task *newTask;
    while (!feof(f)) {
        newTask = malloc(sizeof(struct Task));
        if (fscanf(f, "%d|%d|%[^|]|%[^|]|%d|%d\n",
                   &newTask->id, &newTask->completed,
                   newTask->description, newTask->dueDate,
                   &newTask->priority, &newTask->archived) == 6) {
            newTask->next = NULL;
            if (newTask->id >= nextId) nextId = newTask->id + 1;
            if (!head) head = newTask;
            else {
                struct Task *temp = head;
                while (temp->next) temp = temp->next;
                temp->next = newTask;
            }
        } else free(newTask);
    }
    fclose(f);
}

void freeAll() {
    struct Task *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    loadTasks();
    checkReminders();
    int choice;
    do {
        printf("\n\n--- MENU ---\n");
        printf("1. Add Task\n2. View All\n3. View Pending\n4. View Completed\n5. Mark Done\n6. Undo Completion\n7. Delete Completed\n8. Archive Completed\n9. Search Task\n10. Stats\n11. View Archived\n12. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: addTask(); break;
            case 2: viewTasks(0); break;
            case 3: viewTasks(1); break;
            case 4: viewTasks(2); break;
            case 5: markCompleted(); break;
            case 6: undoCompleted(); break;
            case 7: deleteCompleted(); break;
            case 8: archiveCompleted(); break;
            case 9: searchTask(); break;
            case 10: showStats(); break;
            case 11: viewTasks(3); break;
            case 12: saveTasks(); freeAll(); printf("Goodbye!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 12);

    return 0;
}
