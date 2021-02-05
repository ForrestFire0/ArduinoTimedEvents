/**
 * This library is the same as the TasksSmall except it allows for capturing lambdas, as well as pure function pointers.
*/

#ifndef MAX_TASKS
#define MAX_TASKS 16
#endif

#include "Arduino.h"

typedef void (*TaskFunctionFuncPtr)();
typedef void (*TaskFunctionLambda)(void *);
typedef bool (*ConditionFuncPtr)();

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

typedef union Condition
{
    unsigned long nextCall;
    struct Interval
    {
        unsigned long nextCall;
        unsigned int interval;
    } interval;
    ConditionFuncPtr conditionFuncPtr;
} Condition;

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

typedef struct Task
{
    TaskFunction *taskFunction;
    bool functionType; //false for function pointer, true for lambda pointer
    Condition condition;
    uint8_t conditionType; //1 for interval, 0 for timeout, 2 for Condition function
} Task;

Task *queue[MAX_TASKS];

uint8_t setTask(TaskFunction *f, bool functionType, Condition c, uint8_t conditionType)
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)
    {
        if (queue[i] == nullptr)
        { //This slot is empty
            queue[i] = new Task{f, functionType, c, conditionType};
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
    Condition condition = {(unsigned long)millis() + timeout};
    return setTask(taskFunction, false, condition, 0);
}

//Complex timeout given the lambda as a pointer.
template <typename T>
uint8_t setTimeout(T *lambda, unsigned long timeout)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    Condition condition = {(unsigned long)millis() + timeout};
    return setTask(taskFunction, true, condition, 0);
}

uint8_t addEventListener(TaskFunctionFuncPtr f, ConditionFuncPtr c)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    Condition condition;
    condition.conditionFuncPtr = c;
    return setTask(taskFunction, false, condition, 2);
}

//Basic interval with function pointer.
uint8_t setInterval(TaskFunctionFuncPtr f, unsigned long start, unsigned int interval)
{
    TaskFunction *taskFunction = new TaskFunction;
    taskFunction->funcPtr = f;
    Condition condition;
    condition.interval = {start, interval};
    return setTask(taskFunction, false, condition, 1);
}

//Complex interval given the lambda
template <typename T>
uint8_t setInterval(T *lambda, unsigned long start, unsigned long interval)
{
    TaskFunction *taskFunction = taskFunctionFromLambda(lambda);
    Condition condition;
    condition.interval = {start, interval};
    return setTask(taskFunction, true, condition, 1);
}

//For when we just get the interval
template <typename T>
uint8_t setInterval(T *lambda, unsigned int interval)
{
    return setInterval(lambda, millis()+interval, interval);
}

//For when we just get the interval
uint8_t setInterval(TaskFunctionFuncPtr f, unsigned int interval)
{
    return setInterval(f, millis()+interval, interval);
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
    if (queue[i]->functionType)
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
        if (queue[i] != nullptr)
        {
            // delay(100);
            // Serial.print(i);
            // Serial.print(": ");
            // Serial.println((long)queue[i], HEX);
            bool run = false;
            switch (queue[i]->conditionType)
            {
            case 0:
                if (queue[i]->condition.nextCall <= millis())
                    run = true;
                break;
            case 1:
                if (queue[i]->condition.interval.nextCall <= millis())
                    run = true;
                break;
            case 2:
                run = queue[i]->condition.conditionFuncPtr();
                break;
            }
            if (run)
            {
                if (queue[i]->functionType)
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
                switch (queue[i]->conditionType)
                {
                case 1: //Repeating task, add interval to next call.
                    queue[i]->condition.interval.nextCall += queue[i]->condition.interval.interval;
                    break;
                case 0: // Free memory. Needs to be updated to delete every nested thingy.
                    deleteTask(i);
                    break;
                }
            }
        }
    }
}
