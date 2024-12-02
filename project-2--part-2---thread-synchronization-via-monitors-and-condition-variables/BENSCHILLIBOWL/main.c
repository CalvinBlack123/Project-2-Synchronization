#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "BENSCHILLIBOWL.h"

// Function prototypes
void* BENSCHILLIBOWLCustomer(void* tid);
void* BENSCHILLIBOWLCook(void* tid);

int main() {
    int num_customers = 10;
    int num_cooks = 3;
    int max_size = 5; // Maximum size of the restaurant's order queue
    int expected_num_orders = num_customers;

    // Open the restaurant
    BENSCHILLIBOWL* bcb = OpenRestaurant(max_size, expected_num_orders);

    pthread_t customer_threads[num_customers];
    pthread_t cook_threads[num_cooks];

    // Create customer threads
    for (int i = 0; i < num_customers; i++) {
        int* tid = malloc(sizeof(int));
        *tid = i + 1;
        pthread_create(&customer_threads[i], NULL, BENSCHILLIBOWLCustomer, tid);
    }

    // Create cook threads
    for (int i = 0; i < num_cooks; i++) {
        int* tid = malloc(sizeof(int));
        *tid = i + 1;
        pthread_create(&cook_threads[i], NULL, BENSCHILLIBOWLCook, tid);
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < num_customers; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Wait for all cook threads to finish
    for (int i = 0; i < num_cooks; i++) {
        pthread_join(cook_threads[i], NULL);
    }

    // Close the restaurant
    CloseRestaurant(bcb);

    return 0;
}

void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = *(int*)tid;
    free(tid);

    printf("Customer %d: Arrived at the restaurant.\n", customer_id);

    // Allocate memory for an order
    Order* order = malloc(sizeof(Order));
    order->menu_item = PickRandomMenuItem();
    order->customer_id = customer_id;

    // Add the order to the restaurant
    printf("Customer %d: Ordering %s.\n", customer_id, order->menu_item);
    AddOrder(bcb, order);

    return NULL;
}

void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = *(int*)tid;
    free(tid);

    printf("Cook %d: Ready to take orders.\n", cook_id);

    while (1) {
        Order* order = GetOrder(bcb);
        if (order == NULL) break;

        printf("Cook %d: Fulfilled order for Customer %d (%s).\n", cook_id, order->customer_id, order->menu_item);
        free(order); // Free memory for the order
    }

    printf("Cook %d: All orders are completed.\n", cook_id);
    return NULL;
}
