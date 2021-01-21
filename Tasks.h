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
    TaskFunctionLambda deleteMe;
    long lambdaAddress;
} TaskFunctionLambdaStorage;

typedef union TaskFunction
{
    TaskFunctionLambdaStorage *lambda;
    TaskFunctionFuncPtr funcPtr;
} TaskFunction;

// A template function that is used by name to generate function pointers.
template <typename T>
void lambda_ptr_exec(T *v)
{
    return (void)(*v)();
}

// A template function that is is used by name to generate functions that delete the lambda.
template <typename T>
void lambda_ptr_delete(T *v)
{
    delete v;
}

// Function takes a lambda and returns another function that can be called to delete the lambda.

typedef struct Task
{
    TaskFunction *taskFunction;
    bool lambda;
    unsigned long nextCall; //If this is 0, then remove it no matter what.
    bool repeat;
    unsigned int interval;
} Task;

Task *queue[MAX_TASKS];

uint8_t setTask(TaskFunction *f, bool lambda, unsigned long nextCall, bool repeat, unsigned int interval)
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)
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
uint8_t setTimeout(TaskFunctionFuncPtr f, unsigned long timeout)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    return setTask(taskFunction, false, millis() + timeout, false, 0);
}

//Complex timeout given the lambda as a pointer.
template <typename T>
uint8_t setTimeout(T *lambda, unsigned long timeout)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    return setTask(taskFunction, true, millis() + timeout, false, 0);
}

//Basic interval with function pointer.
uint8_t setInterval(TaskFunctionFuncPtr f, unsigned long start, unsigned int interval)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    return setTask(taskFunction, false, start + interval, true, interval);
}

//Complex interval given the lambda
template <typename T>
uint8_t setInterval(T *lambda, unsigned long start, unsigned long interval)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    return setTask(taskFunction, true, start + interval, true, interval);
}

//Catch all for when we just get the interval
template <typename T>
uint8_t setInterval(T *lambda, unsigned int interval)
{
    return setInterval(lambda, millis(), interval);
}

template <typename T>
TaskFunction *taskFunctionFromLambda(T *lambda)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->lambda = new TaskFunctionLambdaStorage;
    taskFunction->lambda->lambda = (TaskFunctionLambda)lambda_ptr_exec<T>;
    taskFunction->lambda->deleteMe = (TaskFunctionLambda)lambda_ptr_delete<T>;
    taskFunction->lambda->lambdaAddress = (long)lambda;
    // Serial.print("Saving lambda with address ");
    // Serial.println(taskFunction->lambda->lambdaAddress);
    return taskFunction;
}

void deleteTask(uint8_t i)
{
    if (queue[i]->lambda)
    {
        //If we are storing a lambda we need to free a lot of things
        //First, the actual lambda. Delete it using the function from before.
        queue[i]->taskFunction->lambda->deleteMe((void *)queue[i]->taskFunction->lambda->lambdaAddress);
        // Next we need to delete the TaskFunctionLambdaStorage object
        delete queue[i]->taskFunction->lambda;
    }
    //Either way we need to delete task function itself.
    delete queue[i]->taskFunction;

    //And then lastly of course we need to destroy the main object itself.
    delete queue[i];
    queue[i] = nullptr;
}

void runTasks()
{
    //    Serial.println(length);
    for (uint8_t i = 0; i < MAX_TASKS; i++)
    {
        // delay(100);
        // Serial.print(i);
        // Serial.print(": ");
        // Serial.println((long)queue[i], HEX);
        if (queue[i] != NULL && queue[i]->nextCall <= millis())
        {
            if (queue[i]->lambda)
            {
                // Hard bit. Do complicated bits.
                queue[i]->taskFunction->lambda->lambda((void *)queue[i]->taskFunction->lambda->lambdaAddress);
                // Serial.print("Running L @ ");
                // Serial.println(queue[i]->taskFunction->lambda->lambdaAddress);
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
}
