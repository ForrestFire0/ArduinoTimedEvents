#ifndef MAX_TASKS
#define MAX_TASKS 16
#endif

typedef unsigned long (*TaskFunction) ();
typedef struct Task {
    TaskFunction taskFunction;
    unsigned long nextCall; //If this is 0, then remove it no matter what.
    boolean repeat;
    unsigned int interval;
} Task;

Task *queue[MAX_TASKS];

void setTask(TaskFunction f, unsigned long nextCall, boolean repeat, unsigned int interval) {
    for (byte i = 0; i < MAX_TASKS; i++) {
        if (queue[i] == nullptr) {//This slot is empty
            queue[i] = new Task {f, nextCall, repeat, interval};
            return;
        }
    }
    Serial.print(F("\nIncrease MAX_TASKS\n"));
}

void setTimeout(TaskFunction f, unsigned long timeout) {
    setTask(f, millis() + timeout, false, 0);
}

void setInterval(TaskFunction f, unsigned long start, unsigned int interval) {
    setTask(f, start + interval, true, interval);
}

void setInterval(TaskFunction f, unsigned int interval) {
    setInterval(f, millis(), interval);
}


void runTasks() {
    //    Serial.println(length);
    for (byte i = 0; i < MAX_TASKS; i++) {
        //                Serial.println((uint16_t) queue[i].taskFunction, HEX);
//        Serial.print(i);
//        Serial.print(": ");
//        Serial.println((int) queue[i], HEX);
        if (queue[i]->taskFunction != NULL and queue[i]->nextCall < millis() and queue[i]->nextCall != 0) {
            unsigned long r = queue[i]->taskFunction();
            if (queue[i]->repeat) //Repeating task, add interval to next call.
                queue[i]->nextCall += queue[i]->interval;
            else if (r != 0) //Queue function returned next call value. Set it.
                queue[i]->nextCall = r;
            else { //Queue function did not return next call value. Free memory.
                delete queue[i];
                queue[i] = nullptr;
            }
        }
    }
//    Serial.println();
}
