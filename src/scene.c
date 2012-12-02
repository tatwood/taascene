/**
 * @brief     scene graph processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/scene.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

enum
{
    taa_SCENE_CHUNK = 16
};

//****************************************************************************
int32_t taa_scene_add_animation(
    taa_scene* scene,
    const char* name)
{
    uint32_t animid = scene->numanimations;
    taa_sceneanim* anim;
    taa_scene_resize_animations(scene, animid + 1);
    anim = scene->animations + animid;
    strncpy(anim->name, name, sizeof(anim->name));
    anim->name[sizeof(anim->name)-1] = '\0';
    return animid;
}

//****************************************************************************
int32_t taa_scene_add_material(
    taa_scene* scene,
    const char* name)
{
    uint32_t matid = scene->nummaterials;
    taa_scenematerial* mat;
    taa_scene_resize_materials(scene, matid + 1);
    mat = scene->materials + matid;
    strncpy(mat->name, name, sizeof(mat->name));
    mat->name[sizeof(mat->name)-1] = '\0';
    return matid;
}

//****************************************************************************
int32_t taa_scene_add_mesh(
    taa_scene* scene,
    const char* name)
{
    uint32_t meshid = scene->nummeshes;
    taa_scenemesh* mesh;
    taa_scene_resize_meshes(scene, meshid + 1);
    mesh = scene->meshes + meshid;
    strncpy(mesh->name, name, sizeof(mesh->name));
    mesh->name[sizeof(mesh->name)-1] = '\0';
    return meshid;
}

//****************************************************************************
int32_t taa_scene_add_node(
    taa_scene* scene,
    const char* name,
    taa_scenenode_type type,
    int32_t parent)
{
    uint32_t nodeid = scene->numnodes;
    taa_scenenode* node;
    taa_scene_resize_nodes(scene, nodeid + 1);
    node = scene->nodes + nodeid;
    strncpy(node->name, name, sizeof(node->name));
    node->name[sizeof(node->name)-1] = '\0';
    node->type = type;
    node->parent = parent;
    return nodeid;
}

//****************************************************************************
int32_t taa_scene_add_skeleton(
    taa_scene* scene,
    const char* name)
{
    uint32_t skelid = scene->numskeletons;
    taa_sceneskel* skel;
    taa_scene_resize_skeletons(scene, skelid + 1);
    skel = scene->skeletons + skelid;
    strncpy(skel->name, name, sizeof(skel->name));
    skel->name[sizeof(skel->name)-1] = '\0';
    return skelid;
}

//****************************************************************************
int32_t taa_scene_add_texture(
    taa_scene* scene,
    const char* name,
    const char* path,
    taa_scenetexture_origin origin)
{
    uint32_t texid = scene->numtextures;
    taa_scenetexture* texture;
    taa_scene_resize_textures(scene, texid + 1);
    texture = scene->textures + texid;
    strncpy(texture->name, name, sizeof(texture->name));
    texture->name[sizeof(texture->name)-1] = '\0';
    strncpy(texture->path, path, sizeof(texture->path));
    texture->path[sizeof(texture->path)-1] = '\0';
    texture->origin = origin;
    return texid;
}

//****************************************************************************
void taa_scene_convert_upaxis(
    taa_scene* scene,
    taa_scene_upaxis upaxis)
{ 
    if(upaxis != scene->upaxis)
    {
        taa_sceneanim* animitr = scene->animations;
        taa_sceneanim* animend = animitr + scene->numanimations;
        taa_scenemesh* meshitr = scene->meshes;
        taa_scenemesh* meshend = meshitr + scene->nummeshes;
        taa_scenenode* nodeitr = scene->nodes;
        taa_scenenode* nodeend = nodeitr + scene->numnodes;
        int32_t dir = (upaxis == taa_SCENE_Y_UP) ? +1 : -1;
        while(animitr != animend)
        {
            taa_sceneanim_rotate_upaxis(
                animitr,
                scene->nodes,
                scene->numnodes,
                dir);
            ++animitr;
        }
        while(meshitr != meshend)
        {
            taa_scenemesh_rotate_upaxis(meshitr, dir);
            ++meshitr;
        }
        while(nodeitr != nodeend)
        {
            taa_scenenode_rotate_upaxis(nodeitr, dir);
            ++nodeitr;
        }
        scene->upaxis = upaxis;
    }
}

//****************************************************************************
void taa_scene_create(
    taa_scene* scene,
    taa_scene_upaxis upaxis)
{
    memset(scene, 0, sizeof(*scene));
    scene->upaxis = upaxis;
}

//****************************************************************************
void taa_scene_destroy(
    taa_scene* scene)
{
    taa_sceneanim* animitr = scene->animations;
    taa_sceneanim* animend = animitr + scene->numanimations;
    taa_scenematerial* matitr = scene->materials;
    taa_scenematerial* matend = matitr + scene->nummaterials;
    taa_scenemesh* meshitr = scene->meshes;
    taa_scenemesh* meshend = scene->meshes + scene->nummeshes;
    taa_scenenode* nodeitr = scene->nodes;
    taa_scenenode* nodeend = scene->nodes + scene->numnodes;
    taa_sceneskel* skelitr = scene->skeletons;
    taa_sceneskel* skelend = skelitr + scene->numskeletons;
    taa_scenetexture* textureitr = scene->textures;
    taa_scenetexture* textureend = textureitr + scene->numtextures;
    while(animitr != animend)
    {
        taa_sceneanim_destroy(animitr);
        ++animitr;
    }
    while(matitr != matend)
    {
        taa_scenematerial_destroy(matitr);
        ++matitr;
    }
    while(meshitr != meshend)
    {
        taa_scenemesh_destroy(meshitr);
        ++meshitr;
    }
    while(nodeitr != nodeend)
    {
         taa_scenenode_destroy(nodeitr);
        ++nodeitr;
    }
    while(skelitr != skelend)
    {
         taa_sceneskel_destroy(skelitr);
        ++skelitr;
    }
    while(textureitr != textureend)
    {
         taa_scenetexture_destroy(textureitr);
        ++textureitr;
    }
    taa_memalign_free(scene->nodes);
    free(scene->animations);
    free(scene->materials);
    free(scene->meshes);
    free(scene->skeletons);
    free(scene->textures);
}

//****************************************************************************
int taa_scene_find_anim(
    taa_scene* scene,
    const char* name)
{
    int animid = -1;
    taa_sceneanim* animitr = scene->animations;
    taa_sceneanim* animend = animitr + scene->numanimations;
    while(animitr != animend)
    {
        if(!strcmp(animitr->name, name))
        {
            animid = (int32_t)(ptrdiff_t) (animitr - scene->animations);
            break;
        }
        ++animitr;
    }
    return animid;
}

//****************************************************************************
int taa_scenemesh_find_material(
    taa_scene* scene,
    const char* name)
{
    int matid = -1;
    taa_scenematerial* matitr = scene->materials;
    taa_scenematerial* matend = matitr + scene->nummaterials;
    while(matitr != matend)
    {
        if(!strcmp(matitr->name, name))
        {
            matid = (int32_t)(ptrdiff_t) (matitr - scene->materials);
        }
        ++matitr;
    }
    return matid;
}

//****************************************************************************
int taa_scene_find_mesh(
    taa_scene* scene,
    const char* name)
{
    int meshid = -1;
    taa_scenemesh* meshitr = scene->meshes;
    taa_scenemesh* meshend = meshitr + scene->nummeshes;
    while(meshitr != meshend)
    {
        if(!strcmp(meshitr->name, name))
        {
            meshid = (int32_t)(ptrdiff_t) (meshitr - scene->meshes);
            break;
        }
        ++meshitr;
    }
    return meshid;
}

//****************************************************************************
int taa_scene_find_node(
    taa_scene* scene,
    const char* name)
{
    int nodeid = -1;
    taa_scenenode* nodeitr = scene->nodes;
    taa_scenenode* nodeend = nodeitr + scene->numnodes;
    while(nodeitr != nodeend)
    {
        if(!strcmp(nodeitr->name, name))
        {
            nodeid = (int32_t)(ptrdiff_t) (nodeitr - scene->nodes);
            break;
        }
        ++nodeitr;
    }
    return nodeid;
}

//****************************************************************************
int taa_scene_find_skel(
    taa_scene* scene,
    const char* name)
{
    int skelid = -1;
    taa_sceneskel* skelitr = scene->skeletons;
    taa_sceneskel* skelend = skelitr + scene->numskeletons;
    while(skelitr != skelend)
    {
        if(!strcmp(skelitr->name, name))
        {
            skelid = (int32_t)(ptrdiff_t) (skelitr - scene->skeletons);
            break;
        }
        ++skelitr;
    }
    return skelid;
}

//****************************************************************************
int taa_scene_find_texture(
    taa_scene* scene,
    const char* name)
{
    int32_t textureid = -1;
    taa_scenetexture* textureitr = scene->textures;
    taa_scenetexture* textureend = textureitr + scene->numtextures;
    while(textureitr != textureend)
    {
        if(!strcmp(textureitr->name, name))
        {
            textureid = (int32_t)(ptrdiff_t) (textureitr - scene->textures);
            break;
        }
        ++textureitr;
    }
    return textureid;
}

//****************************************************************************
int taa_scene_find_skel_from_node(
    taa_scene* scene,
    int nodeid)
{
    int skelid = -1;
    const taa_scenenode* node = scene->nodes + nodeid;
    taa_sceneskel* skelitr = scene->skeletons;
    taa_sceneskel* skelend = skelitr + scene->numskeletons;
    assert(node->type == taa_SCENENODE_REF_JOINT);
    while(skelitr != skelend)
    {
        if(((size_t) node->value.jointid) < skelitr->numjoints)
        {
            if(skelitr->joints[node->value.jointid].nodeid == nodeid)
            {
                skelid = (int32_t)(ptrdiff_t) (skelitr - scene->skeletons);
                break;
            }
        }
        ++skelitr;
    }
    return skelid;
}

//****************************************************************************
void taa_scene_resize_animations(
    taa_scene* scene,
    uint32_t numanims)
{
    uint32_t oldnum = scene->numanimations;
    taa_sceneanim* anim = scene->animations;
    taa_sceneanim* animend;
    if(numanims > oldnum)
    {
        uint32_t cap = (oldnum  +(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        uint32_t ncap= (numanims+(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        if(cap != ncap)
        {
            anim = (taa_sceneanim*) realloc(anim, ncap * sizeof(*anim));
            scene->animations = anim;
        }
        animend = anim + numanims;
        anim += oldnum;
        while(anim != animend)
        {
            taa_sceneanim_create("", anim);
            ++anim;
        }
        scene->numanimations = numanims;
    }
}

//****************************************************************************
void taa_scene_resize_materials(
    taa_scene* scene,
    uint32_t nummats)
{
    uint32_t oldnum = scene->nummaterials;
    taa_scenematerial* mat = scene->materials;
    taa_scenematerial* matend;
    if(nummats > oldnum)
    {
        uint32_t cap = (oldnum +(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        uint32_t ncap= (nummats+(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        if(cap != ncap)
        {
            mat = (taa_scenematerial*) realloc(mat, ncap * sizeof(*mat));
            scene->materials = mat;
        }
        matend = mat + nummats;
        mat += oldnum;
        while(mat != matend)
        {
            taa_scenematerial_create("", mat);
            ++mat;
        }
        scene->nummaterials = nummats;
    }
}

//****************************************************************************
void taa_scene_resize_meshes(
    taa_scene* scene,
    uint32_t nummeshes)
{
    uint32_t oldnum = scene->nummeshes;
    taa_scenemesh* mesh = scene->meshes;
    taa_scenemesh* meshend;
    if(nummeshes > oldnum)
    {
        uint32_t cap = (oldnum   +(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        uint32_t ncap= (nummeshes+(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        if(cap != ncap)
        {
            mesh = (taa_scenemesh*) realloc(mesh, ncap * sizeof(*mesh));
            scene->meshes = mesh;
        }
        meshend = mesh + nummeshes;
        mesh += oldnum;
        while(mesh != meshend)
        {
            taa_scenemesh_create("", mesh);
            ++mesh;
        }
        scene->nummeshes = nummeshes;
    }
}

//****************************************************************************
void taa_scene_resize_nodes(
    taa_scene* scene,
    uint32_t numnodes)
{
    uint32_t oldnum = scene->numnodes;
    taa_scenenode* node = scene->nodes;
    uint32_t ocap = (oldnum  +(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
    uint32_t ncap = (numnodes+(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
    if(ocap != ncap)
    {
        node = (taa_scenenode*) taa_memalign(16, ncap * sizeof(*node));
        if(scene->nodes != NULL)
        {
            memcpy(node, scene->nodes, ocap * sizeof(*node));
            taa_memalign_free(scene->nodes);
        }
        scene->nodes = node;
    }    
    if(numnodes > oldnum)
    {
        taa_scenenode* nodeend = node + numnodes;
        node += oldnum;
        while(node != nodeend)
        {
            taa_scenenode_create("", taa_SCENENODE_EMPTY, -1, node);
            ++node;
        }
        scene->numnodes = numnodes;
    }
}

//****************************************************************************
void taa_scene_resize_skeletons(
    taa_scene* scene,
    uint32_t numskels)
{
    uint32_t oldnum = scene->numskeletons;
    taa_sceneskel* skel = scene->skeletons;
    taa_sceneskel* skelend;
    if(numskels > oldnum)
    {
        uint32_t cap = (oldnum  +(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        uint32_t ncap= (numskels+(taa_SCENE_CHUNK-1)) & ~(taa_SCENE_CHUNK-1);
        if(cap != ncap)
        {
            skel = (taa_sceneskel*) realloc(skel, ncap * sizeof(*skel));
            scene->skeletons = skel;
        }
        skelend = skel + numskels;
        skel += oldnum;
        while(skel != skelend)
        {
            taa_sceneskel_create("", skel);
            ++skel;
        }
        scene->numskeletons = numskels;
    }
}

//****************************************************************************
void taa_scene_resize_textures(
    taa_scene* scene,
    uint32_t numtextures)
{
    uint32_t oldnum = scene->numtextures;
    taa_scenetexture* tex = scene->textures;
    taa_scenetexture* texend;
    if(numtextures > oldnum)
    {
        uint32_t cap = (oldnum     +(taa_SCENE_CHUNK-1))&~(taa_SCENE_CHUNK-1);
        uint32_t ncap= (numtextures+(taa_SCENE_CHUNK-1))&~(taa_SCENE_CHUNK-1);
        if(cap != ncap)
        {
            tex = (taa_scenetexture*) realloc(tex, ncap * sizeof(*tex));
            scene->textures = tex;
        }
        texend = tex + numtextures;
        tex += oldnum;
        while(tex != texend)
        {
            taa_scenetexture_create("", "", taa_SCENETEXTURE_BOTTOMLEFT, tex);
            ++tex;
        }
        scene->numtextures = numtextures;
    }
}
