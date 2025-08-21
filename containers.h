// containers.h - v1.0 - STB-style header-only containers library for C
//
// USAGE:
//   In exactly one source file (.c/.cpp), define CONTAINERS_DEFINE before including:
//      #define CONTAINERS_DEFINE 
//      #include "containers.h"
//
//   In all other files, just include normally:
//      #include "containers.h"
//
// EXAMPLE:
//   Arena *arena = arena_create(1024);
//   Str str1 = str_create(arena, "Hello ");
//   Str str2 = str_create(arena, "world!");
//   Str result = str_concat(arena, str1, str2);
//   printf("%s\n", result.data); // "Hello world!"
//   arena_destroy(arena);
//
//   DynArray_int arr = dynarr_create_int(8);
//   *dynarr_push_int(&arr) = 42;
//   printf("Value: %d\n", *dynarr_get_int(&arr, 0));
//   dynarr_destroy_int(&arr);
//
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//
// FEATURES:
//   - Arena allocator for fast bulk allocation/deallocation
//   - Generic dynamic arrays with type-safe macros
//   - String type with length caching and arena allocation
//   - Debug mode with bounds checking and use-after-free detection
//   - Memory alignment and growth strategies optimized for performance
//
// DEFINES:
//   CONTAINERS_DEBUG - Enable debug checks and error reporting (auto-set in DEBUG builds)

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
    #ifndef CONTAINERS_DEBUG
        #define CONTAINERS_DEBUG 1
    #endif
#else
    #ifndef CONTAINERS_DEBUG
        #define CONTAINERS_DEBUG 0
    #endif
#endif

// ============================================================================
// DECLARATIONS
// ============================================================================

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

// ============================================================================
// PUBLIC API
// ============================================================================

// Arena API
[[nodiscard]] Arena *arena_create(size_t initial_size);
[[nodiscard]] void *arena_alloc(Arena *arena, size_t size);
[[nodiscard]] bool arena_is_valid(const Arena *arena);
void arena_destroy(Arena *arena);

// Generic DynArray API
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

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

#define arena_alloc_type(arena, type) ((type*)arena_alloc(arena, sizeof(type)))
#define dynarr_create_typed(type, capacity) dynarr_create(sizeof(type), capacity)
#define dynarr_push_typed(arr, type) ((type*)dynarr_push(arr))
#define dynarr_get_typed(arr, type, index) ((type*)dynarr_get(arr, index))
#define dynarr_get_unchecked_typed(arr, type, index) ((type*)dynarr_get_unchecked(arr, index))
#define dynarr_last_typed(arr, type) ((type*)dynarr_last(arr))

#define STR(literal) ((Str){(literal), sizeof(literal) - 1})
#define str_eq(a, b) (str_cmp(a, b) == 0)

// ============================================================================
// TYPE-SAFE DYNAMIC ARRAY GENERATOR
// ============================================================================

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

// ============================================================================
// PREDEFINED TYPE-SAFE ARRAYS
// ============================================================================

DEFINE_DYNARRAY(int)
DEFINE_DYNARRAY(float)
DEFINE_DYNARRAY(double)
DEFINE_DYNARRAY(char)
DEFINE_DYNARRAY(Str)

// ============================================================================
// IMPLEMENTATION
// ============================================================================

#ifdef CONTAINERS_DEFINE 

#define ARENA_DEFAULT_SIZE (64 * 1024)
#define ARENA_MAGIC_ALIVE 0xABCDEF01
#define ARENA_MAGIC_DEAD 0xDEADBEEF

// Actual Arena implementation (hidden from users)
struct Arena {
    uint32_t magic;
    char *memory;
    size_t size;
    size_t offset;
};

Arena *arena_create(size_t initial_size) {
    if (!initial_size)
        initial_size = ARENA_DEFAULT_SIZE;

    Arena *arena = malloc(sizeof(Arena));
    if (!arena)
        return NULL;

    char *memory = malloc(initial_size);
    if (!memory) {
        free(arena);
        return NULL;
    }

    *arena = (Arena){ARENA_MAGIC_ALIVE, memory, initial_size, 0};
    return arena;
}

void *arena_alloc(Arena *arena, size_t size) {
    if (!arena)
        return NULL;

    // Check for use-after-free
    if (arena->magic == ARENA_MAGIC_DEAD) {
        if (CONTAINERS_DEBUG) {
            fprintf(stderr, "ERROR: Attempted to allocate from destroyed arena!\n");
            abort();
        }
        return NULL;
    }

    if (arena->magic != ARENA_MAGIC_ALIVE || !arena->memory)
        return NULL;

    size_t aligned_size = (size + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);

    if (arena->offset + aligned_size > arena->size) {
        size_t new_size = arena->size << 1;
        if (new_size < arena->offset + aligned_size) {
            new_size = arena->offset + aligned_size;
        }
        char *new_memory = realloc(arena->memory, new_size);
        if (!new_memory)
            return NULL;
        arena->memory = new_memory;
        arena->size = new_size;
    }

    void *result = arena->memory + arena->offset;
    arena->offset += aligned_size;
    return result;
}

