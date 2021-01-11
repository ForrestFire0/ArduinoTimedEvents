#include <MemoryFree.h>

#define MAX_TASKS 80
#include "Tasks.h"
// Generic task function that is called


unsigned long printSomething() {
    Serial.println("Running funct");
    return 0;
}

unsigned long fm() {
    Serial.print("Free Memory = ");
    Serial.println(freeMemory());
}


void setup ()
{
    Serial.begin(115200);
    Serial.println("Starting");
    //setInterval(fm, 3000);
}

void loop () {
    if (Serial.available()) {
        while (Serial.available()) Serial.read();
        Serial.print("L");
        setTimeout([]() -> unsigned long
        {
            Serial.println("Hello from timout");
            return 0;
        }, 1000);
    }
    runTasks();
}
