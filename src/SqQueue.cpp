#include "SqQueue.h"
#include <Arduino.h>

/* 初始化一个空队列Q */
Status InitQueue(SqQueue *Q)
{
    Q->front=0;
    Q->rear=0;
    return  true;
}

/* 将Q清为空队列 */
Status ClearQueue(SqQueue *Q)
{
    Q->front=Q->rear=0;
    return true;
}

/* 若队列Q为空队列,则返回TRUE,否则返回FALSE */
Status QueueEmpty(SqQueue Q)
{ 
    if(Q.front==Q.rear) /* 队列空的标志 */
        return true;
    else
        return false;
}

/* 若队列Q为满队列,则返回TRUE,否则返回FALSE */
Status QueueFull(SqQueue Q)
{ 
    if ((Q.rear+1)%MAXSIZE == Q.front) /* 队列满的标志 */
        return true;
    else
        return false;
}

/* 返回Q的元素个数，也就是队列的当前长度 */
int QueueLength(SqQueue Q)
{
    return  (Q.rear-Q.front+MAXSIZE)%MAXSIZE;
}

/* 若队列不空,则用e返回Q的队头元素,并返回OK,否则返回ERROR */
Status GetHead(SqQueue Q,QElemType *e)
{

    if(Q.front==Q.rear) /* 队列空 */
        return false;
    *e=Q.data[Q.front];
    return true;
}

/* 若队列未满，则插入元素e为Q新的队尾元素 */
Status EnQueue(SqQueue *Q,QElemType e)
{
    if ((Q->rear+1)%MAXSIZE == Q->front)    /* 队列满的判断 */
        return false;
    Q->data[Q->rear]=e;         /* 将元素e赋值给队尾 */
    Q->rear=(Q->rear+1)%MAXSIZE;/* rear指针向后移一位置， */
                                /* 若到最后则转到数组头部 */                               
    return  true;
}

/* 若队列不空，则删除Q中队头元素，用e返回其值 */
Status DeQueue(SqQueue *Q,QElemType *e)
{
    if (Q->front == Q->rear)            /* 队列空的判断 */
        return false;
    *e=Q->data[Q->front];               /* 将队头元素赋值给e */
    Q->front=(Q->front+1)%MAXSIZE;  /* front指针向后移一位置， */
                                    /* 若到最后则转到数组头部 */
    return  true;
}

void SqQueueFillData(SqQueue *Q,QElemType Data)
{
    QElemType d; 
    if(QueueFull(*Q)==true)
    {
        DeQueue(Q,&d);
        EnQueue(Q,Data);
    }
    else
    {
        EnQueue(Q,Data);          
    }
}
