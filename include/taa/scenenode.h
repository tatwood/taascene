/**
 * @brief     scene graph node processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENENODE_H_
#define taa_SCENENODE_H_

#include <taa/mat44.h>

//****************************************************************************
// constants

enum
{
    taa_SCENENODE_NAMESIZE = 32
};

//****************************************************************************
// enums

enum taa_scenenode_type_e
{
    taa_SCENENODE_EMPTY,
    taa_SCENENODE_REF_JOINT,
    taa_SCENENODE_REF_MESH,
    taa_SCENENODE_REF_SKEL,
    taa_SCENENODE_TRANSFORM_MATRIX,
    taa_SCENENODE_TRANSFORM_ROTATE,
    taa_SCENENODE_TRANSFORM_SCALE,
    taa_SCENENODE_TRANSFORM_TRANSLATE
};

//****************************************************************************
// typedefs

typedef enum taa_scenenode_type_e taa_scenenode_type;
typedef struct taa_scenenode_s taa_scenenode;

//****************************************************************************
// structs

struct taa_scenenode_s
{ 
    char name[taa_SCENENODE_NAMESIZE];
    taa_scenenode_type type;
    int32_t parent;
    union
    {
        int jointid;
        int meshid;
        int skelid;
        taa_mat44 matrix;
        taa_vec4 rotate;
        taa_vec4 scale;
        taa_vec4 translate;
    } value;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE void taa_scenenode_calc_transform(
    const taa_scenenode* nodes,
    int nodeid,
    taa_mat44* m_out);

taa_SCENE_LINKAGE void taa_scenenode_create(
    const char* name,
    taa_scenenode_type type,
    int parentid,
    taa_scenenode* node_out);

taa_SCENE_LINKAGE void taa_scenenode_destroy(
    taa_scenenode* node);

/**
 * @brief change the up axis of the node
 * @details bakes a 90 degree rotation around the x axis into the node
 * @param dir +1 for counter-clockwise rotation, -1 for clockwise
 */
taa_SCENE_LINKAGE void taa_scenenode_rotate_upaxis(
    taa_scenenode* node,
    int32_t dir);

#endif // taa_SCENENODE_H_
