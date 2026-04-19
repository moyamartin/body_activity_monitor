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

#if !(defined(__CHAR_BIT__) && defined(__SIZEOF_LONG__) && defined(__SIZEOF_LONG_LONG__))
#error Missing required predefined macros for BITS_PER_LONG calculation
#endif

/** Number of bits in a byte. */
#define BITS_PER_BYTE (__CHAR_BIT__)

/** Number of bits in a nibble. */
#define BITS_PER_NIBBLE (__CHAR_BIT__ / 2)

/** Number of nibbles in a byte. */
#define NIBBLES_PER_BYTE (BITS_PER_BYTE / BITS_PER_NIBBLE)

/** Number of bits in a long int. */
#define BITS_PER_LONG (__CHAR_BIT__ * __SIZEOF_LONG__)

/** Number of bits in a long long int. */
#define BITS_PER_LONG_LONG (__CHAR_BIT__ * __SIZEOF_LONG_LONG__)

/**
 * @brief Create a contiguous bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/** @brief Extract the Least Significant Bit from @p value. */
#define LSB_GET(value) ((value) & -(value))

/**
 * @brief Extract a bitfield element from @p value corresponding to
 *	  the field mask @p mask.
 */
#define FIELD_GET(mask, value)  (((value) & (mask)) / LSB_GET(mask))

/**
 * @brief Prepare a bitfield element using @p value with @p mask representing
 *	  its field position and width. The result should be combined
 *	  with other fields using a logical OR.
 */
#define FIELD_PREP(mask, value) (((value) * LSB_GET(mask)) & (mask))

/*
 * @brief Macro to obtain array size
 */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))



#endif /* UTILS_H_ */