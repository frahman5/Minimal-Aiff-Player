/* === Endian-related stuff === */

#define ARRANGE_ENDIAN_16(dat) ( (((dat) & 0xff00 ) >> 8 ) | (((dat) & 0x00ff ) << 8 ) )
#define ARRANGE_ENDIAN_32(dat) ( (((dat) & 0xff000000 ) >> 24 ) | (((dat) & 0x00ff0000 ) >> 8 ) | (((dat) & 0x0000ff00 ) << 8 ) | (((dat) & 0x000000ff ) << 24 ) )

#define WORDS_BIGENDIAN 0                       // Should be able to read this off the computer

#ifdef WORDS_BIGENDIAN
# define ARRANGE_BE16(dat) (dat)
# define ARRANGE_BE32(dat) (dat)
# define ARRANGE_LE16(dat) ARRANGE_ENDIAN_16(dat)
# define ARRANGE_LE32(dat) ARRANGE_ENDIAN_32(dat)
#else
# define ARRANGE_BE16(dat) ARRANGE_ENDIAN_16(dat)
# define ARRANGE_BE32(dat) ARRANGE_ENDIAN_32(dat)
# define ARRANGE_LE16(dat) (dat)
# define ARRANGE_LE32(dat) (dat)
#endif /* WORDS_BIGENDIAN */