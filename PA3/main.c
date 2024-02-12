// Author: Edoardo Silvio Gribaldo (eg1005)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEVICES 6000
#define N_TRIES 10

long N_lin;
long N_bin;
long N_log;

long linearBackoff ( int increment );
long binaryBackoff ( int increment );
long loglogBackoff ( int increment );
long devicePickSlot ( long W, int choice );

int main() {
    // Opening of the output files
    FILE * fout1 = fopen("linearLatency.txt", "w");
    FILE * fout2 = fopen("binaryLatency.txt", "w");
    FILE * fout3 = fopen("loglogLatency.txt", "w");

    for ( int i=0; i < MAX_DEVICES; i += 100) { // for each protocol we increment the number of devices by 100 each time starting from 100 up to 6000
        long total_linear = 0, total_binary = 0, total_loglog = 0;
        // for each protocol we do ten tries and then take the average of them
        for ( int j=0; j<N_TRIES; j++) {
            N_lin = 100;
            total_linear += linearBackoff(i);
        }
        fprintf(fout1, "%lu\n", (long) total_linear / (long) N_TRIES );
        for ( int j=0; j<N_TRIES; j++) {
            N_bin = 100;
            total_binary += binaryBackoff(i);
        }
        fprintf(fout2, "%lu\n", (long) total_binary / (long) N_TRIES);
        for ( int j=0; j<N_TRIES; j++) {
            N_log = 100;
            total_loglog += loglogBackoff(i);
        }
        fprintf(fout3, "%lu\n", (long) total_loglog / (long) N_TRIES);
    }
    //closing the output files
    fclose(fout1);
    fclose(fout2);
    fclose(fout3);
    return 0;
}

long linearBackoff (int increment) {
    N_lin += increment;
    long slots = 2;
    long total_wait = 0;
    long max_slot = devicePickSlot(slots, 0);
    while (N_lin > 0) {
        total_wait += slots;
        slots++; // way of increasing the window size typical of the linear backoff protocol
        max_slot = devicePickSlot(slots, 0);
    }
    total_wait += max_slot;
    return total_wait;

}
long binaryBackoff (int increment) {
    N_bin += increment;
    long slots = 2;
    long total_wait = 0;
    long max_slot = devicePickSlot(slots, 1);
    while (N_bin > 0) {
        total_wait += slots;
        slots *= 2; // way of increasing the window size typical of the binary backoff protocol
        max_slot = devicePickSlot(slots, 1);
    }
    total_wait += max_slot;
    return total_wait;
}
long loglogBackoff (int increment) {
    N_log += increment;
    long slots = 4;
    long total_wait = 0;
    long max_slot = devicePickSlot(slots, 2);
    while (N_log > 0) {
        total_wait += slots;
        slots = floorl((1+ 1/log2l(log2l(slots)))*slots); // way of increasing the window size typical of the loglog backoff protocol
        max_slot = devicePickSlot(slots, 2);
    }
    total_wait += max_slot;
    return total_wait;
}

long devicePickSlot(long W, int choice) {
    long inactivated_device = 0;
    long max_value = 0;
    long limit;
    int * slot_vector = calloc(W, sizeof(int));
    switch (choice) { // depending on which function called the devicePickSlot() we set the limit to the corresponding number of transmissions still to be done
        case 0:
            limit = N_lin;
            break;
        case 1:
            limit = N_bin;
            break;
        case 2:
            limit = N_log;
            break;
    }
    for (int s=0; s<limit; s++) { // we simulate the transmissions
        long random = drand48() * W;
        slot_vector[random]++;
    }
    for (int  s=0; s<W; s++) { // we look for transmissions without collisions
        if (slot_vector[s] == 1) {
            inactivated_device++; // we count how many transmissions went smoothly
            max_value = s; // this value is useful and will be returned to the outer function to calculate the total waiting time
        }
    }
    switch (choice) { // depending on which function called the devicePickSlot() we update the number of transmissions still to be done
        case 0:
            N_lin -= inactivated_device;
            break;
        case 1:
            N_bin -= inactivated_device;
            break;
        case 2:
            N_log -= inactivated_device;
            break;
    }
    free(slot_vector);
    return max_value;
}