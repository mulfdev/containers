# containers.h

A lightweight, STB-style header-only containers library for C providing arena allocation, dynamic arrays, and strings.

## Features

- **Arena Allocator**: Fast bulk allocation with single deallocation
- **Dynamic Arrays**: Generic and type-safe variants with automatic growth/shrinkage
- **Strings**: Length-cached strings with arena allocation
- **Debug Mode**: Bounds checking and use-after-free detection
- **Zero Dependencies**: Only requires standard C library
- **Header-Only**: Single file include, no build system required

## Quick Start

1. Download `containers.h`
2. In **exactly one** source file:
   ```c
   #define CONTAINERS_DEFINE
   #include "containers.h"
   ```
3. In all other files, just include normally:
   ```c
   #include "containers.h"
   ```

## Basic Usage

### Arena Allocation
```c
Arena *arena = arena_create(1024);

// Allocate single values
int *num = arena_alloc_type(arena, int);
*num = 42;

// Allocate arrays
char *buffer = arena_alloc(arena, 256);

// Free everything at once
arena_destroy(arena);
```

### Strings
```c
Arena *arena = arena_create(0); // Use default size

Str str1 = str_create(arena, "Hello ");
Str str2 = str_create(arena, "world!");
Str result = str_concat(arena, str1, str2);

printf("%s\n", result.data); // "Hello world!"
printf("Length: %zu\n", result.len); // 12

arena_destroy(arena);
```

### Type-Safe Dynamic Arrays
```c
// Create array for integers
DynArray_int arr = dynarr_create_int(8);

// Add elements
*dynarr_push_int(&arr) = 42;
*dynarr_push_int(&arr) = 100;

// Access elements
printf("First: %d\n", *dynarr_get_int(&arr, 0));
printf("Last: %d\n", *dynarr_last_int(&arr));

dynarr_destroy_int(&arr);
```

### Custom Types
```c
typedef struct {
    int id;
    double value;
} Item;

DEFINE_DYNARRAY(Item); // Creates DynArray_Item and functions

DynArray_Item items = dynarr_create_Item(0);
Item *item = dynarr_push_Item(&items);
*item = (Item){1, 3.14};

dynarr_destroy_Item(&items);
```

## API Reference

### Arena Functions
- `Arena *arena_create(size_t initial_size)` - Create arena (0 = default size)
- `void *arena_alloc(Arena *arena, size_t size)` - Allocate memory
- `bool arena_is_valid(const Arena *arena)` - Check if arena is valid
- `void arena_destroy(Arena *arena)` - Destroy arena and free all memory

### String Functions
- `Str str_create(Arena *arena, const char *cstr)` - Create from C string
- `Str str_copy(Arena *arena, const char *data, size_t len)` - Copy data
- `Str str_concat(Arena *arena, Str a, Str b)` - Concatenate strings
- `Str str_substr(Arena *arena, Str s, size_t start, size_t len)` - Substring
- `int str_cmp(Str a, Str b)` - Compare strings (-1, 0, 1)

### Dynamic Array Functions (Type-Safe)
For any type `T` (after `DEFINE_DYNARRAY(T)`):
- `DynArray_T dynarr_create_T(size_t capacity)` - Create array
- `T *dynarr_push_T(DynArray_T *arr)` - Add element, return pointer
- `T *dynarr_get_T(DynArray_T *arr, size_t index)` - Get element (bounds checked)
- `T *dynarr_get_unchecked_T(DynArray_T *arr, size_t index)` - Get element (fast)
- `T *dynarr_last_T(DynArray_T *arr)` - Get last element
- `void dynarr_pop_T(DynArray_T *arr)` - Remove last element
- `void dynarr_clear_T(DynArray_T *arr)` - Clear all elements
- `void dynarr_destroy_T(DynArray_T *arr)` - Destroy array

### Predefined Arrays
Ready to use: `DynArray_int`, `DynArray_float`, `DynArray_double`, `DynArray_char`, `DynArray_Str`

### Useful Macros
- `STR("literal")` - Create Str from string literal
- `str_eq(a, b)` - Test string equality
- `arena_alloc_type(arena, type)` - Type-safe arena allocation

## Configuration

### Debug Mode
Define `CONTAINERS_DEBUG=1` or compile with `-DDEBUG` to enable:
- Bounds checking on array access
- Use-after-free detection for arenas
- Memory poisoning on arena destruction
- Double-free detection

### Memory Behavior
- **Arena growth**: Doubles in size when full, minimum allocation request
- **Array growth**: Doubles when full, shrinks to half when 1/4 full
- **Alignment**: All allocations aligned to `max_align_t`
- **Default arena size**: 64KB

## Error Handling

Functions return `NULL` or zero-initialized structs on failure. In debug mode, bounds violations and use-after-free trigger `abort()`.

## Compatibility

- **C Standard**: Requires C23 for `[[nodiscard]]` attribute (remove for older standards)
- **Compilers**: GCC, Clang, MSVC
- **Platforms**: Any with standard C library

## Building

```bash
# Simple build
gcc -std=c23 -O2 main.c -o main

# Debug build
gcc -std=c23 -g -DDEBUG main.c -o main_debug

# With provided Makefile
make run
```

## Performance Notes

- Arena allocation is O(1) amortized
- Array operations are O(1) amortized except growth/shrinkage
- String operations depend on length (memcpy-based)
- Memory overhead: ~32 bytes per arena, ~24 bytes per array

## License

Apache 2.0

## Contributing

This is a single-header library. Modifications should maintain the STB-style format and backward compatibility.
