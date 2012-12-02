/**
 * @brief     animation processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENEANIM_H_
#define taa_SCENEANIM_H_

#include "scenenode.h"

//****************************************************************************
// constants

enum
{
    taa_SCENEANIM_NAMESIZE = 32
};

//****************************************************************************
// enums

enum taa_sceneanim_interpolation_e
{
    taa_SCENEANIM_INTERPOLATE_BEZIER,
    taa_SCENEANIM_INTERPOLATE_LINEAR,
    taa_SCENEANIM_INTERPOLATE_STEP
};

//****************************************************************************
// typedefs

typedef enum taa_sceneanim_interpolation_e taa_sceneanim_interpolation;
typedef struct taa_sceneanim_keyframe_s taa_sceneanim_keyframe;
typedef struct taa_sceneanim_channel_s taa_sceneanim_channel;
typedef struct taa_sceneanim_s taa_sceneanim;

//****************************************************************************
// structs

struct taa_sceneanim_keyframe_s
{
    taa_sceneanim_interpolation interpolation;
    float time;
    float values[16];
    /// control point 0 for bezier interpolation
    taa_vec2 cpin;
    /// control point 1 for bezier interpolation
    taa_vec2 cpout;
};

struct taa_sceneanim_channel_s
{
    int32_t nodeid;
    uint32_t numcomponents;
    uint8_t componentmask[16];
    uint32_t numkeyframes;
    taa_sceneanim_keyframe* keyframes;
};

struct taa_sceneanim_s
{
    char name[taa_SCENEANIM_NAMESIZE];
    /// animation time lengtn, in seconds
    float length;
    uint32_t numchannels;
    taa_sceneanim_channel* channels;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE int32_t taa_sceneanim_add_channel(
    taa_sceneanim* anim,
    uint32_t numcomponents,
    uint8_t componentmask[16],
    uint32_t node);

taa_SCENE_LINKAGE int32_t taa_sceneanim_add_frame(
    taa_sceneanim* anim,
    taa_sceneanim_channel* chan,
    taa_sceneanim_interpolation interp,
    float time,
    const float* values,
    const taa_vec2* cpin,
    const taa_vec2* cpout);

taa_SCENE_LINKAGE void taa_sceneanim_create(
    const char* name,
    taa_sceneanim* anim_out);

taa_SCENE_LINKAGE void taa_sceneanim_destroy(
    taa_sceneanim* anim);

/**
 * Calculates the transform values of the scene nodes at the specified time
 */
taa_SCENE_LINKAGE void taa_sceneanim_play(
    const taa_sceneanim* anim,
    float time,
    taa_scenenode* nodes,
    uint32_t numnodes);

taa_SCENE_LINKAGE void taa_sceneanim_resize_channels(
    taa_sceneanim* anim,
    uint32_t numchannels);

taa_SCENE_LINKAGE void taa_sceneanim_resize_frames(
    taa_sceneanim_channel* chan,
    uint32_t numkeyframes);

/**
 * @brief change the up axis of the animation
 * @details bakes a 90 degree rotation around the x axis into the animation
 * @param dir +1 for counter-clockwise rotation, -1 for clockwise
 */
taa_SCENE_LINKAGE void taa_sceneanim_rotate_upaxis(
    taa_sceneanim* anim,
    taa_scenenode* nodes,
    uint32_t numnodes,
    int32_t dir);

taa_SCENE_LINKAGE float taa_sceneanim_sample(
    const taa_sceneanim_channel* channel,
    float time,
    int component);

#endif // taa_SCENEANIM_H_
