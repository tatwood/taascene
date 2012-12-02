#include <taa/scenetexture.h>
#include <taa/scalar.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//****************************************************************************
static int taa_scenetexture_calc_bpp(
    taa_scenetexture_format format)
{
    int bpp = 0;
    switch(format)
    {
    case taa_SCENETEXTURE_LUM8 : bpp =  8; break;
    case taa_SCENETEXTURE_BGR8 : bpp = 24; break;
    case taa_SCENETEXTURE_BGRA8: bpp = 32; break;
    case taa_SCENETEXTURE_RGB8 : bpp = 24; break;
    case taa_SCENETEXTURE_RGBA8: bpp = 32; break;
    }
    assert(bpp != 0);
    return bpp;
}

//****************************************************************************
void taa_scenetexture_convert_format(
    taa_scenetexture* tex,
    taa_scenetexture_format format)
{
    if(format != tex->format)
    {
        const int srcbytepp =  tex->bpp/8;
        const int dstbytepp =  taa_scenetexture_calc_bpp(format)/8;
        int width = tex->width;
        int height = tex->height;
        void** level = tex->images;
        void** levelend = level + tex->numlevels;
        while(level != levelend)
        {
            const uint8_t* srcitr = (uint8_t*) *level;
            uint8_t* dstimage;
            uint8_t* dstitr;
            uint8_t* dstend;
            if(srcbytepp == dstbytepp)
            {
                dstimage = (uint8_t*) *level;
            }
            else
            {
                dstimage = (uint8_t*) malloc(width * height * dstbytepp);
            }
            dstitr = dstimage;
            dstend = dstitr + width * height * dstbytepp;
            if(dstbytepp == 1)
            {
                while(dstitr != dstend)
                {
                    *dstitr = *srcitr;
                    srcitr += srcbytepp;
                    dstitr += dstbytepp;
                }
            }
            else if(srcbytepp == 1)
            {
                while(dstitr != dstend)
                {
                    uint8_t* dstnext = dstitr + dstbytepp;
                    while(dstitr != dstnext)
                    {
                        *dstitr = *srcitr;
                        ++dstitr;
                    }
                    srcitr += srcbytepp;
                }
            }
            else
            {
                int issrcrgb =
                    tex->format == taa_SCENETEXTURE_RGBA8 ||
                    tex->format == taa_SCENETEXTURE_RGB8;
                int isdstrgb = 
                    format == taa_SCENETEXTURE_RGBA8 ||
                    format == taa_SCENETEXTURE_RGB8;
                while(dstitr != dstend)
                {
                    uint8_t* dstnext = dstitr + dstbytepp;
                    const uint8_t* srcnext = srcitr + srcbytepp;
                    // copy as many channels as will fit into the destination
                    while(srcitr != srcnext && dstitr != dstnext)
                    {
                        *dstitr = *srcitr;
                        ++dstitr;
                        ++srcitr;
                    }
                    // fill out any remaining destination channels
                    while(dstitr != dstnext)
                    {
                        *dstitr = 0xff;
                        ++dstitr;
                    }
                    srcitr = srcnext;
                }
                if(issrcrgb != isdstrgb)
                {
                    // if one is rgb(a), and the other is bgr(a), swap r and b
                    dstitr = dstimage;
                    while(dstitr != dstend)
                    {
                        uint8_t c = dstitr[0];
                        dstitr[0] = dstitr[2];
                        dstitr[2] = c;
                        dstitr += dstbytepp;
                    }
                }
            }
            if(dstimage != *level)
            {
                free(*level);
                *level = dstimage;
            }
            width /= 2;
            height /= 2;
            ++level;
        }
    }
}

//****************************************************************************
void taa_scenetexture_create(
    const char* name,
    const char* path,
    taa_scenetexture_origin origin,
    taa_scenetexture* tex_out)
{
    memset(tex_out, 0, sizeof(*tex_out));
    strncpy(tex_out->name, name, sizeof(tex_out->name) - 1);
    strncpy(tex_out->path, path, sizeof(tex_out->path) - 1);
    tex_out->origin = origin;
}