bool arena_is_valid(const Arena *arena) {
    return arena && arena->magic == ARENA_MAGIC_ALIVE && arena->memory;
}

void arena_destroy(Arena *arena) {
    if (!arena)
        return;

    if (arena->magic == ARENA_MAGIC_DEAD) {
        if (CONTAINERS_DEBUG) {
            fprintf(stderr, "WARNING: Double-destroy of arena detected!\n");
        }
        return;
    }

    if (arena->magic != ARENA_MAGIC_ALIVE && CONTAINERS_DEBUG) {
        fprintf(stderr, "WARNING: Destroying corrupted arena!\n");
    }

    if (CONTAINERS_DEBUG && arena->memory) {
        // Poison memory to catch access-after-free
        memset(arena->memory, 0xDE, arena->size);
    }

    free(arena->memory);
    arena->magic = ARENA_MAGIC_DEAD;
    arena->memory = NULL;
    arena->size = 0;
    arena->offset = 0;

    free(arena);
}

// Generic DynArray implementation
DynArray dynarr_create(size_t element_size, size_t initial_capacity) {
    if (!initial_capacity)
        initial_capacity = 8;
    void *data = malloc(element_size * initial_capacity);
    return data ? (DynArray){data, 0, initial_capacity, element_size} : (DynArray){0};
}

void *dynarr_push(DynArray *arr) {
    if (!arr || !arr->data)
        return NULL;

    if (arr->count >= arr->capacity) {
        size_t new_capacity = arr->capacity << 1;
        void *new_data = realloc(arr->data, arr->element_size * new_capacity);
        if (!new_data)
            return NULL;
        arr->data = new_data;
        arr->capacity = new_capacity;
    }

    void *result = (char *)arr->data + (arr->count * arr->element_size);
    arr->count++;
    return result;
}

void *dynarr_get(const DynArray *arr, size_t index) {
    if (!arr || !arr->data)
        return NULL;

    if (CONTAINERS_DEBUG && index >= arr->count) {
        fprintf(stderr, "ERROR: Array index %zu out of bounds (size: %zu)\n", index, arr->count);
        abort();
    }

    return index < arr->count ? (char *)arr->data + (index * arr->element_size) : NULL;
}

void *dynarr_get_unchecked(const DynArray *arr, size_t index) {
    return (char *)arr->data + (index * arr->element_size);
}

void *dynarr_last(const DynArray *arr) {
    return arr && arr->count > 0 ? dynarr_get_unchecked(arr, arr->count - 1) : NULL;
}

void dynarr_pop(DynArray *arr) {
    if (!arr || arr->count == 0)
        return;

    arr->count--;
    if (arr->count < arr->capacity >> 2 && arr->capacity > 8) {
        size_t new_capacity = arr->capacity >> 1;
        void *new_data = realloc(arr->data, arr->element_size * new_capacity);
        if (new_data) {
            arr->data = new_data;
            arr->capacity = new_capacity;
        }
    }
}

void dynarr_clear(DynArray *arr) {
    if (arr)
        arr->count = 0;
}

void dynarr_destroy(DynArray *arr) {
    if (arr) {
        free(arr->data);
        *arr = (DynArray){0};
    }
}

// String implementation
Str str_create(Arena *arena, const char *cstr) {
    if (!cstr || !arena_is_valid(arena))
        return (Str){0};
    size_t len = strlen(cstr);
    char *data = arena_alloc(arena, len + 1);
    if (!data)
        return (Str){0};
    memcpy(data, cstr, len + 1);
    return (Str){data, len};
}

Str str_copy(Arena *arena, const char *data, size_t len) {
    if (!data || !len || !arena_is_valid(arena))
        return (Str){0};
    char *copy = arena_alloc(arena, len + 1);
    if (!copy)
        return (Str){0};
    memcpy(copy, data, len);
    copy[len] = '\0';
    return (Str){copy, len};
}

Str str_concat(Arena *arena, Str a, Str b) {
    if (!arena_is_valid(arena))
        return (Str){0};
    if (!a.len)
        return str_copy(arena, b.data, b.len);
    if (!b.len)
        return str_copy(arena, a.data, a.len);

    size_t total_len = a.len + b.len;
    char *data = arena_alloc(arena, total_len + 1);
    if (!data)
        return (Str){0};

    memcpy(data, a.data, a.len);
    memcpy(data + a.len, b.data, b.len);
    data[total_len] = '\0';
    return (Str){data, total_len};
}

Str str_substr(Arena *arena, Str s, size_t start, size_t len) {
    if (!arena_is_valid(arena) || start >= s.len)
        return (Str){0};
    if (start + len > s.len)
        len = s.len - start;
    return str_copy(arena, s.data + start, len);
}

int str_cmp(Str a, Str b) {
    if (a.len != b.len)
        return a.len < b.len ? -1 : 1;
    return memcmp(a.data, b.data, a.len);
}

#endif // CONTAINERS_DEFINE 

#endif // CONTAINERS_H
