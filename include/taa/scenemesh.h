/**
 * @brief     mesh processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENEMESH_H_
#define taa_SCENEMESH_H_

#include <taa/system.h>
#include <taa/mat44.h>

//****************************************************************************
// constants

enum { taa_SCENEMESH_NAMESIZE = 32 };

//****************************************************************************
// enums

enum taa_scenemesh_usage_e
{
    taa_SCENEMESH_USAGE_BINORMAL,
    taa_SCENEMESH_USAGE_BLENDINDEX,
    taa_SCENEMESH_USAGE_BLENDWEIGHT,
    taa_SCENEMESH_USAGE_COLOR,
    taa_SCENEMESH_USAGE_FOG,
    taa_SCENEMESH_USAGE_NORMAL,
    taa_SCENEMESH_USAGE_POINTSIZE,
    taa_SCENEMESH_USAGE_POSITION,
    taa_SCENEMESH_USAGE_TANGENT,
    taa_SCENEMESH_USAGE_TEXCOORD,
    taa_SCENEMESH_USAGE_MERGED
};

enum taa_scenemesh_valuetype_e
{
    taa_SCENEMESH_VALUE_FLOAT32,
    taa_SCENEMESH_VALUE_FLOAT64,
    taa_SCENEMESH_VALUE_INT8,
    taa_SCENEMESH_VALUE_INT16,
    taa_SCENEMESH_VALUE_INT32,
    taa_SCENEMESH_VALUE_UINT8,
    taa_SCENEMESH_VALUE_UINT16,
    taa_SCENEMESH_VALUE_UINT32,
    taa_SCENEMESH_VALUE_MERGED
};

//****************************************************************************
// typedefs

typedef enum taa_scenemesh_usage_e taa_scenemesh_usage;
typedef enum taa_scenemesh_valuetype_e taa_scenemesh_valuetype;

typedef struct taa_scenemesh_vertformat_s taa_scenemesh_vertformat;
typedef struct taa_scenemesh_face_s taa_scenemesh_face;
typedef struct taa_scenemesh_skinjoint_s taa_scenemesh_skinjoint;
typedef struct taa_scenemesh_binding_s taa_scenemesh_binding;
typedef struct taa_scenemesh_stream_s taa_scenemesh_stream;
typedef struct taa_scenemesh_s taa_scenemesh;

//****************************************************************************
// structs

/**
 * @brief only used for specifying how to reformat meshes
 */
struct taa_scenemesh_vertformat_s
{
    /**
     * @brief name of the element
     */
    const char* name;
    /**
     * @brief usage hint
     */
    taa_scenemesh_usage usage;
    /**
     * @brief 0 for texcoord0, 1 for texcoord1, etc
     */
    uint32_t set;
    taa_scenemesh_valuetype valuetype;
    /**
     * @brief number of values per vertex (i.e. 3 for vec3)
     */
    uint32_t numcomponents;
    /**
     * @brief offset of the element in each vertex of the stream
     */
    uint32_t offset;
    /**
     * @brief vertex stream for the element
     */
    uint32_t stream;
};

struct taa_scenemesh_face_s
{
    uint32_t firstindex;
    uint32_t numindices;
    /**
     * @brief number of indices / indexSize
     */
    uint32_t numvertices;
};

struct taa_scenemesh_skinjoint_s
{
    /**
     * @brief index of joint in sceneskel instance
     */
    uint32_t animjoint;
    taa_mat44 invbindmatrix;
};

struct taa_scenemesh_binding_s
{
    char name[taa_SCENEMESH_NAMESIZE];
    int32_t materialid;
    uint32_t firstface;
    uint32_t numfaces;
};

struct taa_scenemesh_stream_s
{
    char name[taa_SCENEMESH_NAMESIZE];
    /**
     * @brief one of taa_scenemesh_usage enum
     */
    taa_scenemesh_usage usage;
    /**
     * @brief 0 for texcoord0, 1 for texcoord1, etc
     */
    uint32_t set;
    /**
     * @brief one of taa_scenemesh_valuetype enum
     */
    taa_scenemesh_valuetype valuetype;
    /**
     * @brief number of values per vertex (i.e. 3 for vec3)
     */
    uint32_t numcomponents;
    /**
     * @brief number of bytes per vertex
     */
    uint32_t stride;
    /**
     * @brief which vertex index refers to this vertex stream
     */
    uint32_t indexmapping;
    uint32_t numvertices;
    /**
     * @brief data buffer
     */
    uint8_t* buffer;
};

struct taa_scenemesh_s
{
    char name[taa_SCENEMESH_NAMESIZE];

    /**
     * @brief number of indices per vertex
     */
    int32_t indexsize;
    /**
     * @brief id of skeleton bound to skinning data
     */
    int32_t skeleton;

    uint32_t numjoints;
    uint32_t numfaces;
    uint32_t numbindings;
    uint32_t numstreams;
    uint32_t numindices;

    taa_scenemesh_skinjoint* joints;
    taa_scenemesh_face* faces;
    taa_scenemesh_binding* bindings;
    taa_scenemesh_stream* vertexstreams;
    uint32_t* indices;
};

