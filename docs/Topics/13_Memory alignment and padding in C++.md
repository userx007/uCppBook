# Memory Alignment & Padding in C++

## What is Memory Alignment?

Memory alignment refers to how data is arranged and accessed in memory. A memory address is said to be **aligned** to N bytes when the address is a multiple of N. For example, an address aligned to 4 bytes would be 0, 4, 8, 12, etc.

Different data types have different alignment requirements based on their size. On most modern systems, the alignment requirement typically matches the size of the type (up to the processor's word size).

## Why Does Alignment Matter?

**Hardware Performance:** Modern CPUs read memory in fixed-size chunks (called words), typically 4 or 8 bytes at a time. When data is properly aligned, the CPU can read it in a single memory access. Misaligned data might require multiple memory accesses, significantly slowing down your program.

**Hardware Requirements:** Some architectures (like older ARM processors) generate hardware faults when accessing misaligned data. On x86/x64, misaligned access is allowed but comes with a performance penalty.

## What is Padding?

Padding is extra unused bytes that the compiler inserts between structure members or at the end of a structure to satisfy alignment requirements. This ensures each member is properly aligned.

## How It Works in Practice

Let me show you with concrete examples:

```cpp
struct Example1 {
    char a;    // 1 byte
    int b;     // 4 bytes
    char c;    // 1 byte
};
```

You might expect this structure to be 6 bytes (1+4+1), but on most systems it's actually **12 bytes**. Here's what happens:

- `char a` occupies byte 0
- 3 bytes of padding are added (bytes 1-3)
- `int b` starts at byte 4 (aligned to 4 bytes)
- `char c` occupies byte 8
- 3 bytes of padding are added at the end (bytes 9-11)

The trailing padding ensures that arrays of this structure maintain proper alignment for all elements.

## Order Matters

You can reduce padding by ordering members from largest to smallest:

```cpp
struct Example2 {
    int b;     // 4 bytes
    char a;    // 1 byte
    char c;    // 1 byte
    // 2 bytes padding at end
};
```

This version is only **8 bytes** instead of 12, because the two chars can share the same 4-byte alignment slot.

## Typical Alignment Requirements

On most modern 64-bit systems, the alignment rules are typically:
- `char` (1 byte): 1-byte alignment
- `short` (2 bytes): 2-byte alignment
- `int` (4 bytes): 4-byte alignment
- `long` and pointers (8 bytes on 64-bit): 8-byte alignment
- `double` (8 bytes): 8-byte alignment

The overall structure alignment is determined by its largest member. So a struct containing a `double` will be 8-byte aligned.

## Controlling Alignment

**Check alignment and size:**
```cpp
std::cout << alignof(Example1) << std::endl;  // alignment requirement
std::cout << sizeof(Example1) << std::endl;   // total size with padding
```

**Force specific alignment:**
```cpp
struct alignas(16) AlignedStruct {
    int x;
};  // Will be aligned to 16 bytes
```

**Pack structures (remove padding):**
```cpp
#pragma pack(push, 1)
struct Packed {
    char a;
    int b;
    char c;
};  // Now exactly 6 bytes
#pragma pack(pop)
```

Be cautious with packing - it can cause performance penalties or even crashes on some architectures.

## Performance Implications

Proper alignment can significantly impact performance. Accessing misaligned data can be 2-10x slower, and on some platforms it's illegal. Cache line alignment (typically 64 bytes) is also important for multithreaded code to avoid false sharing.

## Practical Takeaways

When designing structures, consider ordering members from largest to smallest to minimize padding. Use `sizeof` and `alignof` to verify your assumptions. Only pack structures when you have a specific reason (like matching a file format or network protocol), and be aware of the performance trade-offs.

Memory alignment is one of those low-level details that the compiler usually handles well, but understanding it helps you write more efficient code and debug mysterious size discrepancies in your data structures.