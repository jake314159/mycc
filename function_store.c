#include <stdlib.h>
#include <string.h>
#include "function_store.h"

typedef struct FUNC_STORE_LL {
    FUNC_ENTRY entry;
    struct FUNC_STORE_LL *next;
} FUNC_STORE_LL;

void free_func_entry(FUNC_ENTRY *entry)
{
    free(entry);
}

void free_ll(FUNC_STORE_LL *node)
{
    if(node == NULL) return;

    free_ll(node->next);
    free(node);
}

void init_funk_store(VAR_STORE *store)
{
    store->type = 'f';
    store->contents = NULL;
}

PARAM_ENTRY* append_param_lists(PARAM_ENTRY *left, PARAM_ENTRY *right)
{
    if(left == NULL) return right;
    else if(right == NULL) return left;

    PARAM_ENTRY *end = left;
    while(end->next != NULL) {
        end = end->next;
    }
    end->next = right;
    end = left;

    int i = 1;
    while(end->next != NULL) {
        end->param_number = i;
        i++;
        end = end->next;
    }
    return left;
}

PARAM_ENTRY* make_param_list(ptree *tree)
{
    if(tree == NULL) return NULL;

    if(tree->type == NODE_PARAMITER_CHAIN) {
        //make left subtree into chain
        PARAM_ENTRY* left = make_param_list(tree->body.a_parent.left);
        //Make right subtree into chain
        PARAM_ENTRY *right = make_param_list(tree->body.a_parent.right);
        //Append right chain to left tree
        PARAM_ENTRY *total = append_param_lists(left, right);
        //Return appended list
        return total;
    } else {
        //Is a paramiter def
        PARAM_ENTRY *param = malloc(sizeof(PARAM_ENTRY));
        param->type = *getType(tree->body.a_parent.left->body.a_string, tree->type == NODE_PARAMITER_DEF_POINTER);
        param->next = NULL;
        return param;
    }
}

FUNC_ENTRY* make_entry(ptree *tree)
{
    if(tree == NULL || tree->type != NODE_FUNCTION_DEF) return false;

    //b1 == is it going to return a pointer?
    bool pointer = tree->body.a_parent.left->type == NODE_FUNCTION_TYPE_POINTER_NAME_PAIR;
    TYPE_DEF *type = getType(tree->body.a_parent.left->body.a_parent.left->body.a_string, pointer);
    char *name = tree->body.a_parent.left->body.a_parent.right->body.a_string;

    FUNC_ENTRY *entry = malloc(sizeof(FUNC_ENTRY));
    entry->type = *type;
    entry->name = name;
    entry->param = make_param_list(tree->body.a_parent.right->body.a_parent.left);
    return entry;
}

bool add_function_to_store(VAR_STORE *store, ptree *tree)
{
    if(store == NULL) return false;
    if(tree == NULL || tree->type != NODE_FUNCTION_DEF) return false;

    FUNC_STORE_LL *node = malloc(sizeof(FUNC_STORE_LL));
    node->entry = *make_entry(tree);
    node->next = NULL;
    if(get_function_entry(store, node->entry.name) != NULL) {
        //It already exists!
        free_ll(node);
        return false;
    }

    if(store->contents == NULL) {
        //This is the first entry
        store->contents = node;
        return true;
    } else {
        FUNC_STORE_LL *end = store->contents;
        while(end->next != NULL) {
            end = end->next;
        }
        end->next = node;
        return true;
    }
}

FUNC_ENTRY* get_function_entry(VAR_STORE *store, char *name)
{
    FUNC_STORE_LL *node = store->contents;
    while(node != NULL) {
        if(strcmp(node->entry.name, name) == 0) {
            return &node->entry;
        }
        node = node->next;
    }
    return NULL;
}