//****************************************************************************
// functions

taa_SCENE_LINKAGE taa_scenemesh_face* taa_scenemesh_add_face(
    taa_scenemesh* mesh,
    const uint32_t* indices,
    uint32_t numindices,
    uint32_t numvertices);

taa_SCENE_LINKAGE int taa_scenemesh_add_skinjoint(
    taa_scenemesh* mesh,
    uint32_t animjoint,
    const taa_mat44* invbindmatrix);

taa_SCENE_LINKAGE int taa_scenemesh_add_stream(
    taa_scenemesh* mesh,
    const char* name,
    taa_scenemesh_usage usage,
    int set,
    taa_scenemesh_valuetype valuetype,
    int numcomponents,
    int stride,
    int indexmapping,
    uint32_t numvertices,
    const void* srcbuffer);

/**
 * @return the index of the new vertex
 */
taa_SCENE_LINKAGE int taa_scenemesh_add_vertex(
    taa_scenemesh_stream* vs,
    const void* vertdata);

taa_SCENE_LINKAGE void taa_scenemesh_begin_binding(
    taa_scenemesh* mesh,
    const char* name,
    int matid);

taa_SCENE_LINKAGE void taa_scenemesh_create(
    const char* name,
    taa_scenemesh* mesh_out);

taa_SCENE_LINKAGE void taa_scenemesh_destroy(
    taa_scenemesh* mesh);

taa_SCENE_LINKAGE void taa_scenemesh_end_binding(
    taa_scenemesh* mesh);

taa_SCENE_LINKAGE int taa_scenemesh_find_binding(
    taa_scenemesh* mesh,
    const char* name);

taa_SCENE_LINKAGE int taa_scenemesh_find_stream(
    taa_scenemesh* mesh,
    taa_scenemesh_usage usage,
    uint32_t set);

/**
 * @brief finds the index associated with the vertex data in the given stream
 * @param vs the vertex stream to search
 * @param vertdata the source vertex data to match.
 * @return the matching vertex index on success, or -1 on failure.
 */
taa_SCENE_LINKAGE int taa_scenemesh_find_vertex(
    const taa_scenemesh_stream* vs,
    const void* vertdata);

/**
 * Formats the mesh according to the soecified vertex definitions
 * <p>Since the formatting process likely involves merging streams
 * together, this function will also merge indices.</p>
 */
taa_SCENE_LINKAGE void taa_scenemesh_format(
    taa_scenemesh* mesh,
    const taa_scenemesh_vertformat* vf,
    int numvf);

taa_SCENE_LINKAGE void taa_scenemesh_format_stream(
    taa_scenemesh_stream* vs,
    taa_scenemesh_valuetype valuetype,
    unsigned int numcomponents);

/**
 * Merges vertex indices together so that one index exists per vertex
 * <p>If vertex streams have independent indices, this function will rearrange
 * vertices into unique combinations of values so that they can all share one
 * index per vertex. This function cannot be called after streams have been
 * merged. Unnecessary streams should be removed before calling this function
 * to prevent duplicate vertices in the final mesh.</p>
 */
taa_SCENE_LINKAGE void taa_scenemesh_merge_indices(
    taa_scenemesh* mesh);

taa_SCENE_LINKAGE int taa_scenemesh_merge_streams(
    taa_scenemesh* mesh,
    const char* name,
    taa_scenemesh_stream** streams,
    const uint32_t* streamoffsets,
    uint32_t numstreams);

taa_SCENE_LINKAGE void taa_scenemesh_remove_stream(
    taa_scenemesh* mesh,
    taa_scenemesh_stream* vs);

taa_SCENE_LINKAGE void taa_scenemesh_resize_bindings(
    taa_scenemesh* mesh,
    uint32_t numbindings);

taa_SCENE_LINKAGE void taa_scenemesh_resize_faces(
    taa_scenemesh* mesh,
    uint32_t numfaces);

taa_SCENE_LINKAGE void taa_scenemesh_resize_indices(
    taa_scenemesh* mesh,
    uint32_t numindices);

taa_SCENE_LINKAGE void taa_scenemesh_resize_skinjoints(
    taa_scenemesh* mesh,
    uint32_t numjoints);

taa_SCENE_LINKAGE void taa_scenemesh_resize_streams(
    taa_scenemesh* mesh,
    uint32_t numstreams);

taa_SCENE_LINKAGE void taa_scenemesh_resize_vertices(
    taa_scenemesh_stream* vs,
    uint32_t stride,
    uint32_t numverts);

/**
 * @brief change the up axis of the mesh
 * @details bakes a 90 degree rotation around the x axis into the mesh
 * @param dir +1 for counter-clockwise rotation, -1 for clockwise
 */
taa_SCENE_LINKAGE void taa_scenemesh_rotate_upaxis(
    taa_scenemesh* mesh,
    int32_t dir);

taa_SCENE_LINKAGE void taa_scenemesh_triangulate(
    taa_scenemesh* mesh);

#endif // taa_SCENEMESH_H_
