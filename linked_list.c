#include "linked_list.h"

void list_init(Node** head) {
    mem_init(2048);
    *head = NULL;
}

/// @brief inserts first in linked list
/// @param head
/// @param data
void list_insert(Node** head, int data) {
    // Node* newNode = mem_alloc(sizeof(Node));
    // newNode->data = data;
    // newNode->next = *head;
    // *head = newNode;
    Node* newNode = mem_alloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    if(*head == NULL) {
        *head = newNode;
        return;
    }
    Node* walker = *head;
    while(walker->next){
        walker = walker->next;
    }
    walker->next = newNode;
}

/// @brief Inserts a node after prev_node
/// @param prev_node
/// @param data
void list_insert_after(Node* prev_node, int data) {
    if (prev_node == NULL) return;
    Node* newNode = mem_alloc(sizeof(Node));
    newNode->next = prev_node->next;
    newNode->data = data;
    prev_node->next = newNode;
}

void list_insert_before(Node** head, Node* next_node, int data) {
    if (*head == NULL) return;  // ERROR
    Node* walker = *head;
    if (next_node == *head) return list_insert(head, data);

    while (walker->next != next_node && walker->next != NULL) {
        walker = walker->next;
    }
    if (walker->next == NULL) return;  // ERRROR

    walker->next = mem_alloc(sizeof(Node));
    walker->next->next = next_node;
    walker->next->data = data;
}

void list_delete(Node** head, int data) {
    if (*head == NULL) return;
    if ((*head)->data == data) {
        Node* temp = *head;
        *head = (*head)->next;
        mem_free(temp);
        return;
    }
    Node* walker = *head;
    while (walker->next != NULL && walker->next->data != data) {
        walker = walker->next;
    }
    if (walker->next == NULL) return;
    Node* temp = walker->next;
    walker->next = temp->next;
    mem_free(temp);
}

Node* list_search(Node** head, int data) {
    Node* walker = *head;
    while (walker != NULL && walker->data != data) {
        walker = walker->next;
    }
    return walker;
}

// void list_display(Node** head){
//     Node* walker = *head;
//     printf("[");
//     while(walker != NULL){
//         printf("%d", walker->data);
//         if(walker->next) printf(", ");
//     }
//     printf("]\n");
// }

void list_display(Node** head, Node* start_node, Node* end_node) {
    if (*head == NULL) {
        printf("[]\n");
        return;
    }
    if (end_node) end_node = end_node->next;
    if (!start_node) start_node = *head;
    printf("[");
    while (start_node != NULL && start_node != end_node) {
        printf("%d", start_node->data);
        start_node = start_node->next;
        if (start_node && start_node != end_node) printf(", ");
    }
    printf("]\n");
}

void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (end_node) end_node = end_node->next;
    if (!start_node) start_node = *head;
    printf("[");
    while (start_node != NULL && start_node != end_node) {
        printf("%d", start_node->data);
        start_node = start_node->next;
        if (start_node && start_node != end_node) printf(", ");
    }
    printf("]\n");
}

int list_count_nodes(Node** head) {
    if (*head == NULL) return 0;
    Node* walker = *head;
    int counter = 0;
    while (walker != NULL) {
        counter++;
        walker = walker->next;
    }
    return counter;
}

void list_cleanup(Node** head) {
    Node* walker = *head;
    while (walker != NULL) {
        Node* temp = walker;
        walker = walker->next;
        mem_free(temp);
    }
    *head = NULL;
    mem_deinit();
}