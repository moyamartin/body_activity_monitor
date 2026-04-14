/**
 * @file utils.h
 * @brief Utility macros for bit manipulation. All of them were extracted from
 * Zephyr's include/sys/util.h, which is licensed under Apache-2.0 License.
 */
#ifndef UTILS_H_
#define UTILS_H_

#ifndef BIT
#if defined(_ASMLANGUAGE)
#define BIT(n)  (1 << (n))
#else
/**
 * @brief Unsigned integer with bit position @p n set (signed in
 * assembly language).
 * @note extracted from Zephyr's include/sys/util.h, which is licensed under Apache-2.0 License
 */
#define BIT(n)  (1UL << (n))
#endif
#endif

#define IS_BIT_SET(value, bit) (((value) & (bit)) != 0)

/**
 * @brief Create a contiguous bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#endif /* UTILS_H_ */