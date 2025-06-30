/**
 *@file:
 * @author: jiangxiaoping
 * @data: 2025-07-09
 * @version: v1.0.0
 */
#include "driver_list.h"

#include "print.h"
#include "tag.h"

#include <stdlib.h>

/**
 * @brief Create a new node for the linked list.
 * @param driver Pointer to the driver to be stored in the node.
 * @return Pointer to the newly created node, or NULL if failed.
 */
static DriverListNode_t *driverCreateNode(Driver_t *driver)
{
    if (driver == NULL) {
        PRINT_ERROR("driver is NULL!");
        return NULL; 
    }

    DriverListNode_t *newNode = (DriverListNode_t *)malloc(sizeof(DriverListNode_t));
    if (newNode == NULL) {
        PRINT_ERROR("DriverListNode_t malloc failed!");
        return NULL;
    }

    newNode->driver = driver;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

/**
 * @brief Insert a node at the head of the linked list.
 * @param head Pointer to the head of the linked list.
 * @param driver Pointer to the driver to be inserted.
 * @return None.
 */
void driverInsertNodeAtHead(DriverListNode_t **head, Driver_t *driver)
{
    DriverListNode_t *newNode = driverCreateNode(driver);
    if (newNode == NULL)
        return;

    if (*head == NULL) {
        *head = newNode;
    } else {
        (*head)->prev = newNode;
        newNode->next = *head;
        *head = newNode;
    }
}

/**
 * @brief Insert a node at the tail of the linked list.
 * @param head Pointer to the head of the linked list.
 * @param driver Pointer to the driver to be inserted.
 * @return None.
 */
void driverInsertNodeAtTail(DriverListNode_t **head, Driver_t *driver)
{
    DriverListNode_t *newNode = driverCreateNode(driver);
    if (newNode == NULL)
        return;

    if (*head == NULL) {
        *head = newNode;
    } else {
        DriverListNode_t *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        newNode->prev = current;
    }
}

/**
 * @brief Delete a node by tag from the linked list.
 * @param head Pointer to the head of the linked list.
 * @param tag The tag of the driver to be deleted.
 * @return None.
  */
void driverDeleteNodeByTag(DriverListNode_t** head, const uint32_t tag)
{
    if (*head == NULL || tag >= DRIVER_TAG_END) {
        PRINT_DEBUG("head is NULL or tag is invalid");
        return;
    }

    DriverListNode_t *current = *head;
    while(current != NULL) {
        if (current->driver->tag == tag) {
            if ((current->prev == NULL) && (current->next == NULL)) { // only one node
                *head = NULL;
            } else if (current->prev == NULL) { // delete head node
                current->next->prev = NULL;
                *head = current->next;
            } else if (current->next == NULL) { // delete tail node
                current->prev->next = NULL;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            PRINT_INFO("driver tag%d is free", tag);
            free(current);
            current = NULL;
            return;
        }
        current = current->next;
    }

    PRINT_INFO("driver tag%d not exist", tag);
}

/**
 * @brief Get a node by tag from the linked list.
 * @param head Pointer to the head of the linked list.
 * @param tag The tag of the driver to be found.
 * @return Pointer to the found node, or NULL if not found.
 */
DriverListNode_t *driverGetNodeByTag(DriverListNode_t *head, uint32_t tag)
{
    DriverListNode_t *current = head;
    while (current != NULL) {
        if (current->driver->tag == tag) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}