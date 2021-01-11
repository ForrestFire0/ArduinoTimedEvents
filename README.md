# Arduino Timed Events
 This small Arduino file implements setTimeout and setInterval.
 
 To use it
 1. Define MAX_TASKS to the maximum number of tasks that you expect to request. An task only takes 1 byte of memory because it's a pointer. I usually use an absurd number.
 1. Include tasks.h
 ```cpp
#define MAX_TASKS 24
#include "Tasks.h"
 ```
Then use the methods setTimeout and setInterval.

*Note: TaskFunction is simply a typedef for a function that returns void.*
```cpp
typedef void (*TaskFunction) ();
```

Calls the function f timeout milliseconds after the current time.
```cpp
void setTimeout(TaskFunction f, unsigned long timeout)
```
Calls the function f every interval milliseconds. Runs for the first time after the current time plus interval.
```cpp
void setInterval(TaskFunction f, unsigned int interval)
```
Calls the function f every interval milliseconds. Runs for the first time after the start plus interval.
```cpp
void setInterval(TaskFunction f, unsigned long start, unsigned int interval)
```
Here is some example code on how to use it. You can pass in fuctions or lamdas.
```cpp
#include <MemoryFree.h>

#define MAX_TASKS 80
#include <Tasks.h>

void fm() {
    Serial.print("Free Memory = ");
    Serial.println(freeMemory());
    Serial.print(" @ ");
    Serial.println(millis());
}

void setup ()
{
    Serial.begin(115200);
    Serial.print("Starting @ ");
    Serial.println(millis());
    setInterval(fm, 3000);
}

void loop () {
    if (Serial.available()) {
        while (Serial.available()) Serial.read();
        Serial.print("L");
        setTimeout([]() -> void
        {
            Serial.println("Hello from timout");
        }, 1000);
    }
    runTasks();
}
```
Warning - I have no idea if this works after millis() overflows. After the overflow it should work, but if it happens during the overflow, it might work. The next call variable might oveflow successfully... Just know I haven't tested it.
