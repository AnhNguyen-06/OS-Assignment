#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

int empty(struct queue_t *q)
{
        if (q == NULL)
                return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */
        //kiểm tra hàng đợi nếu đã đầy hoặc đầu vào không đúng
        if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE) return;
        q->proc[q->size] = proc; //thêm tiến trình vào vị trí cuối cùng
        q->size++; //tăng size queue
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (empty(q)) return NULL;
        //tìm tiến trình có priority nhỏ nhất
        int target_idx = 0;
        for (int i=1; i < q->size; i++) {
                if (q->proc[i]->priority < q->proc[target_idx]->priority) {
                        target_idx = i;
                }
        }

        //lưu lại tiến trình đó
        struct pcb_t *proc = q->proc[target_idx];

        //dồn phía sau lên
        for (int i = target_idx; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i+1];
        }
        
        q->size--;
        return proc;
}

struct pcb_t *purgequeue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: remove a specific item from queue
         * */
        if (empty(q) || proc == NULL) return NULL;

        int target_idx = -1;
        for (int i=0; i < q->size; i++) {
                if (q->proc[i] == proc) {
                        target_idx = i;
                        break;
                }
        }

        if (target_idx != -1) {
                for (int i = target_idx; i < q->size - 1; i++) {
                        q->proc[i] = q->proc[i + 1];
                }
                q->size--;
                return proc;
        }
        return NULL;
}