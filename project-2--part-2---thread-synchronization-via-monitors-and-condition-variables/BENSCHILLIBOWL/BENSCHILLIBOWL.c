#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "BENSCHILLIBOWL.h"

MenuItem PickRandomMenuItem() {
    return Menu[rand() % MENU_SIZE];
}

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb = malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->max_size = max_size;
    bcb->current_size = 0;
    bcb->expected_num_orders = expected_num_orders;
    bcb->orders_handled = 0;
    pthread_mutex_init(&bcb->mutex, NULL);
    pthread_cond_init(&bcb->can_add_orders, NULL);
    pthread_cond_init(&bcb->can_get_orders, NULL);
    return bcb;
}

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    pthread_mutex_destroy(&bcb->mutex);
    pthread_cond_destroy(&bcb->can_add_orders);
    pthread_cond_destroy(&bcb->can_get_orders);
    free(bcb);
    printf("Restaurant is now closed!\n");
}

int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&bcb->mutex);

    while (bcb->current_size >= bcb->max_size) {
        pthread_cond_wait(&bcb->can_add_orders, &bcb->mutex);
    }

    AddOrderToBack(&bcb->orders, order);
    bcb->current_size++;

    pthread_cond_signal(&bcb->can_get_orders);
    pthread_mutex_unlock(&bcb->mutex);

    return 0;
}

Order* GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&bcb->mutex);

    while (bcb->current_size <= 0) {
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&bcb->mutex);
            return NULL;
        }
        pthread_cond_wait(&bcb->can_get_orders, &bcb->mutex);
    }

    Order* order = bcb->orders;
    bcb->orders = bcb->orders->next;
    bcb->current_size--;
    bcb->orders_handled++;

    pthread_cond_signal(&bcb->can_add_orders);
    pthread_mutex_unlock(&bcb->mutex);

    return order;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
    return bcb->current_size >= bcb->max_size;
}

void AddOrderToBack(Order** orders, Order* order) {
    order->next = NULL;
    if (*orders == NULL) {
        *orders = order;
    } else {
        Order* temp = *orders;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = order;
    }
}
