/**
 * @brief     animation processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/sceneanim.h>
#include <taa/scalar.h>
#include <taa/solve.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//****************************************************************************
static float taa_sceneanim_calc_bezier(
    float p0,
    float cp0,
    float cp1,
    float p1,
    float s)
{
    // B(s) = P0(1-s)^3 + 3C0*s(1-s)^2 + 3C1*s^2(1- s) + P1*s^3
    return
        p0*(1-s)*(1-s)*(1-s) +
        3*cp0*s*(1-s)*(1-s) +
        3*cp1*s*s*(1-s) +
        p1*s*s*s;
}

//****************************************************************************
int32_t taa_sceneanim_add_channel(
    taa_sceneanim* anim,
    uint32_t numcomponents,
    uint8_t componentmask[16],
    uint32_t nodeid)
{
    int32_t animid = anim->numchannels;
    taa_sceneanim_channel* chan;
    taa_sceneanim_resize_channels(anim, animid + 1);
    chan = anim->channels + animid;
    chan->nodeid = nodeid;
    chan->numcomponents = numcomponents;
    memcpy(chan->componentmask, componentmask, sizeof(chan->componentmask));
    return animid;
}

//****************************************************************************
int32_t taa_sceneanim_add_frame(
    taa_sceneanim* anim,
    taa_sceneanim_channel* chan,
    taa_sceneanim_interpolation interp,
    float time,
    const float* values,
    const taa_vec2* cpin,
    const taa_vec2* cpout)
{
    int32_t i = chan->numkeyframes;
    taa_sceneanim_keyframe* kf;
    taa_sceneanim_resize_frames(chan, i + 1);
    kf = chan->keyframes + i;
    kf->interpolation = interp;
    kf->time = time;
    memcpy(kf->values, values, chan->numcomponents*sizeof(*values));
    kf->cpin = *cpin;
    kf->cpout = *cpout;
    if(time > anim->length)
    {
        anim->length = time;
    }
    return i;
}

//****************************************************************************
void taa_sceneanim_create(
    const char* name,
    taa_sceneanim* anim_out)
{
    memset(anim_out, 0, sizeof(*anim_out));
    strncpy(anim_out->name, name, sizeof(anim_out->name) - 1);
}

//****************************************************************************
void taa_sceneanim_destroy(
    taa_sceneanim* anim)
{
    taa_sceneanim_channel* chanitr = anim->channels;
    taa_sceneanim_channel* chanend = chanitr + anim->numchannels;
    while(chanitr != chanend)
    {
        free(chanitr->keyframes);
        ++chanitr;
    }
    free(anim->channels);
}

//****************************************************************************
void taa_sceneanim_play(
    const taa_sceneanim* anim,
    float time,
    taa_scenenode* nodes,
    uint32_t numnodes)
{
    const taa_sceneanim_channel* chanitr = anim->channels;
    const taa_sceneanim_channel* chanend = chanitr + anim->numchannels;
    while(chanitr != chanend)
    {
        assert(((uint32_t) chanitr->nodeid) < numnodes);
        if(((uint32_t) chanitr->nodeid) < numnodes)
        {
            const int numcomps = chanitr->numcomponents;
            taa_scenenode* node = nodes + chanitr->nodeid;
            float* dst = NULL;
            int maskindex;
            int comp;
            switch(node->type)
            {
            case taa_SCENENODE_TRANSFORM_MATRIX:
                dst = &node->value.matrix.x.x;
                break;
            case taa_SCENENODE_TRANSFORM_ROTATE:
                dst = &node->value.rotate.x;
                break;
            case taa_SCENENODE_TRANSFORM_SCALE:
                dst = &node->value.scale.x;
                break;
            case taa_SCENENODE_TRANSFORM_TRANSLATE:
                dst = &node->value.translate.x;
                break;
            default:
                assert(0);
                break;
            }
            comp = 0;
            for(maskindex = 0; maskindex < 16; ++maskindex)
            {
                if(chanitr->componentmask[maskindex] != 0)
                {
                    dst[maskindex] = taa_sceneanim_sample(chanitr,time,comp);
                    ++comp;
                    if(comp == numcomps)
                    {
                        break;
                    }
                }
            }
        }
        ++chanitr;
    }
}

//****************************************************************************
void taa_sceneanim_resize_channels(
    taa_sceneanim* anim,
    uint32_t numchannels)
{
    uint32_t oldnum = anim->numchannels;
    taa_sceneanim_channel* chan = anim->channels;
    if(numchannels > anim->numchannels)
    {
        uint32_t cap = (oldnum     +63) & ~63;
        uint32_t ncap= (numchannels+63) & ~63;
        if(cap != ncap)
        {
            chan = (taa_sceneanim_channel*) realloc(chan,ncap*sizeof(*chan));
            anim->channels = chan;
        }
        chan += oldnum;
        memset(chan, 0, (numchannels-oldnum) * sizeof(*chan));
    }
    else
    {
        taa_sceneanim_channel* chanend = chan + anim->numchannels;
        chan += numchannels;
        while(chan != chanend)
        {
            free(chan->keyframes);
            ++chan;
        }
    }
    anim->numchannels = numchannels;
}

//****************************************************************************
void taa_sceneanim_resize_frames(
    taa_sceneanim_channel* chan,
    uint32_t numkeyframes)
{
    uint32_t oldnum = chan->numkeyframes;
    taa_sceneanim_keyframe* kf = chan->keyframes;
    if(numkeyframes > chan->numkeyframes)
    {
        uint32_t cap = (oldnum      +63) & ~63;
        uint32_t ncap= (numkeyframes+63) & ~63;
        if(cap != ncap)
        {
            kf = (taa_sceneanim_keyframe*) realloc(kf, ncap*sizeof(*kf));
            chan->keyframes = kf;
        }
        kf += oldnum;
        memset(kf, 0, (numkeyframes-oldnum) * sizeof(*kf));
    }
    chan->numkeyframes = numkeyframes;
}

//****************************************************************************
void taa_sceneanim_rotate_upaxis(
    taa_sceneanim* anim,
    taa_scenenode* nodes,
    uint32_t numnodes,
    int32_t dir)
{
    taa_sceneanim_channel* chanitr = anim->channels;
    taa_sceneanim_channel* chanend = chanitr + anim->numchannels;

    while(chanitr != chanend)
    {
        assert(((uint32_t) chanitr->nodeid) < numnodes);
        if(chanitr->numcomponents == 1)
        {
            int kfdir = 1;
            // only one component altered by this channel
            // if it targets y or z component, the axes must be flipped
            if(chanitr->componentmask[1])
            {
                // Y Component
                chanitr->componentmask[1] = 0;
                chanitr->componentmask[2] = 1;
                kfdir = -dir;
            }
            else if(chanitr->componentmask[2])
            {
                // Z Component
                chanitr->componentmask[1] = 1;
                chanitr->componentmask[2] = 0;
                kfdir = dir;
            }
            if(kfdir < 0)
            {
                // if the channel targets the y or z component of a
                // translation, the sign of the keyframe values may flip
                if(((uint32_t) chanitr->nodeid) < numnodes)
                {
                    taa_scenenode* node = nodes + chanitr->nodeid;
                    if(node->type == taa_SCENENODE_TRANSFORM_TRANSLATE)
                    {
                        taa_sceneanim_keyframe* kfitr;
                        taa_sceneanim_keyframe* kfend;
                        kfitr = chanitr->keyframes;
                        kfend = kfitr + chanitr->numkeyframes;
                        while(kfitr != kfend)
                        {
                            kfitr->cpin.y    = -kfitr->cpin.y;
                            kfitr->cpout.y   = -kfitr->cpout.y;
                            kfitr->values[0] = -kfitr->values[0];
                            ++kfitr;
                        }
                    }
                }
            }
        }
        else if(((uint32_t) chanitr->nodeid) < numnodes)
        {
            // this channel targets multiple components
            taa_scenenode* node = nodes + chanitr->nodeid;
            if(node->type == taa_SCENENODE_TRANSFORM_MATRIX)
            {
                taa_sceneanim_keyframe* kfitr;
                taa_sceneanim_keyframe* kfend;
                taa_mat44 pitch;
                taa_mat44 invpitch;
                taa_mat44_pitch(dir * taa_radians(90.0f), &pitch);
                taa_mat44_transpose(&pitch, &invpitch);
                kfitr = chanitr->keyframes;
                kfend = kfitr + chanitr->numkeyframes;
                while(kfitr != kfend)
                {
                    taa_mat44 m;
                    taa_mat44 n;
                    memcpy(&n, kfitr->values, sizeof(n));
                    taa_mat44_multiply(&invpitch, &n, &m);
                    taa_mat44_multiply(&m, &pitch, &n);
                    memcpy(kfitr->values, &n, sizeof(n));
                    ++kfitr;
                }
            }
            else
            {
                // TODO
                assert(0);
            }
        }
        ++chanitr;
    }
}

//****************************************************************************
float taa_sceneanim_sample(
    const taa_sceneanim_channel* channel,
    float time,
    int component)
{
    const taa_sceneanim_keyframe* f0;
    const taa_sceneanim_keyframe* f1;
    float result;
    int key;
    int lastkey;
    // determine key frame range at specified time
    key = 0;
    lastkey = channel->numkeyframes - 1;
    while(key < lastkey)
    {
        if(channel->keyframes[key + 1].time > time)
        {
            break;
        }
        ++key;
    }
    f0 = channel->keyframes + key;
    f1 = f0 + 1;
    // calculate result depending on interpolation type
    if(key==lastkey || f0->interpolation==taa_SCENEANIM_INTERPOLATE_STEP)
    {
        // if the time is at the end of the input array, or the interpolation
        // mode is stepped, just return the last result directly.
        result = f0->values[component];
    }
    else if(f0->interpolation==taa_SCENEANIM_INTERPOLATE_BEZIER)
    {
        float solutions[3];
        float a;
        float b;
        float c;
        float d;
        float s;
        int i;
        int n;
        // solve cubic equation to find parameter of curve at specified time
        // time should be the result of sampling the x axis of the curve
        // therefore, s is the only unknown variable in the following equation
        // time = P0_x(1-s)^3 + 3C0_x*s(1-s)^2 + 3C1_x*s^2(1 - s) + P1_x*s^3
        a =   -f0->time + 3*f0->cpout.x - 3*f1->cpin.x + f1->time;
        b =  3*f0->time - 6*f0->cpout.x + 3*f1->cpin.x;
        c = -3*f0->time + 3*f0->cpout.x;
        d =    f0->time - time;
        n = taa_solve_cubic(a, b, c, d, solutions);
        s = 0.0f;
        for(i = 0; i < n; ++i)
        {
            if(solutions[i] >= 0.0f && solutions[i] <= 1.0f)
            {
                s = solutions[i];
#ifndef NDEBUG
                // debug check to ensure accuracy of computations
                // sampling the x axis of the curve using the computed
                // parameter should result in the provided time
                result =  taa_sceneanim_calc_bezier(
                    f0->time,
                    f0->cpout.x,
                    f1->cpin.x,
                    f1->time,
                    s);
                assert(fabs(time - result) < 1e-3f);
#endif
                break;
            }
        }
        assert(n > 0);
        // sample the y axis curve using the computed parameter
        result = taa_sceneanim_calc_bezier(
            f0->values[component],
            f0->cpout.y,
            f1->cpin.y,
            f1->values[component],
            s);
    }
    else
    {
        // linear interpolation
        float s = (time - f0->time)/(f1->time - f0->time);
        result = taa_mix(f0->values[component], f1->values[component], s);
    }
    return result;
}
