#include <MemoryFree.h>
//Macros I like
#define pl(x) Serial.println(F(x));
#define pt(x) Serial.print(F(x));
#define pSize(x) {Serial.print(F("sizeof(" #x)); Serial.print(F(") = "));Serial.println(sizeof(x));}

#define MAX_TASKS 24
#include <Tasks.h>

void fm() {
    pt("Free Memory = ");
    Serial.println(freeMemory());
}

void setup ()
{
    Serial.begin(115200);
    setTimeout(fm, 6000);
    pt("Starting free memory: ");
    fm();
    setupAll();
    pt("After setup: ");
    fm();
}
void setupAll() {
    for (byte i = 1; i <= 5; i++) {
        setTimeout(new auto ([ = ]() -> void {
            Serial.print("This is lambda ");
            Serial.print(i);
            Serial.print(" runnning at time ");
            Serial.println(millis());
        }), i * 1000);
    }
}

void loop () {
    runTasks();
}