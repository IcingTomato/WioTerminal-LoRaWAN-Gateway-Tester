#ifndef __SQQUEUE_H__
#define __SQQUEUE_H__

#define MAXSIZE (33)//20 /* 存储空间初始分配量，实际分配量是MAXSIZE-1 */

typedef int Status; 
typedef int QElemType; /* QElemType类型根据实际情况而定，这里假设为int */

/* 循环队列的顺序存储结构 */
typedef struct
{
    QElemType data[MAXSIZE];
    int front;      /* 头指针 */
    int rear;       /* 尾指针，若队列不空，指向队列尾元素的下一个位置 */
}SqQueue;

Status InitQueue(SqQueue *Q);
Status ClearQueue(SqQueue *Q);
Status QueueEmpty(SqQueue Q);
Status QueueFull(SqQueue Q);
int QueueLength(SqQueue Q);
Status GetHead(SqQueue Q,QElemType *e);
Status EnQueue(SqQueue *Q,QElemType e);
Status DeQueue(SqQueue *Q,QElemType *e);
void SqQueueFillData(SqQueue *Q,QElemType Data);
#endif
