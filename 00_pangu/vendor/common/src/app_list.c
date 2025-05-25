/**
 *@file:
 * @author: jiangxiaoping
 * @data: 2025-04-23
 * @version: v1.0.0
 */
#include "app_list.h"

#include "print.h"
#include "tag.h"

#include <stdlib.h>

/**
 * @brief Create a new node for the linked list.
 * @param app Pointer to the app to be stored in the node.
 * @return Pointer to the newly created node, or NULL if failed.
 */
static AppListNode_t *appCreateNode(App_t *app)
{
    if (app == NULL) {
        PRINT_ERROR("app is NULL!");
        return NULL; 
    }

    AppListNode_t *newNode = (AppListNode_t *)malloc(sizeof(AppListNode_t));
    if (newNode == NULL) {
        PRINT_ERROR("AppListNode_t malloc failed!");
        return NULL;
    }

    newNode->app = app;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

/**
 * @brief Insert a node at the head of the linked list.
 * @param head Pointer to the head of the linked list.
 * @param app Pointer to the app to be inserted.
 * @return None.
 */
void appInsertNodeAtHead(AppListNode_t **head, App_t *app)
{
    AppListNode_t *newNode = appCreateNode(app);
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
 * @param app Pointer to the app to be inserted.
 * @return None.
 */
void appInsertNodeAtTail(AppListNode_t **head, App_t *app)
{
    AppListNode_t *newNode = appCreateNode(app);
    if (newNode == NULL)
        return;

    if (*head == NULL) {
        *head = newNode;
    } else {
        AppListNode_t *current = *head;
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
 * @param tag The tag of the app to be deleted.
 * @return None.
  */
void appDeleteNodeByTag(AppListNode_t** head, const uint32_t tag)
{
    if (*head == NULL || tag >= APP_TAG_END) {
        PRINT_DEBUG("head is NULL or tag is invalid");
        return;
    }

    AppListNode_t *current = *head;
    while(current != NULL) {
        if (current->app->tag == tag) {
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
            PRINT_INFO("app tag%d is free", tag);
            free(current);
            current = NULL;
            return;
        }
        current = current->next;
    }

    PRINT_INFO("app tag%d not exist", tag);
}

/**
 * @brief Get a node by tag from the linked list.
 * @param head Pointer to the head of the linked list.
 * @param tag The tag of the app to be found.
 * @return Pointer to the found node, or NULL if not found.
 */
AppListNode_t *appGetNodeByTag(AppListNode_t *head, uint32_t tag)
{
    AppListNode_t *current = head;
    while (current != NULL) {
        if (current->app->tag == tag) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}