/**
 * @brief     scene graph file format implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/scenefile.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define taa_SCENEFILE_MAGIC ( \
    (((uint64_t) 't') <<  0) | \
    (((uint64_t) 'a') <<  8) | \
    (((uint64_t) 'a') << 16) | \
    (((uint64_t) 'S') << 24) | \
    (((uint64_t) 'C') << 32) | \
    (((uint64_t) 'E') << 40) | \
    (((uint64_t) 'N') << 48) | \
    (((uint64_t) 'E') << 56) )

//****************************************************************************
static int32_t taa_scenefile_deserialize_animation(
    taa_filestream* fs,
    taa_sceneanim* anim)
{
    int32_t err = 0;
    uint32_t numchannels;
    err |= taa_filestream_read_i8n(fs, anim->name, sizeof(anim->name));
    err |= taa_filestream_read_f32(fs, &anim->length);
    err |= taa_filestream_read_i32(fs, &numchannels);
    if(err == 0)
    {
        taa_sceneanim_channel* chanitr;
        taa_sceneanim_channel* chanend;
        taa_sceneanim_resize_channels(anim, numchannels);
        chanitr = anim->channels;
        chanend = chanitr + anim->numchannels;
        while(chanitr != chanend && err == 0)
        {
            taa_sceneanim_keyframe* kfitr;
            taa_sceneanim_keyframe* kfend;
            uint32_t numkeyframes;
            err |= taa_filestream_read_i32(fs, &chanitr->nodeid);
            err |= taa_filestream_read_i32(fs, &chanitr->numcomponents);
            err |= taa_filestream_read_i8n(fs, chanitr->componentmask, 16);
            err |= taa_filestream_read_i32(fs, &numkeyframes);
            taa_sceneanim_resize_frames(chanitr, numkeyframes);
            kfitr = chanitr->keyframes;
            kfend = kfitr + numkeyframes;
            while(kfitr != kfend && err == 0)
            {
                err |= taa_filestream_read_f32(fs, &kfitr->interpolation);
                err |= taa_filestream_read_f32(fs, &kfitr->time);
                err |= taa_filestream_read_f32n(fs, kfitr->values, 16);
                err |= taa_filestream_read_f32n(fs, &kfitr->cpin.x, 2);
                err |= taa_filestream_read_f32n(fs, &kfitr->cpout.x, 2);
                ++kfitr;
            }
            ++chanitr;
        }
    }
    return err;
}

//****************************************************************************
static int32_t taa_scenefile_deserialize_material(
    taa_filestream* fs,
    taa_scenematerial* mat)
{
    int32_t err = 0;
    err|=taa_filestream_read_i8n(fs, mat->name, sizeof(mat->name));
    err|=taa_filestream_read_i32(fs, &mat->diffusetexture);
    return err;
}

//****************************************************************************
static int32_t taa_scenefile_deserialize_mesh(
    taa_filestream* fs,
    taa_scenemesh* mesh)
{
    int32_t err = 0;
    uint32_t numjoints;
    uint32_t numfaces;
    uint32_t numbindings;
    uint32_t numstreams;
    uint32_t numindices;
    err |= taa_filestream_read_i8n(fs, mesh->name, sizeof(mesh->name));
    err |= taa_filestream_read_i32(fs, &mesh->indexsize);
    err |= taa_filestream_read_i32(fs, &mesh->skeleton);
    err |= taa_filestream_read_i32(fs, &numjoints);
    err |= taa_filestream_read_i32(fs, &numfaces);
    err |= taa_filestream_read_i32(fs, &numbindings);
    err |= taa_filestream_read_i32(fs, &numstreams);
    err |= taa_filestream_read_i32(fs, &numindices);
    if(err == 0)
    {
        taa_scenemesh_skinjoint* jointitr;
        taa_scenemesh_skinjoint* jointend;
        taa_scenemesh_resize_skinjoints(mesh, numjoints);
        jointitr = mesh->joints;
        jointend = jointitr + numjoints;
        while(jointitr != jointend && err == 0)
        {
            err|=taa_filestream_read_i32(fs, &jointitr->animjoint);
            err|=taa_filestream_read_f32n(fs,&jointitr->invbindmatrix.x.x,16);
            ++jointitr;
        }
    }
    if(err == 0)
    {
        taa_scenemesh_face* faceitr;
        taa_scenemesh_face* faceend;
        taa_scenemesh_resize_faces(mesh, numfaces);
        faceitr = mesh->faces;
        faceend = faceitr + numfaces;
        while(faceitr != faceend && err == 0)
        {
            err |= taa_filestream_read_i32(fs, &faceitr->firstindex);
            err |= taa_filestream_read_i32(fs, &faceitr->numindices);
            err |= taa_filestream_read_i32(fs, &faceitr->numvertices);
            ++faceitr;
        }
    }
    if(err == 0)
    {
        taa_scenemesh_binding* binditr;
        taa_scenemesh_binding* bindend;
        taa_scenemesh_resize_bindings(mesh, numbindings);
        binditr = mesh->bindings;
        bindend = binditr + numbindings;
        while(binditr != bindend && err == 0)
        {
            err |= taa_filestream_read_i8n(
                fs,
                binditr->name,
                sizeof(binditr->name));
            err |= taa_filestream_read_i32(fs, &binditr->materialid);
            err |= taa_filestream_read_i32(fs, &binditr->firstface);
            err |= taa_filestream_read_i32(fs, &binditr->numfaces);
            ++binditr;
        }
    }
    if(err == 0)
    {
        taa_scenemesh_stream* vsitr;
        taa_scenemesh_stream* vsend;
        taa_scenemesh_resize_streams(mesh, numstreams);
        vsitr = mesh->vertexstreams;
        vsend = vsitr + mesh->numstreams;
        while(vsitr != vsend && err == 0)
        {
            int32_t valuesize;
            int32_t stride;
            uint32_t numvertices;
            err|=taa_filestream_read_i8n(fs, vsitr->name,sizeof(vsitr->name));
            err|=taa_filestream_read_i32(fs, &vsitr->usage);
            err|=taa_filestream_read_i32(fs, &vsitr->set);
            err|=taa_filestream_read_i32(fs, &vsitr->valuetype);
            err|=taa_filestream_read_i32(fs, &vsitr->numcomponents);
            err|=taa_filestream_read_i32(fs, &vsitr->indexmapping);
            err|=taa_filestream_read_i32(fs, &stride);
            err|=taa_filestream_read_i32(fs, &numvertices);
            taa_scenemesh_resize_vertices(vsitr, stride, numvertices);
            valuesize = vsitr->stride / vsitr->numcomponents;
            switch(valuesize)
            {
            case 2:
                err |= taa_filestream_read_i16n(
                    fs,
                    vsitr->buffer,
                    numvertices * vsitr->numcomponents);
                break;
            case 4:
                err |= taa_filestream_read_i32n(
                    fs,
                    vsitr->buffer,
                    numvertices * vsitr->numcomponents);
                break;
            case 8:
                err |= taa_filestream_read_i64n(
                    fs,
                    vsitr->buffer,
                    numvertices * vsitr->numcomponents);
                break;
            default:
                err |= taa_filestream_read_i8n(
                    fs,
                    vsitr->buffer,
                    numvertices * vsitr->stride);
                break;
            }
            ++vsitr;
        }
    }
    if(err == 0)
    {
        taa_scenemesh_resize_indices(mesh, numindices);
        err |= taa_filestream_read_i32n(fs, mesh->indices, numindices);
    }
    return err;
}

//****************************************************************************
static int32_t taa_scenefile_deserialize_node(
    taa_filestream* fs,
    taa_scenenode* node)
{
    int32_t err = 0;
    err |= taa_filestream_read_i8n(fs, node->name, sizeof(node->name));
    err |= taa_filestream_read_i32(fs, &node->type);
    err |= taa_filestream_read_i32(fs, &node->parent);
    err |= taa_filestream_read_i32n(fs, &node->value, 16);
    return err;
}

//****************************************************************************
static int32_t taa_scenefile_deserialize_skeleton(
    taa_filestream* fs,
    taa_sceneskel* skel)
{
    int32_t err = 0;
    uint32_t numjoints;
    err |= taa_filestream_read_i8n(fs, skel->name, sizeof(skel->name));
    err |= taa_filestream_read_i32(fs, &numjoints);
    if(err == 0)
    {
        taa_sceneskel_joint* jointitr;
        taa_sceneskel_joint* jointend;
        taa_sceneskel_resize_joints(skel, numjoints);
        jointitr = skel->joints;
        jointend = jointitr + numjoints;
        while(jointitr != jointend && err == 0)
        {
            err |= taa_filestream_read_i32(fs, &jointitr->parent);
            err |= taa_filestream_read_i32(fs, &jointitr->nodeid);
            ++jointitr;
        }
    }
    return err;
}

//****************************************************************************
static int32_t taa_scenefile_deserialize_texture(
    taa_filestream* fs,
    taa_scenetexture* tex)
{
    int32_t err = 0;
    err |= taa_filestream_read_i8n(fs, tex->name, sizeof(tex->name));
    err |= taa_filestream_read_i8n(fs, tex->path, sizeof(tex->path));
    err |= taa_filestream_read_i32(fs, &tex->origin);
    if(err == 0)
    {
        uint32_t numlevels;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t bpp;
        uint32_t format;
        err |= taa_filestream_read_i32(fs, &numlevels);
        err |= taa_filestream_read_i32(fs, &width);
        err |= taa_filestream_read_i32(fs, &height);
        err |= taa_filestream_read_i32(fs, &depth);
        err |= taa_filestream_read_i32(fs, &bpp);
        err |= taa_filestream_read_i32(fs, &format);
        taa_scenetexture_resize(
            tex,
            numlevels,
            width,
            height,
            depth,
            (taa_scenetexture_format) format);
        if(tex->bpp != bpp)
        {
            err = -1;
        }
    }
    if(err == 0)
    {
        void** levelitr = tex->images;
        void** levelend = levelitr + tex->numlevels;
        uint32_t w = tex->width;
        uint32_t h = tex->height;
        uint32_t d = tex->depth;
        while(levelitr != levelend && err == 0)
        {
            uint32_t imagesize = w * h * d * tex->bpp/8;
            err |= taa_filestream_read_i8n(fs, *levelitr, imagesize);
            w >>= 1;
            h >>= 1;
            d >>= 1;
            ++levelitr;
        }
    }
    return err;
}

//****************************************************************************
static void taa_scenefile_serialize_animation(
    const taa_sceneanim* anim,
    taa_filestream* fs)
{
    const taa_sceneanim_channel* chanitr = anim->channels;
    const taa_sceneanim_channel* chanend = chanitr + anim->numchannels;
    taa_filestream_write_i8n(fs, anim->name, sizeof(anim->name));
    taa_filestream_write_f32(fs, anim->length);
    taa_filestream_write_i32(fs, anim->numchannels);
    while(chanitr != chanend)
    {
        const taa_sceneanim_keyframe* kfitr = chanitr->keyframes;
        const taa_sceneanim_keyframe* kfend = kfitr+chanitr->numkeyframes;
        taa_filestream_write_i32(fs, chanitr->nodeid);
        taa_filestream_write_i32(fs, chanitr->numcomponents);
        taa_filestream_write_i8n(fs, chanitr->componentmask, 16);
        taa_filestream_write_i32(fs, chanitr->numkeyframes);
        while(kfitr != kfend)
        {
            taa_filestream_write_i32(fs, kfitr->interpolation);
            taa_filestream_write_f32(fs, kfitr->time);
            taa_filestream_write_f32n(fs, kfitr->values, 16);
            taa_filestream_write_f32n(fs, &kfitr->cpin.x, 2);
            taa_filestream_write_f32n(fs, &kfitr->cpout.x, 2);
            ++kfitr;
        }
        ++chanitr;
    }
}

//****************************************************************************
static void taa_scenefile_serialize_material(
    const taa_scenematerial* mat,
    taa_filestream* fs)
{
    taa_filestream_write_i8n(fs, mat->name, sizeof(mat->name));
    taa_filestream_write_i32(fs, mat->diffusetexture);
}

//****************************************************************************
static void taa_scenefile_serialize_mesh(
    const taa_scenemesh* mesh,
    taa_filestream* fs)
{
    const taa_scenemesh_skinjoint* jointitr = mesh->joints;
    const taa_scenemesh_skinjoint* jointend = jointitr + mesh->numjoints;
    const taa_scenemesh_face* faceitr = mesh->faces;
    const taa_scenemesh_face* faceend = faceitr + mesh->numfaces;
    const taa_scenemesh_binding* binditr = mesh->bindings;
    const taa_scenemesh_binding* bindend = binditr + mesh->numbindings;
    const taa_scenemesh_stream* vsitr = mesh->vertexstreams;
    const taa_scenemesh_stream* vsend = vsitr + mesh->numstreams;
    taa_filestream_write_i8n(fs, mesh->name, sizeof(mesh->name));
    taa_filestream_write_i32(fs, mesh->indexsize);
    taa_filestream_write_i32(fs, mesh->skeleton);
    taa_filestream_write_i32(fs, mesh->numjoints);
    taa_filestream_write_i32(fs, mesh->numfaces);
    taa_filestream_write_i32(fs, mesh->numbindings);
    taa_filestream_write_i32(fs, mesh->numstreams);
    taa_filestream_write_i32(fs, mesh->numindices);
    while(jointitr != jointend)
    {
        taa_filestream_write_i32(fs, jointitr->animjoint);
        taa_filestream_write_f32n(fs, &jointitr->invbindmatrix.x.x, 16);
        ++jointitr;
    }
    while(faceitr != faceend)
    {
        taa_filestream_write_i32(fs, faceitr->firstindex);
        taa_filestream_write_i32(fs, faceitr->numindices);
        taa_filestream_write_i32(fs, faceitr->numvertices);
        ++faceitr;
    }
    while(binditr != bindend)
    {
        taa_filestream_write_i8n(fs, binditr->name, sizeof(binditr->name));
        taa_filestream_write_i32(fs, binditr->materialid);
        taa_filestream_write_i32(fs, binditr->firstface);
        taa_filestream_write_i32(fs, binditr->numfaces);
        ++binditr;
    }
    while(vsitr != vsend)
    {
        int32_t valuesize = vsitr->stride / vsitr->numcomponents;
        taa_filestream_write_i8n(fs, vsitr->name,sizeof(vsitr->name));
        taa_filestream_write_i32(fs, vsitr->usage);
        taa_filestream_write_i32(fs, vsitr->set);
        taa_filestream_write_i32(fs, vsitr->valuetype);
        taa_filestream_write_i32(fs, vsitr->numcomponents);
        taa_filestream_write_i32(fs, vsitr->indexmapping);
        taa_filestream_write_i32(fs, vsitr->stride);
        taa_filestream_write_i32(fs, vsitr->numvertices);
        switch(valuesize)
        {
        case 2:
            taa_filestream_write_i16n(
                fs,
                vsitr->buffer,
                vsitr->numvertices * vsitr->numcomponents);
            break;
        case 4:
            taa_filestream_write_i32n(
                fs,
                vsitr->buffer,
                vsitr->numvertices * vsitr->numcomponents);
            break;
        case 8:
            taa_filestream_write_i64n(
                fs,
                vsitr->buffer,
                vsitr->numvertices * vsitr->numcomponents);
            break;
        default:
            taa_filestream_write_i8n(
                fs,
                vsitr->buffer,
                vsitr->numvertices * vsitr->stride);
            break;
        }
        ++vsitr;
    }
    taa_filestream_write_i32n(fs, mesh->indices, mesh->numindices);
}

//****************************************************************************
static void taa_scenefile_serialize_node(
    const taa_scenenode* node,
    taa_filestream* fs)
{
    taa_filestream_write_i8n(fs, node->name, sizeof(node->name));
    taa_filestream_write_i32(fs, node->type);
    taa_filestream_write_i32(fs, node->parent);
    taa_filestream_write_i32n(fs, &node->value, 16);
}

//****************************************************************************
static void taa_scenefile_serialize_skeleton(
    const taa_sceneskel* skel,
    taa_filestream* fs)
{
    const taa_sceneskel_joint* jointitr = skel->joints;
    const taa_sceneskel_joint* jointend = jointitr + skel->numjoints;
    taa_filestream_write_i8n(fs, skel->name, sizeof(skel->name));
    taa_filestream_write_i32(fs, skel->numjoints);
    while(jointitr != jointend)
    {
        taa_filestream_write_i32(fs, jointitr->parent);
        taa_filestream_write_i32(fs, jointitr->nodeid);
        ++jointitr;
    }
}

//****************************************************************************
static void taa_scenefile_serialize_texture(
    const taa_scenetexture* tex,
    taa_filestream* fs)
{
    void** levelitr = tex->images;
    void** levelend = levelitr + tex->numlevels;
    uint32_t w = tex->width;
    uint32_t h = tex->height;
    uint32_t d = tex->depth;
    taa_filestream_write_i8n(fs, tex->name, sizeof(tex->name));
    taa_filestream_write_i8n(fs, tex->path, sizeof(tex->path));
    taa_filestream_write_i32(fs, tex->origin);
    taa_filestream_write_i32(fs, tex->numlevels);
    taa_filestream_write_i32(fs, tex->width);
    taa_filestream_write_i32(fs, tex->height);
    taa_filestream_write_i32(fs, tex->depth);
    taa_filestream_write_i32(fs, tex->bpp);
    taa_filestream_write_i32(fs, tex->format);
    while(levelitr != levelend)
    {
        uint32_t imagesize = w * h * d * tex->bpp/8;
        taa_filestream_write_i8n(fs, *levelitr, imagesize);
        w >>= 1;
        h >>= 1;
        d >>= 1;
        ++levelitr;
    }
}

//****************************************************************************
int32_t taa_scenefile_deserialize(
    taa_filestream* fs,
    taa_scene* scene)
{
    int32_t err = 0;
    uint64_t magic;
    uint32_t version;
    uint32_t numanimations;
    uint32_t nummaterials;
    uint32_t nummeshes;
    uint32_t numnodes;
    uint32_t numskeletons;
    uint32_t numtextures;
    err = taa_filestream_read_i64(fs, &magic);
    if(err == 0)
    {
        err = (magic == taa_SCENEFILE_MAGIC) ? 0 : -1;
    }
    if(err == 0)
    {
        err = taa_filestream_read_i32(fs, &version);
        if(err == 0)
        {
            err = (version == 0) ? 0 : -1;
        }
    }
    if(err == 0)
    {
        err |= taa_filestream_read_i32(fs, &scene->upaxis);
        err |= taa_filestream_read_i32(fs, &numanimations);
        err |= taa_filestream_read_i32(fs, &nummaterials);
        err |= taa_filestream_read_i32(fs, &nummeshes);
        err |= taa_filestream_read_i32(fs, &numnodes);
        err |= taa_filestream_read_i32(fs, &numskeletons);
        err |= taa_filestream_read_i32(fs, &numtextures);
    }
    if(err == 0)
    {
        taa_sceneanim* animitr;
        taa_sceneanim* animend;
        taa_scene_resize_animations(scene, numanimations);
        animitr = scene->animations;
        animend = animitr + numanimations;
        while(animitr != animend && err == 0)
        {
            err = taa_scenefile_deserialize_animation(fs, animitr);
            ++animitr;
        }
    }
    if(err == 0)
    {
        taa_scenematerial* matitr;
        taa_scenematerial* matend;
        taa_scene_resize_materials(scene, nummaterials);
        matitr = scene->materials;
        matend = matitr + nummaterials;
        while(matitr != matend && err == 0)
        {
            err = taa_scenefile_deserialize_material(fs, matitr);
            ++matitr;
        }
    }
    if(err == 0)
    {
        taa_scenemesh* meshitr;
        taa_scenemesh* meshend;
        taa_scene_resize_meshes(scene, nummeshes);
        meshitr = scene->meshes;
        meshend = meshitr + nummeshes;
        while(meshitr != meshend && err == 0)
        {
            err = taa_scenefile_deserialize_mesh(fs, meshitr);
            ++meshitr;
        }
    }
    if(err == 0)
    {
        taa_scenenode* nodeitr;
        taa_scenenode* nodeend;
        taa_scene_resize_nodes(scene, numnodes);
        nodeitr = scene->nodes;
        nodeend = nodeitr + numnodes;
        while(nodeitr != nodeend && err == 0)
        {
            err = taa_scenefile_deserialize_node(fs, nodeitr);
            ++nodeitr;
        }
    }
    if(err == 0)
    {
        taa_sceneskel* skelitr;
        taa_sceneskel* skelend;
        taa_scene_resize_skeletons(scene, numskeletons);
        skelitr = scene->skeletons;
        skelend = skelitr + numskeletons;
        while(skelitr != skelend && err == 0)
        {
            err = taa_scenefile_deserialize_skeleton(fs, skelitr);
            ++skelitr;
        }
    }
    if(err == 0)
    {
        taa_scenetexture* texitr;
        taa_scenetexture* texend;
        taa_scene_resize_textures(scene, numtextures);
        texitr = scene->textures;
        texend = texitr + numtextures;
        while(texitr != texend && err == 0)
        {
            err = taa_scenefile_deserialize_texture(fs, texitr);
            ++texitr;
        }
    }
    return err;
}

//****************************************************************************
void taa_scenefile_serialize(
    const taa_scene* scene,
    taa_filestream* fs)
{
    const taa_sceneanim* animitr = scene->animations;
    const taa_sceneanim* animend = animitr + scene->numanimations;
    const taa_scenematerial* matitr = scene->materials;
    const taa_scenematerial* matend = matitr + scene->nummaterials;
    const taa_scenemesh* meshitr = scene->meshes;
    const taa_scenemesh* meshend = meshitr + scene->nummeshes;
    const taa_scenenode* nodeitr = scene->nodes;
    const taa_scenenode* nodeend = nodeitr + scene->numnodes;
    const taa_sceneskel* skelitr = scene->skeletons;
    const taa_sceneskel* skelend = skelitr + scene->numskeletons;
    const taa_scenetexture* texitr = scene->textures;
    const taa_scenetexture* texend = texitr + scene->numtextures;
    // write magic number
    taa_filestream_write_i64(fs, taa_SCENEFILE_MAGIC);
    // write version number
    taa_filestream_write_i32(fs, 0);
    // write up axis
    taa_filestream_write_i32(fs, scene->upaxis);
    // write counts of items
    taa_filestream_write_i32(fs, scene->numanimations);
    taa_filestream_write_i32(fs, scene->nummaterials);
    taa_filestream_write_i32(fs, scene->nummeshes);
    taa_filestream_write_i32(fs, scene->numnodes);
    taa_filestream_write_i32(fs, scene->numskeletons);
    taa_filestream_write_i32(fs, scene->numtextures);
    // write animations
    while(animitr != animend)
    {
        taa_scenefile_serialize_animation(animitr, fs);
        ++animitr;
    }
    // write materials
    while(matitr != matend)
    {
        taa_scenefile_serialize_material(matitr, fs);
        ++matitr;
    }
    // write meshes
    while(meshitr != meshend)
    {
        taa_scenefile_serialize_mesh(meshitr, fs);
        ++meshitr;
    }
    // write nodes
    while(nodeitr != nodeend)
    {
        taa_scenefile_serialize_node(nodeitr, fs);
        ++nodeitr;
    }
    // write skeletons
    while(skelitr != skelend)
    {
        taa_scenefile_serialize_skeleton(skelitr, fs);
        ++skelitr;
    }
    // write textures
    while(texitr != texend)
    {
        taa_scenefile_serialize_texture(texitr, fs);
        ++texitr;
    }
}
