/**
 * @brief     skeleton processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENESKEL_H_
#define taa_SCENESKEL_H_

#include "scenenode.h"

//****************************************************************************
// constants

enum
{
    taa_SCENESKEL_NAMESIZE = 32
};

//****************************************************************************
// typedefs

typedef struct taa_sceneskel_joint_s taa_sceneskel_joint;
typedef struct taa_sceneskel_s taa_sceneskel;

//****************************************************************************
// structs

struct taa_sceneskel_joint_s
{
    /**
     * @brief index of parent joint within the skeleton.
     * @details note that the parent joint may not be the parent node.
     */
    int32_t parent;
    /**
     * @brief id of associated node within the scene
     */
    int32_t nodeid;
};

struct taa_sceneskel_s
{
    char name[taa_SCENESKEL_NAMESIZE];
    size_t numjoints;
    taa_sceneskel_joint* joints;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE uint32_t taa_sceneskel_add_joint(
    taa_sceneskel* skel,
    int32_t parent,
    int32_t nodeid);

/**
 * @brief calculates local rotation of a joint relative to its parent joint
 * @details the parent joint may not be the parent node. therefore, the local
 *          joint transform may not be equivalent to the local node transform.
 */
taa_SCENE_LINKAGE void taa_sceneskel_calc_rotate(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_quat* q_out);

/**
 * @brief calculates local scale of a joint relative to its parent joint
 * @details the parent joint may not be the parent node. therefore, the local
 *          joint transform may not be equivalent to the local node transform.
 */
taa_SCENE_LINKAGE void taa_sceneskel_calc_scale(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_vec4* v_out);

/**
 * @brief calculates local transform of a joint relative to its parent joint
 * @details the parent joint may not be the parent node. therefore, the local
 *          joint transform may not be equivalent to the local node transform.
 */
taa_SCENE_LINKAGE void taa_sceneskel_calc_transform(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_mat44* m_out);

/**
 * @brief calculates local translation of a joint relative to its parent joint
 * @details the parent joint may not be the parent node. therefore, the local
 *          joint transform may not be equivalent to the local node transform.
 */
taa_SCENE_LINKAGE void taa_sceneskel_calc_translate(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_vec4* v_out);

taa_SCENE_LINKAGE void taa_sceneskel_create(
    const char* name,
    taa_sceneskel* skel_out);

taa_SCENE_LINKAGE void taa_sceneskel_destroy(
    taa_sceneskel* skel);

taa_SCENE_LINKAGE int32_t taa_sceneskel_find_joint(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    const char* name);

taa_SCENE_LINKAGE void taa_sceneskel_resize_joints(
    taa_sceneskel* skel,
    uint32_t numjoints);

#endif // taa_SCENESKEL_H_
