#include <stdint.h>

void infinite_loop() {
    volatile uint32_t temp;
    while(1) {
        temp++;
    }
}
