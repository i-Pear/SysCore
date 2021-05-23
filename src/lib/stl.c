#include "stl.h"

inline pair_int make_pair(int a, int b) {
    pair_int result = {a, b};
    return result;
}

void Vec_init(Vec **vec, size_t type_size) {
    *vec = k_malloc(sizeof(Vec));
    (*vec)->length = 0;
    (*vec)->capacity = 16;
    (*vec)->start = k_malloc(type_size * (*vec)->capacity);
    (*vec)->type_size = type_size;
}

void Vec_push(Vec *vec, void *x) {
    if (vec->length == vec->capacity) {
        vec->capacity *= 2;
        char *new_arr = k_malloc(vec->type_size * vec->capacity);
        memcpy(new_arr, vec->start, vec->type_size * (vec->capacity / 2));
        k_free(vec->start);
        vec->start = new_arr;
    }
    void *tar = vec->start + vec->length * vec->type_size;
    memcpy(tar, x, vec->type_size);
    vec->length++;
}

void Vec_pop(Vec *vec, void *x) {
    void *ori = vec->start + (vec->length - 1) * vec->type_size;
    memcpy(x, ori, vec->type_size);
    vec->length--;
}

void Vec_get(Vec *vec, size_t index, void *x) {
    memcpy(x, vec->start + vec->type_size * index, vec->type_size);
}

void Vec_set(Vec *vec, size_t index, void *x) {
    memcpy(vec->start + vec->type_size * index, x, vec->type_size);
}

void Vec_clear(Vec *vec) {
    vec->length = 0;
}

size_t Vec_empty(Vec *vec) {
    return vec->length == 0;
}

void Vec_free(Vec **vec) {
    k_free((*vec)->start);
    k_free((*vec));
}


void Map_init(Map **map) {
    (*map) = k_malloc(sizeof(Map));
    (*map)->start = k_malloc(sizeof(MapNode));
    (*map)->start->left = NULL;
    (*map)->start->right = NULL;
    (*map)->length = 0;
}

void _Map_put(MapNode *node, size_t key, size_t value) {
    if (key == node->key) {
        return;
    } else if (key < node->key) {
        if (!node->left) {
            node->left = k_malloc(sizeof(MapNode));
            node->left->left = NULL;
            node->left->right = NULL;
            node->left->key = key;
            node->left->value = value;
            return;
        }
        _Map_put(node->left, key, value);
    } else {
        if (!node->right) {
            node->right = k_malloc(sizeof(MapNode));
            node->right->left = NULL;
            node->right->right = NULL;
            node->right->key = key;
            node->right->value = value;
            return;
        }
        _Map_put(node->right, key, value);
    }
}

void Map_put(Map *map, size_t key, size_t value) {
//    printf("Len: %d\n", map->length);
    map->length++;
    if (map->length == 0) {
        map->start->key = key;
        map->start->value = value;
        return;
    }
    _Map_put(map->start, key, value);
}

int _Map_get(MapNode *node, size_t key, size_t *value) {
    if (node->key == key) {
        *value = node->value;
        return 1;
    }
    if (key < node->key && node->left) {
        return _Map_get(node->left, key, value);
    }
    if (key > node->key && node->right) {
        return _Map_get(node->right, key, value);
    }
    return 0;
}

int Map_get(Map *map, size_t key, size_t *value) {
    if (map->length == 0) {
        return 0;
    }
    int success = _Map_get(map->start, key, value);
    if (success)
        return 1;
    else
        return 0;
}

void memcpy(void *to, void *from, size_t size) {
    char *tar = to, *ori = from;
    for (size_t i = 0; i < size; i++) {
        *(tar + i) = *(ori + i);
    }
}

void memset(void *p, char content, int size) {
    for (int i = 0; i < size; i++) {
        *((char *) p + i) = content;
    }
}

int strlen(char *s) {
    int n;
    for (n = 0; s[n]; n++);
    return n;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}