# Arduino Timed Events
This small Arduino file implements setTimeout and setInterval. It's for lazy people like me that don't want to deal with millis but also don't want to use the dreaded delay.

If anyone wants to make it millis-overflow safe please make a pull request.
 
 To use it
 1. Define MAX_TASKS to the maximum number of tasks that you expect to request. An task only takes 2 bytes of memory because it's a pointer. I usually use an absurd number.
 1. Include Tasks.h.
 1. Use methods in your code.
 1. Put "runTasks();" somewhere that will run very often. It should either be the only thing in void loop() or along with some other function that needs to be called extremely often and doesn't take very long; perhaps one looking for input from the Serial monitor. However, your tasks will be put off until you call "runTasks();" again.
 ```cpp
#define MAX_TASKS 24
#include <Tasks.h>
 ```
Then use the methods setTimeout and setInterval.

## What counts as a function I can pass to setTimeout or setInterval?
Parameter-less Function pointers that return void or pointers to parameter-less lambdas that return void.
**Lambdas must be created with new to ensure they are on the HEAP! It is more than likely that lambda will be out of scope by the time it is called.**
Examples:
```cpp
void funct1() {
    Serial.println("inside function 1");
}

void setupAll() {
    setTimeout(funct1, 1000); //Runs funct1 1 second after this is called.
    for (byte i = 1; i <= 5; i++) {
        //Sets five timeouts using capturing lambdas. You must use new to place it on the heap, auto is the type, and it needs to all be in parenthesis. Who knows why.
        //                     \/ This equals means copy all variables that are captured. So each lambda has a copy of 'i' at its current state.
        setTimeout(new auto ([ = ]() -> void {
            Serial.print("This is lambda ");
            Serial.print(i);
            Serial.print(" running at time ");
            Serial.println(millis());
        }), i * 1000); // Runs i seconds after this is called.
    }
}
```

## The full list of functions:
Calls f timeout milliseconds after the current time.
```cpp
uint8_t setTimeout(TaskFunctionFuncPtr f, unsigned long timeout);
/* or */
template <typename T>
uint8_t setTimeout(T *lambda, unsigned long timeout);
```
Calls the function every interval milliseconds. Runs for the first time after the current time plus interval.
```cpp
uint8_t setInterval(TaskFunctionFuncPtr f, unsigned long start, unsigned int interval);
/* or */
template <typename T>
uint8_t setInterval(T *lambda, unsigned long start, unsigned long interval);
```
Calls the function every interval milliseconds. Runs for the first time after the start plus interval.
```cpp
template <typename T>
uint8_t setInterval(/*Function pointer or lambda here*/, unsigned int interval);
```
Calls parameter 1 whenever parameter 2 is true. Parameter 2 must be a function pointer to a function with no parameters and that returns a boolean.
```cpp
uint8_t addEventListener(TaskFunctionFuncPtr f, ConditionFuncPtr c)
/* or */
template <typename T>
uint8_t addEventListener(T *lambda, ConditionFuncPtr c)
```

### What is returned?
The ID of that task. Use deleteTask(ID) to remove it from the tasks list. If you wanted something to run every 10 seconds for 100 times, you could use setInterval and then have a timeout that removed that task after 1000 seconds. Tasks that only run once (aka setTimeout) are automatically deleted after they run, so no need to clean them up.

Here is some example code on how to use it. You can pass in functions or lambdas.
```cpp
#include <MemoryFree.h>
#define pl(x) Serial.println(F(x));
#define pt(x) Serial.print(F(x));
#define pSize(x) {Serial.print(F("sizeof(" #x)); Serial.print(F(") = "));Serial.println(sizeof(x));}

#include <Tasks.h>
#define PIN 2

void fm() {
    pt("Free Memory = ");
    Serial.println(freeMemory());
}

void setup ()
{
    //Required setup
    pinMode(PIN, INPUT_PULLUP);
    Serial.begin(115200);

    //Setup something that prints the free memory every second
    setInterval(fm, 0, 1000);
    pt("Starting free memory: ");
    fm();

    // Add an event listener that should run 
    byte x = addEventListener([]() {
        Serial.println("Pin is high!");
        delay(100);
    } , []() -> boolean {return digitalRead(PIN);});

    // Cancel the event listener using the ID of the old event listener after 20 seconds
    setTimeout(new auto ([ = ]() {
        pl("Stopping event listener");
        deleteTask(x);
    }), 20000);

    pt("After setup: ");
    fm();
}

void loop () {
    runTasks();
}
```
### Code fails when millis() overflows. Ill probably have to fix that eventually.
