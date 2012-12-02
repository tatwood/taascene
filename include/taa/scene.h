/**
 * @brief     scene graph processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENE_H_
#define taa_SCENE_H_

#ifndef taa_SCENE_LINKAGE
#define taa_SCENE_LINKAGE
#endif

#include "sceneanim.h"
#include "scenematerial.h"
#include "scenemesh.h"
#include "sceneskel.h"
#include "scenetexture.h"

//****************************************************************************
// enums

enum taa_scene_upaxis_e
{
    taa_SCENE_Y_UP = 1,
    taa_SCENE_Z_UP = 2
};

//****************************************************************************
// typedefs

typedef enum taa_scene_upaxis_e taa_scene_upaxis;
typedef struct taa_scene_s taa_scene;

//****************************************************************************
// structs

struct taa_scene_s
{
    taa_scene_upaxis upaxis;
    uint32_t numanimations;
    uint32_t nummaterials;
    uint32_t nummeshes;
    uint32_t numskeletons;
    uint32_t numnodes;
    uint32_t numtextures;
    taa_sceneanim* animations;
    taa_scenematerial* materials;
    taa_scenemesh* meshes;
    taa_sceneskel* skeletons;
    taa_scenenode* nodes;
    taa_scenetexture* textures;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE int taa_scene_add_animation(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_add_material(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_add_mesh(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_add_node(
    taa_scene* scene,
    const char* name,
    taa_scenenode_type type,
    int parent);

taa_SCENE_LINKAGE int taa_scene_add_skeleton(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_add_texture(
    taa_scene* scene,
    const char* name,
    const char* path,
    taa_scenetexture_origin origin);

taa_SCENE_LINKAGE void taa_scene_convert_upaxis(
    taa_scene* scene,
    taa_scene_upaxis upaxis);

taa_SCENE_LINKAGE void taa_scene_create(
    taa_scene* scene,
    taa_scene_upaxis upaxis);

taa_SCENE_LINKAGE void taa_scene_destroy(
    taa_scene* scene);

taa_SCENE_LINKAGE int taa_scene_find_anim(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_find_material(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_find_mesh(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_find_node(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_find_skel(
    taa_scene* scene,
    const char* name);

taa_SCENE_LINKAGE int taa_scene_find_texture(
    taa_scene* scene,
    const char* name);

/**
 * @brief given a node id, finds the skeleton containing that node
 * @return the id of the skeleton on success, or -1 on error
 */
taa_SCENE_LINKAGE int taa_scene_find_skel_from_node(
    taa_scene* scene,
    int nodeid);

taa_SCENE_LINKAGE void taa_scene_resize_animations(
    taa_scene* scene,
    uint32_t numanims);

taa_SCENE_LINKAGE void taa_scene_resize_materials(
    taa_scene* scene,
    uint32_t nummats);

taa_SCENE_LINKAGE void taa_scene_resize_meshes(
    taa_scene* scene,
    uint32_t nummeshes);

taa_SCENE_LINKAGE void taa_scene_resize_nodes(
    taa_scene* scene,
    uint32_t numnodes);

taa_SCENE_LINKAGE void taa_scene_resize_skeletons(
    taa_scene* scene,
    uint32_t numskels);

taa_SCENE_LINKAGE void taa_scene_resize_textures(
    taa_scene* scene,
    uint32_t numtextures);

#endif // taa_SCENE_H_
