#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define UNUSED "Unused"
#define command_size 128

// 块的最后地址（检查是否越界）
int last_address_space;

// 虚拟头节点
struct Node *head;

// 初始化的整个块
struct Node *total;

// 临时节点用以遍历
struct Node *tmp;
struct Node
/*链表存储每一块的信息，便于插入修改*/
{
    int available_space;
    int start_address;
    int end_address;
    struct Node *next;
    char process_id[100];
};

// 创建空块（未使用），输入参数为大小和起点地址
void creat_unused(int space_requested, int left_over_space) {
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));

    strcpy(newNode->process_id, UNUSED);
    newNode->available_space = left_over_space;
    tmp->next->available_space = space_requested;

    newNode->start_address = tmp->next->end_address + 1;
    newNode->end_address = newNode->start_address + left_over_space;

    // 不能超过最大地址位置
    if (newNode->end_address > last_address_space) {
        newNode->end_address = last_address_space;
    }
    newNode->next = tmp->next->next;
    tmp->next->next = newNode;

    // edit addresses of nodes after created node
    // check if created node is not initial block of memory
    if (tmp->next->next->next != NULL) {
        // check if node after created node have same start address as end
        // address of created node
        if (tmp->next->next->end_address ==
            tmp->next->next->next->start_address) {
            // point tmp to created node
            tmp = tmp->next->next;
            // change all addresses until reach initial block of memory
            while (tmp->next->next != NULL) {
                tmp->next->start_address = tmp->end_address + 1;
                tmp->next->end_address =
                    tmp->next->start_address + tmp->next->available_space;
                tmp = tmp->next;
            }
            // change start address of initial block of memory
            tmp->next->start_address = tmp->end_address + 1;
        }
    }
}

// 相邻空块合并，需要循环merge直到没有相邻的块为空
void merge() {
    struct Node *node_to_remove;

    if (strcmp(tmp->process_id, UNUSED) == 0) {
        tmp->end_address = tmp->next->end_address;
        tmp->available_space += tmp->next->available_space;
        node_to_remove = tmp->next;
        tmp->next = tmp->next->next;
        free(node_to_remove);
    } else if (tmp->next->next != NULL) {

        if (strcmp(tmp->next->next->process_id, UNUSED) == 0) {

            tmp->next->next->start_address = tmp->next->start_address;
            tmp->next->next->available_space += tmp->next->available_space;
            node_to_remove = tmp->next;
            tmp->next = tmp->next->next;
            free(node_to_remove);
        }
    }
    if (strcmp(tmp->process_id, UNUSED) == 0 &&
        strcmp(tmp->next->process_id, UNUSED) == 0) {
        tmp->end_address = tmp->next->end_address;
        tmp->available_space += tmp->next->available_space;
        node_to_remove = tmp->next;
        tmp->next = tmp->next->next;
        free(node_to_remove);
    }
}

// 交换两节点位置函数，可用于整合碎片
void swap_nodes() {
    struct Node *newNode;

    tmp->next->next->start_address = tmp->next->start_address;
    tmp->next->next->end_address =
        tmp->next->next->start_address + tmp->next->next->available_space;

    tmp->next->start_address = tmp->next->next->end_address + 1;
    tmp->next->end_address =
        tmp->next->start_address + tmp->next->available_space;

    newNode = tmp->next->next;
    tmp->next->next = tmp->next->next->next;
    newNode->next = tmp->next;
    tmp->next = newNode;
}

// 查找对应process是否已经存在，若存在则赋值给tmp节点
int find_process(char process_id[3]) {
    int found = 0;
    tmp = head;

    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, process_id) == 0) {
            found = 1;
            break;
        }

        tmp = tmp->next;
    }

    return found;
}

// FIRST_FIT算法找第一个合适的空间
void first_fit(char process_id[3], int space_requested) {
    tmp = head;
    int left_over_space = 0;

    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, UNUSED) == 0 &&
            tmp->next->available_space >= space_requested) {

            head->available_space = head->available_space - space_requested;

            strcpy(tmp->next->process_id, process_id);
            tmp->next->end_address = tmp->next->start_address + space_requested;

            left_over_space = tmp->next->available_space - space_requested;
            if (left_over_space > 0) {
                creat_unused(space_requested, left_over_space);
            }

            return;
        } else {
            tmp = tmp->next;
        }
    }
    printf("为进程%s申请内存失败\n", process_id);
}

// BEST_FIT算法找符合条件且大小最接近的空间
void best_fit(char process_id[3], int space_requested) {
    int smallest_space = __INT_MAX__;
    int left_over_space = 0;
    tmp = head;

    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, UNUSED) == 0 &&
            tmp->next->available_space >= space_requested) {
            if (tmp->next->available_space <= smallest_space) {
                smallest_space = tmp->next->available_space;
            }
        }

        tmp = tmp->next;
    }

    tmp = head;
    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, UNUSED) == 0 &&
            tmp->next->available_space == smallest_space) {
            head->available_space = head->available_space - space_requested;

            strcpy(tmp->next->process_id, process_id);
            tmp->next->end_address = tmp->next->start_address + space_requested;

            left_over_space = tmp->next->available_space - space_requested;
            if (left_over_space > 0) {
                creat_unused(space_requested, left_over_space);
            }

            return;
        } else {
            tmp = tmp->next;
        }
    }

    printf("为进程%s申请内存失败\n", process_id);
}

