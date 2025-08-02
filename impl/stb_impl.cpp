//
// Created by brant on 7/4/25.
//

#include "zlib.h"
extern "C" unsigned char *stbi_zlib_compress_using_zlib(const unsigned char *data, int data_len, int *out_len, int quality);

#ifdef STBIW_ZLIB_COMPRESS
#undef STBIW_ZLIB_COMPRESS
#endif
#define STBIW_ZLIB_COMPRESS stbi_zlib_compress_using_zlib
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <rmkit.h>

extern "C" unsigned char *stbi_zlib_compress_using_zlib(const unsigned char *data, int data_len, int *out_len, int quality) {
    z_stream zs = {};
    deflateInit2(&zs, Z_BEST_SPEED, Z_DEFLATED, 15, 1, Z_DEFAULT_STRATEGY);

    auto bufsize = deflateBound(&zs, data_len);
    auto* out = (unsigned char*) malloc(bufsize);

    zs.next_in = (Bytef*)data;
    zs.avail_in = data_len;
    zs.next_out = out;
    zs.avail_out = bufsize;

    deflate(&zs, Z_FINISH);
    *out_len = zs.total_out;

    deflateEnd(&zs);
    return out;
}
