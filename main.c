#include "containers.h"
#include <stdio.h>
int main() {

    Arena *arena = arena_create(1024);

    Str str1 = str_create(arena, "Hello ");
    Str str2 = str_create(arena, "world!");
    Str result = str_concat(arena, str1, str2);

    printf("%s\\n", result.data); // Prints: Hello world!

    arena_destroy(arena);
    return 0;
}