// WROST_FIT找最大空间
void worst_fit(char process_id[3], int space_requested) {
    int largest_space = -__INT_MAX__;
    int left_over_space = 0;
    tmp = head;

    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, UNUSED) == 0 &&
            tmp->next->available_space >= space_requested) {
            if (tmp->next->available_space >= largest_space) {
                largest_space = tmp->next->available_space;
            }
        }

        tmp = tmp->next;
    }

    tmp = head;
    while (tmp->next != NULL) {
        if (strcmp(tmp->next->process_id, UNUSED) == 0 &&
            tmp->next->available_space == largest_space) {
            head->available_space = head->available_space - space_requested;

            strcpy(tmp->next->process_id, process_id);
            tmp->next->end_address = tmp->next->start_address + space_requested;

            left_over_space = tmp->next->available_space - space_requested;
            if (left_over_space > 0) {
                creat_unused(space_requested, left_over_space);
            }

            return;
        } else {
            tmp = tmp->next;
        }
    }

    printf("为进程%s申请内存失败\n", process_id);
}

// RQ命令处理，根据情况调用三种算法，需强制选择一种算法。
void request_memory(char process_id[3], int space_requested, char mode[2]) {
    if (find_process(process_id) == 1) {
        printf("进程%s已存在\n", process_id);
    } else if (strcmp("W", mode) == 0) {
        worst_fit(process_id, space_requested);
    } else if (strcmp("B", mode) == 0) {
        best_fit(process_id, space_requested);
    } else if (strcmp("F", mode) == 0) {
        first_fit(process_id, space_requested);
    } else {
        printf("未选择三种算法之一,默认使用first_fit算法\n");
        first_fit(process_id, space_requested);
        return;
    }
}

// 释放process对应空间
void release_memory(char process_id[3]) {
    // 先找到
    if (find_process(process_id) == 1) {
        // 剩余总大小
        head->available_space =
            head->available_space + tmp->next->available_space;

        // 标记为Unused即为空块（以释放）
        strcpy(tmp->next->process_id, UNUSED);

        // 连接左右空块
        merge();
        puts("succeed");
        return;
    }
    // 没找到
    printf("未找到对应进程%s\n", process_id);
}

// defrag过程，把空块连接
void compact() {
    tmp = head;

    while (tmp->next != NULL) {
        if ((strcmp(tmp->next->process_id, UNUSED) == 0 &&
             tmp->next->next != NULL) ||
            (strcmp(tmp->next->process_id, UNUSED) != 0 &&
             strcmp(tmp->process_id, UNUSED) == 0)) {

            // 交换节点，把空的块一直往后移动
            swap_nodes();
            tmp = tmp->next;
            // 相邻的要merge
            merge();
        } else {
            tmp = tmp->next;
        }
    }
    puts("succeed");
}

// 内存块报表
void status_report() {
    tmp = head;
    while (tmp->next != NULL) {
        printf("Addresses [%d : %d] Process %s\n", tmp->next->start_address,
               tmp->next->end_address, tmp->next->process_id);
        tmp = tmp->next;
    }
}

// 退出，释放所有内存并结束
void prepare_to_exit() {
    puts("exiting...");
    tmp = head;
    struct Node *node_to_remove;

    while (tmp->next != NULL) {
        node_to_remove = tmp->next;
        tmp->next = tmp->next->next;
        free(node_to_remove);
    }
    free(head);
}

// 主函数，需要从命令行运行并指定最大内存大小
int main(int argc, char *argv[]) {
    char command_type[5];
    char process[3];
    int space_requested;
    char mode[2];

    int inital_memory = atoi(argv[1]) - 1;

    head = (struct Node *)malloc(sizeof(struct Node));
    total = (struct Node *)malloc(sizeof(struct Node));

    strcpy(head->process_id, "head");
    head->start_address = -1;
    head->end_address = -1;
    head->available_space = inital_memory;
    head->next = total;

    strcpy(total->process_id, UNUSED);
    total->start_address = 0;
    total->end_address = inital_memory;
    total->available_space = total->end_address - total->start_address;
    total->next = NULL;

    last_address_space = inital_memory;
    char input[command_size];

    // 第一次读取，可以用gets，scanf("%[*^/n]")等等读取一行，需要控制命令长度可以用read
    printf("allocator>");
    fflush(stdout);

    if (read(0, input, command_size) < 0) {
        puts("输入错误");
    }
    while (input[0] != 'X') {
        // 输入
        if ((sscanf(input, "%s", command_type)) < 0) {
            puts("输入错误");
        }

        // 退出循环然后退出程序
        if (strcmp("X", command_type) == 0) {
            break;
        }

        // RQ命令
        if (strcmp("RQ", command_type) == 0) {
            sscanf(input, "%s %s %d %s", command_type, process,
                   &space_requested, mode);
            request_memory(process, space_requested, mode);
        }
        // RL命令
        else if (strcmp("RL", command_type) == 0) {
            sscanf(input, "%s %s", command_type, process);
            release_memory(process);
        }
        // C命令
        else if (strcmp("C", command_type) == 0) {
            compact();
        }
        // STAT命令
        else if (strcmp("STAT", command_type) == 0) {
            status_report();
        }
        // 非法命令
        else {
            puts("命令无法识别");
        }

        // 循环读取
        printf("allocator>");
        fflush(stdout);

        if (read(0, input, command_size) < 0) {
            puts("输入错误");
        }
    }
    prepare_to_exit();
    return 0;
}