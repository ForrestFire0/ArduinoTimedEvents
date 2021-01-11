#include <MemoryFree.h>

#define MAX_TASKS 80
#include <Tasks.h>
// Generic task function that is called


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