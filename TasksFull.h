/**
 * This library is the same as the TasksSmall except it allows for capturing lambdas, as well as pure function pointers.
*/

#ifndef MAX_TASKS
#define MAX_TASKS 16
#endif

#include "Arduino.h"

typedef void (*TaskFunctionFuncPtr)();
typedef void (*TaskFunctionLambda)(void *);

typedef struct TaskFunctionLambdaStorage
{
    TaskFunctionLambda lambda;
    long lambdaAddress;
} TaskFunctionLambdaStorage;

typedef union TaskFunction
{
    TaskFunctionLambdaStorage *lambda;
    TaskFunctionFuncPtr funcPtr;
} TaskFunction;

template <typename T>
void lambda_ptr_exec(T *v)
{
    return (void)(*v)();
}

template <typename T>
TaskFunctionLambda lambda_ptr(__attribute__((unused)) T &v)
{
    return (TaskFunctionLambda)lambda_ptr_exec<T>;
}

typedef struct Task
{
    TaskFunction *taskFunction;
    bool lambda;
    unsigned long nextCall; //If this is 0, then remove it no matter what.
    bool repeat;
    unsigned int interval;
} Task;

Task *queue[MAX_TASKS];

byte setTask(TaskFunction *f, bool lambda, unsigned long nextCall, bool repeat, unsigned int interval)
{
    for (byte i = 0; i < MAX_TASKS; i++)
    {
        if (queue[i] == nullptr)
        { //This slot is empty
            queue[i] = new Task{f, lambda, nextCall, repeat, interval};
            return i;
        }
    }
    Serial.print(F("Inc MAX_TASKS\n"));
    return -1;
}

//Basic timout given simple function pointer
byte setTimeout(TaskFunctionFuncPtr f, unsigned long timeout)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    return setTask(taskFunction, false, millis() + timeout, false, 0);
}

//Complex timout given lambda object as type T
template <typename T>
byte setTimeout(T lambda, unsigned long timeout)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    return setTask(taskFunction, true, millis() + timeout, false, 0);
}

//Complex timeout given the lambda as a pointer.
template <typename T>
byte setTimeout(T * lambda, unsigned long timeout)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(*lambda);
    return setTask(taskFunction, true, millis() + timeout, false, 0);
}


template <typename T>
TaskFunction *taskFunctionFromLambda(T lambda)
{
    TaskFunctionLambda fp = lambda_ptr(lambda);
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->lambda = new TaskFunctionLambdaStorage;
    taskFunction->lambda->lambda = fp;
    taskFunction->lambda->lambdaAddress = (long)&lambda;
    Serial.print("Saving lambda with address ");
    Serial.println(taskFunction->lambda->lambdaAddress);
    return taskFunction;
}

//Basic interval with function pointer.
byte setInterval(TaskFunctionFuncPtr f, unsigned long start, unsigned int interval)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    return setTask(taskFunction, false, start + interval, true, interval);
}

//Complex interval given the lambda
template <typename T>
byte setInterval(T lambda, unsigned long start, unsigned long interval)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    return setTask(taskFunction, true, start + interval, true, interval);
}

//Catch all for when we just get the interval
template <typename T>
byte setInterval(T lambda, unsigned int interval)
{
    return setInterval(lambda, millis(), interval);
}

void deleteTask(byte i)
{
    if (queue[i]->lambda)
    { //If we are storing a lambda we need to free the pointer to the TaskFunctionLambdaStorage
        delete queue[i]->taskFunction->lambda;
    }
    //Either way we need to delete task function itself.
    delete queue[i]->taskFunction;

    //And then lastly of course we need to destroy the struct itself.
    delete queue[i];
    queue[i] = nullptr;
}

void runTasks()
{
    //    Serial.println(length);
    for (byte i = 0; i < MAX_TASKS; i++)
    {
        //        Serial.println((uint16_t) queue[i].taskFunction, HEX);
        //        Serial.print(i);
        //        Serial.print(": ");
        //        Serial.println((int) queue[i], HEX);
        if (queue[i]->taskFunction != NULL && queue[i]->nextCall <= millis())
        {
            if (queue[i]->lambda)
            {
                // Hard bit. Do complicated bits.
                queue[i]->taskFunction->lambda->lambda((void *)queue[i]->taskFunction->lambda->lambdaAddress);
                Serial.print("Running lambda at address ");
                Serial.println(queue[i]->taskFunction->lambda->lambdaAddress);
            }
            else
            {
                //Easy one. Just simply run the function.
                queue[i]->taskFunction->funcPtr();
            }
            if (queue[i]->repeat) //Repeating task, add interval to next call.
                queue[i]->nextCall += queue[i]->interval;
            else
            { // Free memory. Needs to be updated to delete every nested thingy.
                deleteTask(i);
            }
        }
    }
    //    Serial.println();
}
