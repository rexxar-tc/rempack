add_library(rmkit_impl STATIC rmkit_impl.cpp)
add_library(stb_impl STATIC stb_impl.cpp)

target_link_libraries(rmkit_impl PUBLIC stb_impl)
target_compile_definitions(rmkit_impl PRIVATE
        #when building for native hardware, REMARKABLE must be defined
        $<$<STREQUAL:${TARGET_PLATFORM},rm>:REMARKABLE>
        #debug builds require an external font file. I've provided one for you in /include
        $<$<STREQUAL:${TARGET_PLATFORM},x86>:FONT_EMBED_H="font_embed.h">
)
target_compile_definitions(stb_impl PRIVATE
        #when building for native hardware, REMARKABLE must be defined
        $<$<STREQUAL:${TARGET_PLATFORM},rm>:REMARKABLE>
        #debug builds require an external font file. I've provided one for you in /include
        $<$<STREQUAL:${TARGET_PLATFORM},x86>:FONT_EMBED_H="font_embed.h">
)