//****************************************************************************
void taa_scenetexture_make_linear(
    taa_scenetexture* tex)
{
    /*
            {  cs / 12.92,                 cs <= 0.04045
       cl = {
            {  ((cs + 0.055)/1.055)^2.4,   cs >  0.04045
    */
    const int bytepp = tex->bpp/8;
    int width = tex->width;
    int height = tex->height;
    void** lvlitr = tex->images;
    void** lvlend = lvlitr + tex->numlevels;
    while(lvlitr != lvlend)
    {
        uint8_t* px = (uint8_t*) *lvlitr;
        uint8_t* pxend = px + width * height * bytepp;
        while(px != pxend)
        {
            uint8_t* pxnext = px + bytepp;
            uint8_t* rgbend = px + 3;
            while(px != pxnext)
            {
                if(px < rgbend)
                {
                    float cs = (*px)/255.0f;
                    float cl;
                    if(cs <= 0.04045f)
                    {
                        cl = cs/12.92f;
                    }
                    else
                    {
                        cl = (float) pow(((cs + 0.055)/1.055), 2.4);
                    }
                    *px = (uint8_t) taa_clampi((int) (cl * 255.0f), 0, 255);
                }
                ++px;
            }
        }
        width /= 2;
        height /= 2;
        ++lvlitr;
    }
}

//****************************************************************************
void taa_scenetexture_destroy(
    taa_scenetexture* tex)
{
    void** lvlitr = tex->images;
    void** lvlend = lvlitr + tex->numlevels;
    while(lvlitr != lvlend)
    {
        free(*lvlitr);
        ++lvlitr;
    }
    free(tex->images);
}

//****************************************************************************
void taa_scenetexture_generate_mipmaps(
    taa_scenetexture* tex)
{
    const int bytepp = tex->bpp/8;
    int width = tex->width;
    int height = tex->height;
    void** prevlevel = tex->images;
    void** level = prevlevel + 1;
    void** levelend = prevlevel + tex->numlevels;
    width  /= 2;
    height /= 2;
    while(level != levelend)
    {
        const uint8_t* src0 = (uint8_t*) *prevlevel;  // row of prev mip
        const uint8_t* src1 = src0 + (2*width*bytepp); // next row of prev mip
        uint8_t* dst = (uint8_t*) *level;
        uint8_t* dstend = dst + width*height*bytepp;
        while(dst != dstend)
        {
            uint8_t* dstrowend = dst + width*bytepp;
            while(dst != dstrowend)
            {
                // loop through each pixel of the current mip level
                uint8_t* dstnext = dst + bytepp;
                while(dst != dstnext)
                {
                    // loop through each channel of the current pixel
                    // and average the 4 source pixels from the previous mip
                    uint32_t c;
                    c  = src0[0];
                    c += src0[bytepp];
                    c += src1[0];
                    c += src1[bytepp];
                    *dst = (uint8_t) (c/4);
                    ++dst;
                    ++src0;
                    ++src1;
                }
                // previous mip level has twice as many pixels per row
                // increment it forward again
                src0 += bytepp;
                src1 += bytepp;
            }
            // previous mip level had twice as many rows, increment it forward
            src0 += 2*width*bytepp;
            src1 += 2*width*bytepp;
        }
        prevlevel = level;
        width /= 2;
        height /= 2;
        ++level;
    }
}

//****************************************************************************
void taa_scenetexture_resize(
    taa_scenetexture* tex,
    int numlevels,
    int width,
    int height,
    int depth,
    taa_scenetexture_format format)
{
    void** level;
    void** levelend;
    int bpp;
    int oldlevels;
    int w;
    int h;
    int d;
    // determine the number of bits per pixel
    bpp = taa_scenetexture_calc_bpp(format);
    // automatically calculate the number of levels
    if(numlevels <= 0)
    {
        numlevels = 0;
        w = width;
        h = height;
        while(w > 0 && h > 0)
        {
            ++numlevels;
            w >>= 1;
            h >>= 1;
        }
    }
    // determine if the level buffer needs to be resized
    oldlevels = tex->numlevels;
    if(numlevels > oldlevels)
    {
        level = tex->images;
        level = (void**) realloc(
            level,
            numlevels * sizeof(*level));
        tex->images = level;
        memset(level + oldlevels, 0, (numlevels-oldlevels) * sizeof(*level));
    }
    else if(numlevels < oldlevels)
    {
        level = tex->images + numlevels;
        levelend = level + oldlevels;
        while(level != levelend)
        {
            free(*level);
            ++level;
        }
    }
    // allocate images for each level
    level = tex->images;
    levelend = level + numlevels;
    w = width;
    h = height;
    d = depth;
    while(level != levelend)
    {
        uint32_t imagesize = w * h * depth * bpp/8;
        // TODO: resize the image if necessary
        *level = realloc(*level, imagesize);
        w >>= 1;
        h >>= 1;
        d >>= 1;
        ++level;
    }
    // set struct members
    tex->numlevels = numlevels;
    tex->width = width;
    tex->height = height;
    tex->depth = depth;
    tex->bpp = bpp;
    tex->format = format;
}
