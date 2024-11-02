// Last Update: 2024-10-29
// Author: Tong Wu
// Description: Cacheline aligned memory allocation

#pragma once

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <sys/mman.h>
#include <stdexcept>
#define CACHELINE 64
#if defined(__SSE2__)
#define CACHELINE 64
#include <immintrin.h>
#elif defined(__aarch64__)
#define CACHELINE 128
#include <arm_neon.h>
#endif

#if defined(__clang__)

#define FAST_BEGIN
#define FAST_END
#define PG_INLINE __attribute__((always_inline))

#elif defined(__GNUC__)

#define FAST_BEGIN                       \
    _Pragma("GCC push_options") _Pragma( \
        "GCC optimize (\"unroll-loops,associative-math,no-signed-zeros\")")
#define FAST_END _Pragma("GCC pop_options")
#define PG_INLINE [[gnu::always_inline]]
#else

#define FAST_BEGIN
#define FAST_END
#define PG_INLINE

#endif

inline void alloc64B(void **hostPtr, size_t nbytes, int value)
{
    size_t len = (nbytes + (1 << 6) - 1) >> 6 << 6;
    if (posix_memalign(hostPtr, 1 << 6, len) != 0)
    {
        throw std::bad_alloc(); // 分配失败时抛出异常
    }
    memset(*hostPtr, value, len);
}

inline void alloc2M(void **hostPtr, size_t nbytes, int value)
{
    size_t len = (nbytes + (1 << 21) - 1) >> 21 << 21;
    if (posix_memalign(hostPtr, 1 << 21, len) != 0)
    {
        throw std::bad_alloc(); // 分配失败时抛出异常
    }
    madvise(*hostPtr, len, MADV_HUGEPAGE); // 使用大页内存
    memset(*hostPtr, value, len);
}

template <typename T>
struct align_alloc
{
    using value_type = T;

    align_alloc() = default;

    template <typename U>
    constexpr align_alloc(const align_alloc<U> &) noexcept {}

    // allocate 方法，参数为 std::size_t
    T *allocate(std::size_t n)
    {
        T *ptr = nullptr;
        if (n <= (1 << 14))
        {
            alloc64B((void **)&ptr, n * sizeof(T), 0);
        }
        else
        {
            alloc2M((void **)&ptr, n * sizeof(T), 0);
        }
        return ptr;
    }

    // deallocate 方法，参数为 std::size_t
    void deallocate(T *p, std::size_t) noexcept
    {
        free(p);
    }

    template <typename U>
    struct rebind
    {
        typedef align_alloc<U> other;
    };

    bool operator==(const align_alloc &) const noexcept { return true; }
    bool operator!=(const align_alloc &) const noexcept { return false; }
};

template <typename T1, typename T2, typename U, typename... Params>
using Dist = U (*)(const T1 *, const T2 *, int, Params...);

PG_INLINE inline void prefetch_L1(const void *address)
{
#if defined(__SSE2__)
    _mm_prefetch((const char *)address, _MM_HINT_T0);
#else
    __builtin_prefetch(address, 0, 3);
#endif
}

PG_INLINE inline void prefetch_L2(const void *address)
{
#if defined(__SSE2__)
    _mm_prefetch((const char *)address, _MM_HINT_T1);
#else
    __builtin_prefetch(address, 0, 2);
#endif
}

PG_INLINE inline void prefetch_L3(const void *address)
{
#if defined(__SSE2__)
    _mm_prefetch((const char *)address, _MM_HINT_T2);
#else
    __builtin_prefetch(address, 0, 1);
#endif
}

inline void mem_prefetch(char *ptr, const int num_lines)
{
    switch (num_lines)
    {
    default:
        [[fallthrough]];
    case 28:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 27:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 26:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 25:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 24:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 23:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 22:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 21:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 20:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 19:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 18:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 17:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 16:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 15:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 14:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 13:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 12:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 11:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 10:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 9:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 8:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 7:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 6:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 5:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 4:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 3:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 2:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 1:
        prefetch_L1(ptr);
        ptr += 64;
        [[fallthrough]];
    case 0:
        break;
    }
}

