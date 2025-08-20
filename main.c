#include "containers.h"
#include <stdio.h>
int main() {

    Arena *arena = arena_create(1024);

    Str str1 = str_create(arena, "Hello ");
    Str str2 = str_create(arena, "world!");
    Str result = str_concat(arena, str1, str2);

    printf("%s\n", result.data); // Prints: Hello world!

    arena_destroy(arena);

    DynArray_int arr = dynarr_create_int(8);

    // Add elements
    *dynarr_push_int(&arr) = 42;
    *dynarr_push_int(&arr) = 100;
    *dynarr_push_int(&arr) = 7;

    // Access elements
    printf("First: %d\n", *dynarr_get_int(&arr, 0));
    printf("Last: %d\n", *dynarr_last_int(&arr));
    printf("Count: %zu\n", arr.count);

    // Clean up
    dynarr_destroy_int(&arr);
    return 0;
}
