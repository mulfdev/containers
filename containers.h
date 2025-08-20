#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef DEBUG
    #include <assert.h>
    #define CONTAINERS_DEBUG 1
#else
    #define CONTAINERS_DEBUG 0
#endif

// Opaque types - implementation hidden
typedef struct Arena Arena;

// String with length + null termination
typedef struct {
    char *data;  // null-terminated C string
    size_t len;  // cached length (excluding null terminator)
} Str;

// Generic dynamic array (for compatibility)
typedef struct {
    void *data;
    size_t count;
    size_t capacity;
    size_t element_size;
} DynArray;

// Public Arena API
[[nodiscard]] Arena *arena_create(size_t initial_size);
[[nodiscard]] void *arena_alloc(Arena *arena, size_t size);
[[nodiscard]] bool arena_is_valid(const Arena *arena);
void arena_destroy(Arena *arena);

// Public DynArray API (generic)
[[nodiscard]] DynArray dynarr_create(size_t element_size, size_t initial_capacity);
[[nodiscard]] void *dynarr_push(DynArray *arr);
[[nodiscard]] void *dynarr_get(const DynArray *arr, size_t index);
[[nodiscard]] void *dynarr_get_unchecked(const DynArray *arr, size_t index);
[[nodiscard]] void *dynarr_last(const DynArray *arr);
void dynarr_pop(DynArray *arr);
void dynarr_clear(DynArray *arr);
void dynarr_destroy(DynArray *arr);

// String API
[[nodiscard]] Str str_create(Arena *arena, const char *cstr);
[[nodiscard]] Str str_copy(Arena *arena, const char *data, size_t len);
[[nodiscard]] Str str_concat(Arena *arena, Str a, Str b);
[[nodiscard]] Str str_substr(Arena *arena, Str s, size_t start, size_t len);
int str_cmp(Str a, Str b);

// Basic macros
#define arena_alloc_type(arena, type) ((type*)arena_alloc(arena, sizeof(type)))
#define dynarr_create_typed(type, capacity) dynarr_create(sizeof(type), capacity)
#define dynarr_push_typed(arr, type) ((type*)dynarr_push(arr))
#define dynarr_get_typed(arr, type, index) ((type*)dynarr_get(arr, index))
#define dynarr_get_unchecked_typed(arr, type, index) ((type*)dynarr_get_unchecked(arr, index))
#define dynarr_last_typed(arr, type) ((type*)dynarr_last(arr))

#define STR(literal) ((Str){(literal), sizeof(literal) - 1})
#define str_eq(a, b) (str_cmp(a, b) == 0)

// Type-safe dynamic array generator
#define DEFINE_DYNARRAY(T) \
typedef struct { \
    T *data; \
    size_t count; \
    size_t capacity; \
} DynArray_##T; \
\
[[nodiscard]] static inline DynArray_##T dynarr_create_##T(size_t capacity) { \
    if (!capacity) capacity = 8; \
    T *data = (T*)malloc(sizeof(T) * capacity); \
    return data ? (DynArray_##T){data, 0, capacity} : (DynArray_##T){0}; \
} \
\
[[nodiscard]] static inline T *dynarr_push_##T(DynArray_##T *arr) { \
    if (arr->count >= arr->capacity) { \
        size_t new_capacity = arr->capacity << 1; \
        T *new_data = (T*)realloc(arr->data, sizeof(T) * new_capacity); \
        if (!new_data) return NULL; \
        arr->data = new_data; \
        arr->capacity = new_capacity; \
    } \
    return &arr->data[arr->count++]; \
} \
\
[[nodiscard]] static inline T *dynarr_get_##T(DynArray_##T *arr, size_t index) { \
    if (CONTAINERS_DEBUG && index >= arr->count) { \
        fprintf(stderr, "ERROR: Array index %zu out of bounds (size: %zu)\n", index, arr->count); \
        abort(); \
    } \
    return index < arr->count ? &arr->data[index] : NULL; \
} \
\
[[nodiscard]] static inline T *dynarr_get_unchecked_##T(DynArray_##T *arr, size_t index) { \
    return &arr->data[index]; \
} \
\
[[nodiscard]] static inline T *dynarr_last_##T(DynArray_##T *arr) { \
    return arr->count > 0 ? &arr->data[arr->count - 1] : NULL; \
} \
\
static inline void dynarr_pop_##T(DynArray_##T *arr) { \
    if (arr->count == 0) return; \
    arr->count--; \
    if (arr->count < arr->capacity >> 2 && arr->capacity > 8) { \
        size_t new_capacity = arr->capacity >> 1; \
        T *new_data = (T*)realloc(arr->data, sizeof(T) * new_capacity); \
        if (new_data) { \
            arr->data = new_data; \
            arr->capacity = new_capacity; \
        } \
    } \
} \
\
static inline void dynarr_clear_##T(DynArray_##T *arr) { \
    arr->count = 0; \
} \
\
static inline void dynarr_destroy_##T(DynArray_##T *arr) { \
    free(arr->data); \
    *arr = (DynArray_##T){0}; \
}

// Common type-safe arrays
DEFINE_DYNARRAY(int)
DEFINE_DYNARRAY(float)
DEFINE_DYNARRAY(double)
DEFINE_DYNARRAY(char)
DEFINE_DYNARRAY(Str)

#endif // CONTAINERS_H
