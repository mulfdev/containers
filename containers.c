#include "containers.h"

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
