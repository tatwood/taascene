#ifndef taa_SCENETEXTURE_H_
#define taa_SCENETEXTURE_H_

#include <taa/system.h>

enum { taa_SCENETEXTURE_NAMESIZE = 32 };
enum { taa_SCENETEXTURE_PATHSIZE = 256 };

//****************************************************************************
// enums

enum taa_scenetexture_format_e
{
    taa_SCENETEXTURE_LUM8,
    taa_SCENETEXTURE_BGR8,
    taa_SCENETEXTURE_BGRA8,
    taa_SCENETEXTURE_RGB8,
    taa_SCENETEXTURE_RGBA8,
};

enum taa_scenetexture_origin_e
{
    taa_SCENETEXTURE_BOTTOMLEFT,
    taa_SCENETEXTURE_TOPLEFT
};

//****************************************************************************
// typedefs

typedef enum taa_scenetexture_format_e taa_scenetexture_format;
typedef enum taa_scenetexture_origin_e taa_scenetexture_origin;

typedef struct taa_scenetexture_s taa_scenetexture;

//****************************************************************************
// structs

struct taa_scenetexture_s
{
    char name[taa_SCENETEXTURE_NAMESIZE];
    char path[taa_SCENETEXTURE_PATHSIZE];
    taa_scenetexture_origin origin;
    /// number of mip maps
    uint32_t numlevels;
    /// x resolution
    uint32_t width;
    /// y resolution
    uint32_t height;
    /// z resolution
    uint32_t depth;
    /// bits per pixel
    uint32_t bpp;
    taa_scenetexture_format format;
    void** images;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE void taa_scenetexture_convert_format(
    taa_scenetexture* tex,
    taa_scenetexture_format format);

taa_SCENE_LINKAGE void taa_scenetexture_create(
    const char* name,
    const char* path,
    taa_scenetexture_origin origin,
    taa_scenetexture* tex_out);

taa_SCENE_LINKAGE void taa_scenetexture_destroy(
    taa_scenetexture* tex);

taa_SCENE_LINKAGE void taa_scenetexture_generate_mipmaps(
    taa_scenetexture* tex);

taa_SCENE_LINKAGE void taa_scenetexture_make_linear(
    taa_scenetexture* tex);

taa_SCENE_LINKAGE void taa_scenetexture_resize(
    taa_scenetexture* tex,
    int numlevels,
    int width,
    int height,
    int depth,
    taa_scenetexture_format format);

#endif // taa_SCENETEXTURE_H_
