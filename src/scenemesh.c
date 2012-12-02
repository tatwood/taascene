/**
 * @brief     mesh processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/scenemesh.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// converts source vertex value type to new type in a loop
#define taa_FORMAT_LOOP(srct, dstt, src, itr, end, srcinc, dstinc) \
    while(itr < end) \
    { \
        *((dstt*) itr) = (dstt) (*((srct*) src)); \
        itr += dstinc; \
        src += srcinc; \
    }

// converts new value type enum to C type during vertex format conversion
#define taa_FORMAT_NEWT(srct,dstenum,src,itr,end,srcinc,dstinc) \
    switch(dstenum) \
    { \
    case taa_SCENEMESH_VALUE_FLOAT32: \
        taa_FORMAT_LOOP(srct,float,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_FLOAT64: \
        taa_FORMAT_LOOP(srct,double,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT8: \
        taa_FORMAT_LOOP(srct,int8_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT8: \
        taa_FORMAT_LOOP(srct,uint8_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT16: \
        taa_FORMAT_LOOP(srct,int16_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT16: \
        taa_FORMAT_LOOP(srct,uint16_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT32: \
        taa_FORMAT_LOOP(srct,int32_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT32: \
        taa_FORMAT_LOOP(srct,uint32_t,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_MERGED: \
        assert(0); \
        break; \
    }

// converts source value type enum to C type during vertex format conversion
#define taa_FORMAT_SRCT(srcEnum,dstenum,src,itr,end,srcinc,dstinc) \
    switch(srcEnum) \
    { \
    case taa_SCENEMESH_VALUE_FLOAT32: \
        taa_FORMAT_NEWT(float,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_FLOAT64: \
        taa_FORMAT_NEWT(double,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT8: \
        taa_FORMAT_NEWT(int8_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT8: \
        taa_FORMAT_NEWT(uint8_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT16: \
        taa_FORMAT_NEWT(int16_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT16: \
        taa_FORMAT_NEWT(uint16_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_INT32: \
        taa_FORMAT_NEWT(int32_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_UINT32: \
        taa_FORMAT_NEWT(uint32_t,dstenum,src,itr,end,srcinc,dstinc) \
        break; \
    case taa_SCENEMESH_VALUE_MERGED: \
        assert(0); \
        break; \
    }

#define taa_FORMAT(srcEnum,dstenum,src,itr,end,srcinc,dstinc) \
    taa_FORMAT_SRCT(srcEnum, dstenum, src, itr, end, srcinc, dstinc)

//****************************************************************************
static void* taa_scenemesh_aligned_realloc(
    void* ptr,
    size_t align,
    size_t oldsize,
    size_t newsize)
{
    void* newptr = taa_memalign(align, newsize);
    if(ptr != NULL)
    {
        memcpy(newptr, ptr, oldsize);
        taa_memalign_free(ptr);
    }
    return newptr;
}

//****************************************************************************
static int taa_scenemesh_calc_stride(
    int valuetype,
    int numcomponents)
{
    int compstride = 0;
    switch(valuetype)
    {
    case taa_SCENEMESH_VALUE_FLOAT32: compstride = sizeof(float);    break;
    case taa_SCENEMESH_VALUE_FLOAT64: compstride = sizeof(double);   break;
    case taa_SCENEMESH_VALUE_INT8:    compstride = sizeof(int8_t);   break;
    case taa_SCENEMESH_VALUE_INT16:   compstride = sizeof(int16_t);  break;
    case taa_SCENEMESH_VALUE_INT32:   compstride = sizeof(int32_t);  break;
    case taa_SCENEMESH_VALUE_UINT8:   compstride = sizeof(uint8_t);  break;
    case taa_SCENEMESH_VALUE_UINT16:  compstride = sizeof(uint16_t); break;
    case taa_SCENEMESH_VALUE_UINT32:  compstride = sizeof(uint32_t); break;
    case taa_SCENEMESH_VALUE_MERGED:  assert(0); break;
    }
    return compstride * numcomponents;
}

//****************************************************************************
taa_scenemesh_face* taa_scenemesh_add_face(
    taa_scenemesh* mesh,
    const uint32_t* indices,
    uint32_t numindices,
    uint32_t numvertices)
{
    int32_t faceid = mesh->numfaces;
    int32_t firstindex = mesh->numindices;
    taa_scenemesh_face* newface;
    uint32_t* newindices;
    // resize face buffer
    taa_scenemesh_resize_faces(mesh, faceid + 1);
    newface = mesh->faces + faceid;
    newface->firstindex = firstindex;
    newface->numindices = numindices;
    newface->numvertices = numvertices;
    // resize index buffer and copy indices
    taa_scenemesh_resize_indices(mesh, firstindex + numindices);
    newindices = mesh->indices + firstindex;
    memcpy(newindices, indices, sizeof(*newindices)*numindices);
    return newface;
}

//****************************************************************************
int taa_scenemesh_add_skinjoint(
    taa_scenemesh* mesh,
    uint32_t animjoint,
    const taa_mat44* invbindmatrix)
{
    uint32_t jointid = mesh->numjoints;
    taa_scenemesh_skinjoint* joint;
    taa_scenemesh_resize_skinjoints(mesh, jointid + 1);
    joint = mesh->joints + jointid;
    joint->animjoint = animjoint;
    joint->invbindmatrix = *invbindmatrix;
    return jointid;
}

//****************************************************************************
int taa_scenemesh_add_stream(
    taa_scenemesh* mesh,
    const char* name,
    taa_scenemesh_usage usage,
    int set,
    taa_scenemesh_valuetype valuetype,
    int numcomponents,
    int stride,
    int indexmapping,
    uint32_t numvertices,
    const void* srcbuffer)
{
    int32_t vsid = mesh->numstreams;
    taa_scenemesh_stream* vs;
    uint32_t bufsize;

    // resize vertex stream array and grab pointer to new stream
    taa_scenemesh_resize_streams(mesh, vsid + 1);
    vs = mesh->vertexstreams + vsid;

    strncpy(vs->name, name, sizeof(vs->name));
    vs->name[sizeof(vs->name) - 1] = '\0';
    vs->usage = usage;
    vs->set = set;
    vs->valuetype = valuetype;
    vs->numcomponents = numcomponents;
    vs->indexmapping = indexmapping;
    taa_scenemesh_resize_vertices(vs, stride, numvertices);
    bufsize = numvertices * stride;
    if(srcbuffer != NULL)
    {
        memcpy(vs->buffer, srcbuffer, bufsize);
    }
    else
    {
        memset(vs->buffer, 0, bufsize);
    }

    if(indexmapping >= mesh->indexsize)
    {
        mesh->indexsize = indexmapping + 1;
    }

    return vsid;
}

//****************************************************************************
int taa_scenemesh_add_vertex(
    taa_scenemesh_stream* vs,
    const void* vertdata)
{
    int index = vs->numvertices;
    int stride = vs->stride;
    taa_scenemesh_resize_vertices(vs, stride, index + 1);
    memcpy(((uint8_t*) vs->buffer) + stride*index, vertdata, stride);
    return index;
}

//****************************************************************************
void taa_scenemesh_begin_binding(
    taa_scenemesh* mesh,
    const char* name,
    int matid)
{
    int i = mesh->numbindings;
    taa_scenemesh_binding* binding;
    taa_scenemesh_resize_bindings(mesh, i + 1);
    binding = mesh->bindings + i;
    strncpy(binding->name, name, sizeof(binding->name)-1);
    binding->materialid = matid;
    binding->firstface = mesh->numfaces;
    binding->numfaces = 0;
}

//****************************************************************************
void taa_scenemesh_create(
    const char* name,
    taa_scenemesh* mesh_out)
{
    memset(mesh_out, 0, sizeof(*mesh_out));
    if(name != NULL)
    {
        strncpy(mesh_out->name, name, sizeof(mesh_out->name));
        mesh_out->name[sizeof(mesh_out->name)-1] = '\0';
    }
    mesh_out->skeleton = -1;
}

//****************************************************************************
void taa_scenemesh_destroy(
    taa_scenemesh* mesh)
{
    taa_scenemesh_stream* vsitr = mesh->vertexstreams;
    taa_scenemesh_stream* vsend = vsitr + mesh->numstreams;
    // free aligned allocations
    while(vsitr != vsend)
    {
        taa_memalign_free(vsitr->buffer);
        ++vsitr;
    }
    taa_memalign_free(mesh->joints);
    // free normal allocations
    free(mesh->indices);
    free(mesh->faces);
    free(mesh->bindings);
    free(mesh->vertexstreams);
}

//****************************************************************************
void taa_scenemesh_end_binding(
    taa_scenemesh* mesh)
{
    taa_scenemesh_binding* binding;
    binding = mesh->bindings + (mesh->numbindings - 1);
    binding->numfaces = mesh->numfaces - binding->firstface;
}

//****************************************************************************
int taa_scenemesh_find_binding(
    taa_scenemesh* mesh,
    const char* name)
{
    int result = -1;
    taa_scenemesh_binding* bitr = mesh->bindings;
    taa_scenemesh_binding* bend = bitr + mesh->numbindings;
    while(bitr != bend)
    {
        if(!strcmp(bitr->name, name))
        {
            result = (int)(ptrdiff_t) (bitr - mesh->bindings);
            break;
        }
        ++bitr;
    }
    return result;
}

//****************************************************************************
int taa_scenemesh_find_stream(
    taa_scenemesh* mesh,
    taa_scenemesh_usage usage,
    uint32_t set)
{
    int result = -1;
    taa_scenemesh_stream* vsitr = mesh->vertexstreams;
    taa_scenemesh_stream* vsend = vsitr + mesh->numstreams;
    while(vsitr != vsend)
    {
        if(vsitr->usage == usage && vsitr->set == set)
        {
            result = (int)(ptrdiff_t) (vsitr - mesh->vertexstreams);
            break;
        }
        ++vsitr;
    }
    return result;
}

//****************************************************************************
int taa_scenemesh_find_vertex(
    const taa_scenemesh_stream* vs,
    const void* vertdata)
{
    int index = -1;
    int stride = vs->stride;
    const uint8_t* vitr = vs->buffer;
    const uint8_t* vend = vs->buffer + (stride * vs->numvertices);
    while(vitr != vend)
    {
        if(!memcmp(vertdata, vitr, stride))
        {
            index = (int) ((ptrdiff_t) (vitr - vs->buffer))/stride;
            break;
        }
        vitr += stride;
    }
    return index;
}    

//****************************************************************************
void taa_scenemesh_format(
    taa_scenemesh* mesh,
    const taa_scenemesh_vertformat* vf,
    int numvf)
{
    const taa_scenemesh_vertformat* vfitr;
    const taa_scenemesh_vertformat* vfend;
    unsigned int i;
    // remove unused vertex streams
    i = 0;
    while(i < mesh->numstreams)
    {
        taa_scenemesh_stream* vs = mesh->vertexstreams + i;
        vfitr = vf;
        vfend = vfitr + numvf;
        while(vfitr != vfend)
        {
            if(vfitr->usage == vs->usage && vfitr->set == vs->set)
            {
                break;
            }
            ++vfitr;
        }
        if(vfitr == vfend)
        {
            taa_scenemesh_remove_stream(mesh, vs);
        }
        else
        {
            ++i;
        }
    }
    // if any of the source streams are missing, fill them in
    vfitr = vf;
    vfend = vfitr + numvf;
    while(vfitr != vfend)
    {
        if(taa_scenemesh_find_stream(mesh,vfitr->usage,vfitr->set) < 0)
        {
            int indexmapping = 0;
            int numverts = 0;
            int vs;
            int stride;
            if(mesh->numstreams > 0)
            {
                indexmapping = mesh->vertexstreams[0].indexmapping;
                numverts = mesh->vertexstreams[0].numvertices;                
            }
            stride = taa_scenemesh_calc_stride(
                vfitr->valuetype,
                vfitr->numcomponents);
            vs = taa_scenemesh_add_stream(
                mesh,
                vfitr->name,
                vfitr->usage,
                vfitr->set,
                vfitr->valuetype,
                vfitr->numcomponents,
                stride,
                indexmapping,
                numverts,
                NULL);
            memset(mesh->vertexstreams[vs].buffer, 0, numverts*stride);
        }
        ++vfitr;
    }
    assert(mesh->numstreams == numvf); // stream mismatch (should not happen)
    if(mesh->numstreams == numvf)
    {
        taa_scenemesh_stream** mrgvs;
        uint32_t* mrgoff;
        void** streambufmap;
        uint32_t nummrg;
        vfitr = vf;
        vfend = vfitr + numvf;
        // reformat vertex streams
        while(vfitr != vfend)
        {
            int32_t vs;
            taa_scenemesh_stream* pvs;
            vs = taa_scenemesh_find_stream(mesh, vfitr->usage, vfitr->set);
            pvs = mesh->vertexstreams + vs;
            assert(vs >= 0); // already should have filtered this case
            taa_scenemesh_format_stream(
                pvs,
                vfitr->valuetype,
                vfitr->numcomponents);
            ++vfitr;
        }
        // convert vertices to a common set of indices
        taa_scenemesh_merge_indices(mesh);
        // merge streams together
        mrgvs = (taa_scenemesh_stream**) malloc(numvf * sizeof(*mrgvs));
        mrgoff = (uint32_t*) malloc(numvf * sizeof(*mrgoff));
        streambufmap = (void**) malloc(numvf * sizeof(*streambufmap));
        memset(mrgvs, 0, numvf * sizeof(*mrgvs));
        memset(mrgoff, 0, numvf * sizeof(*mrgoff));
        memset(streambufmap, 0, numvf * sizeof(*streambufmap));
        i = 0;
        do
        {
            const char* mrgname = "";
            taa_scenemesh_stream* pvs;
            nummrg = 0;
            vfitr = vf;
            while(vfitr != vfend)
            {
                if(vfitr->stream == i)
                {
                    int32_t vs;
                    vs = taa_scenemesh_find_stream(
                        mesh,
                        vfitr->usage,
                        vfitr->set);
                    assert(vs >= 0); // already should have filtered this case
                    pvs = mesh->vertexstreams + vs;
                    mrgname = (vfitr->name != NULL) ? vfitr->name : "";
                    mrgvs[nummrg] = pvs;
                    mrgoff[nummrg] = vfitr->offset;
                    ++nummrg;
                }
                ++vfitr;
            }
            if(nummrg == 0)
            {
                // finished
                break;
            }
            else if(nummrg == 1)
            {
                // nothing to merge
                pvs = mrgvs[0];
                strncpy(pvs->name, mrgname, sizeof(pvs->name));
                pvs->name[sizeof(pvs->name)-1] = '\0';
            }
            else
            {
                uint32_t vs;
                vs = taa_scenemesh_merge_streams(
                    mesh,
                    mrgname,
                    mrgvs,
                    mrgoff,
                    nummrg);
                pvs = mesh->vertexstreams + vs;
            }
            streambufmap[i] = pvs->buffer;
            ++i;
        }
        while(1);
        assert(i == mesh->numstreams);

        // reorder streams
        for(i = 0; i < mesh->numstreams; ++i)
        {
            void* bufmapping = streambufmap[i];
            taa_scenemesh_stream vsbak = mesh->vertexstreams[i];
            taa_scenemesh_stream* vsitr = mesh->vertexstreams;
            while(vsitr->buffer != bufmapping)
            {
                assert(vsitr < mesh->vertexstreams+mesh->numstreams);
                ++vsitr;
            }
            // swap
            mesh->vertexstreams[i] = *vsitr;
            *vsitr = vsbak;
        }
        free(streambufmap);
        free(mrgoff);
        free(mrgvs);
    }
}

//****************************************************************************
void taa_scenemesh_format_stream(
    taa_scenemesh_stream* vs,
    taa_scenemesh_valuetype valuetype,
    unsigned int numcomponents)
{
    unsigned int i;
    int srccomponentsize;
    int newcomponentsize;
    int newstride;
    int srcstride;
    uint64_t srczero;
    uint8_t* bufitr;
    uint8_t* bufend;
    const uint8_t* bufsrc;

    // if the source vertex stream is a merged aggregate,
    // it cannot be reformatted
    assert(vs->valuetype != taa_SCENEMESH_VALUE_MERGED);

    if(valuetype != vs->valuetype || numcomponents != vs->numcomponents)
    {
        taa_scenemesh_stream tmpvs;
        newstride = taa_scenemesh_calc_stride(valuetype, numcomponents);
        tmpvs = *vs;
        tmpvs.valuetype = valuetype;
        tmpvs.numcomponents = numcomponents;
        tmpvs.stride = 0;
        tmpvs.numvertices = 0;
        tmpvs.buffer = NULL;
        taa_scenemesh_resize_vertices(&tmpvs, newstride, vs->numvertices);
        bufend = tmpvs.buffer + (newstride * vs->numvertices);

        srczero = 0;
        srcstride = vs->stride;
        newcomponentsize = newstride / numcomponents;
        srccomponentsize = vs->stride / vs->numcomponents;
        for(i = 0; i < numcomponents; ++i)
        {
            bufitr = tmpvs.buffer + i*newcomponentsize;
            bufsrc = vs->buffer + i*srccomponentsize;
            if(i >= vs->numcomponents)
            {
                // if the new vertex format has more components than original,
                // fill with zeroes
                bufsrc = (uint8_t*) &srczero;
                srcstride = 0;
            }
            taa_FORMAT(
                vs->valuetype,
                valuetype,
                bufsrc,
                bufitr,
                bufend,
                srcstride,
                newstride)
        }
        // apply the new format to the stream
        taa_memalign_free(vs->buffer);
        *vs = tmpvs;
    }
}

//****************************************************************************
void taa_scenemesh_merge_indices(
    taa_scenemesh* mesh)
{
    uint32_t size;
    uint32_t indexsize;
    uint32_t numdstindices;
    uint32_t numbakindices;
    uint32_t* indices;
    uint32_t* bakindices;
    taa_scenemesh_face* faceitr;
    taa_scenemesh_face* faceend;
    taa_scenemesh_stream* dststreams;
    taa_scenemesh_stream* vsitr;
    taa_scenemesh_stream* vsend;
    const taa_scenemesh_stream* vssrc;

    // reserve a temporary buffer for recording merged indices
    indexsize = mesh->indexsize;
    indices = mesh->indices;
    numdstindices = 0;
    size = sizeof(*bakindices) * mesh->numindices;
    bakindices = (uint32_t*) malloc(size);
    numbakindices = 0;

    // create a new set of vertex streams from the original set
    size = sizeof(*dststreams) * mesh->numstreams;
    dststreams = (taa_scenemesh_stream*) malloc(size);
    vsitr = dststreams;
    vsend = vsitr + mesh->numstreams;
    vssrc = mesh->vertexstreams;
    while(vsitr != vsend)
    {
        memcpy(vsitr->name, vssrc->name, sizeof(vsitr->name));
        vsitr->usage = vssrc->usage;
        vsitr->set = vssrc->set;
        vsitr->valuetype = vssrc->valuetype;
        vsitr->numcomponents = vssrc->numcomponents;
        vsitr->stride = vssrc->stride;
        vsitr->indexmapping = 0; // all streams will use index 0 now
        vsitr->numvertices = 0;
        vsitr->buffer = NULL;
        ++vssrc;
        ++vsitr;
    }

    // loop through all the faces and create dst faces to replace them
    faceitr = mesh->faces;
    faceend = faceitr + mesh->numfaces;
    while(faceitr != faceend)
    {
        const uint32_t* indexsrc;
        const uint32_t* indexsrcend;

        // loop through each vertex in the face and get the merged index
        indexsrc = indices + faceitr->firstindex;
        indexsrcend = indexsrc + faceitr->numindices;
        faceitr->firstindex = numdstindices;
        while(indexsrc != indexsrcend)
        {
            uint32_t* bakitr;
            uint32_t* bakend;
            int32_t dstindex = -1;

            // try to find a matching vertex combination from a previous face
            bakitr = bakindices;
            bakend = bakitr + numbakindices*indexsize;
            while(bakitr != bakend)
            {
                if(!memcmp(bakitr,indexsrc,sizeof(*bakitr)*indexsize))
                {
                    dstindex=((uint32_t)(bakitr-bakindices))/indexsize;
                    break;
                }
                bakitr += indexsize;
            }

            if(dstindex == -1)
            {
                // no matching vertex combination found, create a dst vertex
                dstindex = numbakindices++;
                // map the merged index to the old combination
                size = sizeof(*bakindices) * numbakindices * indexsize;
                bakindices = (uint32_t*) realloc(bakindices, size);
                size = sizeof(*bakindices) * indexsize;
                bakitr = bakindices + dstindex*indexsize;
                bakend = bakitr + indexsize;
                memcpy(bakitr, indexsrc, size);
                // copy the vertex data to the vertex streams
                vsitr = dststreams;
                vsend = vsitr + mesh->numstreams;
                vssrc = mesh->vertexstreams;
                while(vsitr != vsend)
                {
                    const uint8_t* vertsrc;
                    uint8_t* vertBuf;
                    uint32_t* bakidx = bakitr + vssrc->indexmapping;
                    assert(bakidx < bakend); // index buffer overflow
                    // dst vert goes at end of vertex buffer
                    taa_scenemesh_resize_vertices(
                        vsitr,
                        vsitr->stride,
                        vsitr->numvertices+1);
                    size = vsitr->stride * vsitr->numvertices;
                    vertBuf = vsitr->buffer + (size - vsitr->stride);
                    // old vert comes from data at index in old buffer
                    vertsrc = vssrc->buffer + (vssrc->stride * (*bakidx));
                    assert(*bakidx < vssrc->numvertices); // bad index
                    memcpy(vertBuf, vertsrc, vsitr->stride);
                    ++vssrc;
                    ++vsitr;
                }
            }
            indices[numdstindices++] = dstindex;
            indexsrc += indexsize;
        }
        faceitr->numindices = numdstindices - faceitr->firstindex;
        faceitr->numvertices = faceitr->numindices;
        ++faceitr;
    }

    // clean up the temporary buffer for holding the merged indices
    free(bakindices);
    mesh->numindices = numdstindices;

    // clean up the old vertex streams and replace with the new streams
    vsitr = mesh->vertexstreams;
    vsend = vsitr + mesh->numstreams;
    while(vsitr != vsend)
    {
        taa_memalign_free(vsitr->buffer);
        ++vsitr;
    }
    free(mesh->vertexstreams);
    mesh->vertexstreams = dststreams;

    mesh->indexsize = 1; // only one index per vertex now
}

//****************************************************************************
int taa_scenemesh_merge_streams(
    taa_scenemesh* mesh,
    const char* name,
    taa_scenemesh_stream** streams,
    const uint32_t* streamoffsets,
    uint32_t numstreams)
{
    uint32_t newnumstreams;
    taa_scenemesh_stream* vsnew;
    taa_scenemesh_stream** vssrcitr;
    taa_scenemesh_stream** vssrcend;
    taa_scenemesh_stream* vssrc;
    taa_scenemesh_stream* vsitr;
    taa_scenemesh_stream* vsend;
    const uint32_t* offitr;

    assert(numstreams <= mesh->numstreams); // too many streams

    // create the merged stream
    vsnew = (taa_scenemesh_stream*) malloc(sizeof(*vsnew));
    strncpy(vsnew->name, name, sizeof(vsnew->name) - 1);
    vsnew->name[sizeof(vsnew->name) - 1] = '\0';
    vsnew->usage = taa_SCENEMESH_USAGE_MERGED;
    vsnew->set = 0;
    vsnew->valuetype = taa_SCENEMESH_VALUE_MERGED;
    vsnew->numcomponents = 1;
    vsnew->stride = 0;
    vsnew->indexmapping = 0;
    vsnew->numvertices = 0;
    vsnew->buffer = NULL;

    // determine stride, index offset, and vertex count of the merged stream
    vssrcitr = streams;
    vssrcend = vssrcitr + numstreams;
    offitr = streamoffsets;
    while(vssrcitr != vssrcend)
    {
        vssrc = *vssrcitr;
        // all the merged streams should share the same index
        assert(vssrc->indexmapping==vsnew->indexmapping || vssrcitr==streams);
        // all the merged streams should have the same vertex count
        assert(vssrc->numvertices==vsnew->numvertices || vssrcitr==streams);

        vsnew->indexmapping = vssrc->indexmapping;
        if(((*offitr) + vssrc->stride) > vsnew->stride)
        {
            taa_scenemesh_resize_vertices(
                vsnew,
                (*offitr) + vssrc->stride,
                vssrc->numvertices);        
        }
        ++offitr;
        ++vssrcitr;
    }

    // copy the vertex data from each source stream into the merged stream
    vssrcitr = streams;
    vssrcend = vssrcitr + numstreams;
    offitr = streamoffsets;
    while(vssrcitr != vssrcend)
    {
        uint8_t* bufitr;
        uint8_t* bufend;
        const uint8_t* bufsrc;

        vssrc = *vssrcitr;
        bufsrc = vssrc->buffer;
        bufitr = vsnew->buffer + (*offitr);
        bufend = bufitr + (vsnew->stride * vsnew->numvertices);
        while(bufitr != bufend)
        {
            memcpy(bufitr, bufsrc, vssrc->stride);
            bufsrc += vssrc->stride;
            bufitr += vsnew->stride;
        }
        ++offitr;
        ++vssrcitr;
    }

    // add original vertex streams back into new stream set
    newnumstreams = 1; // first stream is the newly merged stream
    vsitr = mesh->vertexstreams;
    vsend = vsitr + mesh->numstreams;
    while(vsitr != vsend)
    {
        vssrcitr = streams;
        vssrcend = vssrcitr + numstreams;
        while(vssrcitr != vssrcend)
        {
            if(*vssrcitr == vsitr)
            {
                // this stream is one of the merged streams
                break;
            }
            ++vssrcitr;
        }
        if(vssrcitr == vssrcend)
        {
            // the stream was not merged, add it to the new stream set
            ++newnumstreams;
            vsnew = (taa_scenemesh_stream*) realloc(
                vsnew,
                sizeof(*vsnew) * newnumstreams);
            vsnew[newnumstreams-1] = *vsitr;
        }
        ++vsitr;
    }
    assert(newnumstreams==(mesh->numstreams-numstreams)+1);

    // clean up the source streams
    vssrcitr = streams;
    vssrcend = vssrcitr + numstreams;
    while(vssrcitr != vssrcend)
    {
        vssrc = *vssrcitr;
        taa_memalign_free(vssrc->buffer);
        ++vssrcitr;
    }
    free(mesh->vertexstreams);

    mesh->vertexstreams = vsnew;
    mesh->numstreams = newnumstreams;
    return 0; // new stream is first
}

//****************************************************************************
void taa_scenemesh_remove_stream(
    taa_scenemesh* mesh,
    taa_scenemesh_stream* vs)
{
    uint32_t i;
    uint32_t iend;
    uint32_t removeoffset;
    taa_scenemesh_stream* vsitr;
    taa_scenemesh_stream* vsend;

    removeoffset = 1; // true
    vsitr = mesh->vertexstreams;
    vsend = vsitr + mesh->numstreams;
    while(vsitr != vsend)
    {
        if((vsitr != vs) && vsitr->indexmapping == vs->indexmapping)
        {
            // the indices are shared with another vertex stream
            removeoffset = 0; // false
            break;
        }
        ++vsitr;
    }

    if(removeoffset)
    {
        taa_scenemesh_face* faceitr;
        taa_scenemesh_face* faceend;
        uint32_t* indexitr;

        // remove indices from index buffer and all the faces
        removeoffset = vs->indexmapping;
        iend = mesh->indexsize--;
        faceitr = mesh->faces;
        faceend = faceitr + mesh->numfaces;
        indexitr = mesh->indices;
        while(faceitr != faceend)
        {
            const uint32_t* indexsrc;
            const uint32_t* indexsrcend;
            uint32_t* indexoff;

            indexoff = indexitr;
            indexsrc = mesh->indices + faceitr->firstindex;
            indexsrcend = indexsrc + faceitr->numindices;
            while(indexsrc != indexsrcend)
            {
                for(i = 0; i != iend; ++i)
                {
                    if(i != removeoffset)
                    {
                        *indexitr = *indexsrc;
                        ++indexitr;
                    }
                    ++indexsrc;
                }
            }
            faceitr->firstindex = (uint32_t) (indexoff - mesh->indices);
            faceitr->numindices = (uint32_t) (indexitr - indexoff);
            assert(faceitr->numindices ==
                   faceitr->numvertices * mesh->indexsize);
            ++faceitr;
        }

        // fix offsets of any affected vertex streams
        vsitr = mesh->vertexstreams;
        vsend = vsitr + mesh->numstreams;
        while(vsitr != vsend)
        {
            if(removeoffset != 0 && vsitr->indexmapping > removeoffset)
            {
                --vsitr->indexmapping;
            }
            ++vsitr;
        }
        mesh->numindices = (uint32_t) (indexitr - mesh->indices);
    }
    // clean up the vertex stream and remove from array
    taa_memalign_free(vs->buffer);
    --mesh->numstreams;
    vsitr = vs;
    vsend = mesh->vertexstreams + mesh->numstreams;
    while(vsitr != vsend)
    {
        *vsitr = *(vsitr + 1);
        ++vsitr;
    }
}

//****************************************************************************
void taa_scenemesh_resize_bindings(
    taa_scenemesh* mesh,
    uint32_t numbindings)
{
    uint32_t oldnum = mesh->numbindings;
    taa_scenemesh_binding* binding = mesh->bindings;
    if(numbindings > oldnum)
    {
        uint32_t cap = (oldnum     +7) & ~7;
        uint32_t ncap= (numbindings+7) & ~7;
        if(cap != ncap)
        {
            binding = (taa_scenemesh_binding*) realloc(
                binding,
                ncap * sizeof(*binding));
            mesh->bindings = binding;
        }
        binding += oldnum;
        memset(binding, 0, (numbindings-oldnum) * sizeof(*binding));
    }
    mesh->numbindings = numbindings;
}

//****************************************************************************
void taa_scenemesh_resize_faces(
    taa_scenemesh* mesh,
    uint32_t numfaces)
{
    uint32_t oldnum = mesh->numfaces;
    taa_scenemesh_face* face = mesh->faces;
    if(numfaces > oldnum)
    {
        uint32_t cap = (oldnum   + 1023) & ~1023;
        uint32_t ncap= (numfaces + 1023) & ~1023;
        if(cap != ncap)
        {
            // realloc the face buffer to nearest 1k capacity
            face = (taa_scenemesh_face*) realloc(
                face,
                ncap * sizeof(*face));
            mesh->faces = face;
        }
        face += oldnum;
        memset(face, 0, (numfaces-oldnum) * sizeof(*face));
    }
    mesh->numfaces = numfaces;
}

//****************************************************************************
void taa_scenemesh_resize_indices(
    taa_scenemesh* mesh,
    uint32_t numindices)
{
    uint32_t oldnum = mesh->numindices;
    uint32_t* indices = mesh->indices;
    if(numindices > oldnum)
    {
        uint32_t cap = (oldnum     + 1023) & ~1023;
        uint32_t ncap= (numindices + 1023) & ~1023;
        if(cap != ncap)
        {
            // realloc the face buffer to nearest 1k capacity
            indices = (uint32_t*) realloc(
                indices,
                ncap * sizeof(*indices));
            mesh->indices = indices;
        }
        indices += oldnum;
        memset(indices, 0, (numindices-oldnum) * sizeof(*indices));
    }
    mesh->numindices = numindices;
}

//****************************************************************************
void taa_scenemesh_resize_skinjoints(
    taa_scenemesh* mesh,
    uint32_t numjoints)
{
    uint32_t oldnum = mesh->numjoints;
    taa_scenemesh_skinjoint* joint = mesh->joints;
    uint32_t cap = (oldnum   +31) & ~31;
    uint32_t ncap= (numjoints+31) & ~31;
    if(cap != ncap)
    {
        joint = (taa_scenemesh_skinjoint*) taa_scenemesh_aligned_realloc(
            joint,
            16,
            cap * sizeof(*joint),
            ncap * sizeof(*joint));
        mesh->joints = joint;
    }
    if(numjoints > oldnum)
    {
        joint += oldnum;
        memset(joint, 0, (numjoints-oldnum) * sizeof(*joint));
    }
    mesh->numjoints = numjoints;
}

//****************************************************************************
void taa_scenemesh_resize_streams(
    taa_scenemesh* mesh,
    uint32_t numstreams)
{
    uint32_t oldnum = mesh->numstreams;
    taa_scenemesh_stream* vs = mesh->vertexstreams;
    if(numstreams > oldnum)
    {
        uint32_t cap = (oldnum    +7) & ~7;
        uint32_t ncap= (numstreams+7) & ~7;
        if(cap != ncap)
        {
            vs = (taa_scenemesh_stream*) realloc(vs, ncap * sizeof(*vs));
            mesh->vertexstreams = vs;
        }
        vs += oldnum;
        memset(vs, 0, (numstreams-oldnum) * sizeof(*vs));
    }
    mesh->numstreams = numstreams;
}

//****************************************************************************
void taa_scenemesh_resize_vertices(
    taa_scenemesh_stream* vs,
    uint32_t stride,
    uint32_t numverts)
{
    uint32_t oldsz = vs->stride * vs->numvertices;
    uint32_t newsz = stride * numverts;
    // round to nearest mb
    uint32_t ocap = (oldsz + (1024*1024-1)) & ~(1024*1024-1);
    uint32_t ncap = (newsz + (1024*1024-1)) & ~(1024*1024-1);
    if(ocap != ncap)
    {
        uint8_t* buf = vs->buffer;    
        buf = (uint8_t*) taa_scenemesh_aligned_realloc(buf, 16, ocap, ncap);
        vs->buffer = buf;
    }
    vs->stride = stride;
    vs->numvertices = numverts;
}

//****************************************************************************
void taa_scenemesh_rotate_upaxis(
    taa_scenemesh* mesh,
    int32_t dir)
{
    taa_scenemesh_skinjoint* jointitr = mesh->joints;
    taa_scenemesh_skinjoint* jointend = jointitr + mesh->numjoints;
    taa_scenemesh_stream* vsitr = mesh->vertexstreams;
    taa_scenemesh_stream* vsend = vsitr + mesh->numstreams;
    taa_mat44 pitch;
    taa_mat44 invpitch;
    while(vsitr != vsend)
    {
        int32_t numcomps = vsitr->numcomponents;
        int32_t numverts = vsitr->numvertices;
        int32_t fixvs = 0;
        switch(vsitr->usage)
        {
        case taa_SCENEMESH_USAGE_BINORMAL:    fixvs = 1; break;
        case taa_SCENEMESH_USAGE_BLENDINDEX:  fixvs = 0; break;
        case taa_SCENEMESH_USAGE_BLENDWEIGHT: fixvs = 0; break;
        case taa_SCENEMESH_USAGE_COLOR:       fixvs = 0; break;
        case taa_SCENEMESH_USAGE_FOG:         fixvs = 0; break;
        case taa_SCENEMESH_USAGE_NORMAL:      fixvs = 1; break;
        case taa_SCENEMESH_USAGE_POINTSIZE:   fixvs = 0; break;
        case taa_SCENEMESH_USAGE_POSITION:    fixvs = 1; break;
        case taa_SCENEMESH_USAGE_TANGENT:     fixvs = 1; break;
        case taa_SCENEMESH_USAGE_TEXCOORD:    fixvs = 0; break;
        case taa_SCENEMESH_USAGE_MERGED:      fixvs = 0; break;
        }
        if(fixvs && (numcomps >= 3))
        {
            // rotate Y and Z components
            switch(vsitr->valuetype)
            {
            case taa_SCENEMESH_VALUE_FLOAT32:
                {
                    float* vitr = (float*) vsitr->buffer;
                    float* vend = vitr + (numverts * numcomps);
                    float fdir = (float) dir;
                    while(vitr != vend)
                    {
                        float tmp = vitr[1];
                        vitr[1] = fdir * vitr[2];
                        vitr[2] = fdir * -tmp;
                        vitr += numcomps;
                    }
                }
                break;
            case taa_SCENEMESH_VALUE_FLOAT64:
                {
                    double* vitr = (double*) vsitr->buffer;
                    double* vend = vitr + (numverts * numcomps);
                    double fdir = (double) dir;
                    while(vitr != vend)
                    {
                        double tmp = vitr[1];
                        vitr[1] = fdir * vitr[2];
                        vitr[2] = fdir * -tmp;
                        vitr += numcomps;
                    }
                }
                break;
            case taa_SCENEMESH_VALUE_INT8:
            case taa_SCENEMESH_VALUE_UINT8:
                {
                    int8_t* vitr = (int8_t*) vsitr->buffer;
                    int8_t* vend = vitr + (numverts * numcomps);
                    while(vitr != vend)
                    {
                        int8_t tmp = vitr[1];
                        vitr[1] = (int8_t) (dir * vitr[2]);
                        vitr[2] = (int8_t) (dir * -tmp);
                        vitr += numcomps;
                    }
                }
                break;
            case taa_SCENEMESH_VALUE_INT16:
            case taa_SCENEMESH_VALUE_UINT16:
                {
                    int16_t* vitr = (int16_t*) vsitr->buffer;
                    int16_t* vend = vitr + (numverts * numcomps);
                    while(vitr != vend)
                    {
                        int16_t tmp = vitr[1];
                        vitr[1] = (int16_t) (dir * vitr[2]);
                        vitr[2] = (int16_t) (dir * -tmp);
                        vitr += numcomps;
                    }
                }
                break;
            case taa_SCENEMESH_VALUE_INT32:
            case taa_SCENEMESH_VALUE_UINT32:
                {
                    int32_t* vitr = (int32_t*) vsitr->buffer;
                    int32_t* vend = vitr + (numverts * numcomps);
                    while(vitr != vend)
                    {
                        int32_t tmp = vitr[1];
                        vitr[1] = (int32_t) (dir * vitr[2]);
                        vitr[2] = (int32_t) (dir * -tmp);
                        vitr += numcomps;
                    }
                }
                break;
            case taa_SCENEMESH_VALUE_MERGED:
                break;
            }
        }
        ++vsitr;
    }
    // fix inverse bind matrices
    taa_mat44_pitch(dir * taa_radians(90.0f), &pitch);
    taa_mat44_transpose(&pitch, &invpitch);
    while(jointitr != jointend)
    {
        taa_mat44 m;
        taa_mat44_multiply(&invpitch, &jointitr->invbindmatrix, &m);
        taa_mat44_multiply(&m, &pitch, &jointitr->invbindmatrix);
        ++jointitr;
    }
}

//****************************************************************************
void taa_scenemesh_triangulate(
    taa_scenemesh* mesh)
{
    uint32_t size;
    uint32_t newnumfaces;
    uint32_t newnumindices;
    uint32_t* newindices;
    taa_scenemesh_face* newfaces;
    taa_scenemesh_face* tri;
    taa_scenemesh_binding* newbindings;
    taa_scenemesh_binding* bindingitr;
    const taa_scenemesh_binding* bindingsrcitr;
    const taa_scenemesh_binding* bindingsrcend;

    size = sizeof(*newindices) * mesh->numindices;
    newindices = (uint32_t*) malloc(size);

    size = sizeof(*newfaces) * mesh->numfaces;
    newfaces = (taa_scenemesh_face*) malloc(size);

    size = sizeof(*newbindings) * mesh->numbindings;
    newbindings = (taa_scenemesh_binding*) malloc(size);

    newnumindices = 0;
    newnumfaces = 0;
    bindingitr = newbindings;
    bindingsrcitr = mesh->bindings;
    bindingsrcend = bindingsrcitr + mesh->numbindings;
    while(bindingsrcitr != bindingsrcend)
    {
        const taa_scenemesh_face* facesrc;
        const taa_scenemesh_face* facesrcend;

        strcpy(bindingitr->name, bindingsrcitr->name);
        bindingitr->materialid = bindingsrcitr->materialid;
        bindingitr->firstface = newnumfaces;

        facesrc = mesh->faces + bindingsrcitr->firstface;
        facesrcend = facesrc + bindingsrcitr->numfaces;
        while(facesrc != facesrcend)
        {
            const uint32_t* indexsrc0;
            const uint32_t* indexsrc1;
            const uint32_t* indexsrc2;
            const uint32_t* indexsrcend;

            assert(facesrc->numvertices>=3); // face needs more verts

            // Create a triangle fan from the polygon
            indexsrc0 = mesh->indices + facesrc->firstindex;
            indexsrc1 = indexsrc0 + mesh->indexsize;
            indexsrc2 = indexsrc1 + mesh->indexsize;
            indexsrcend = indexsrc0 + facesrc->numindices;
            while(indexsrc2 != indexsrcend)
            {
                uint32_t* indexitr;

                // for each vertex after the second, create a new triangle
                // from the first, the previous, and this vertex.
                size = sizeof(*newfaces) * (++newnumfaces);
                newfaces = (taa_scenemesh_face*) realloc(
                    newfaces,
                    size);
                tri = newfaces + newnumfaces - 1;
                tri->firstindex = newnumindices;
                tri->numindices = 3 * mesh->indexsize;
                tri->numvertices = 3;

                // copy the indices into the new index buffer
                newnumindices += tri->numindices;
                size = sizeof(*newindices) * newnumindices;
                newindices = (uint32_t*) realloc(
                    newindices,
                    size);
                // first vert of the source face
                indexitr = newindices + tri->firstindex;
                size = sizeof(*indexitr) * mesh->indexsize;
                memcpy(indexitr, indexsrc0, size);
                // previous vert of the source face
                indexitr += mesh->indexsize;
                memcpy(indexitr, indexsrc1, size);
                // current vert of the source face
                indexitr += mesh->indexsize;
                memcpy(indexitr, indexsrc2, size);

                indexsrc1 += mesh->indexsize;
                indexsrc2 += mesh->indexsize;
            }
            ++facesrc;
        }
        bindingitr->numfaces = newnumfaces - bindingitr->firstface;

        ++bindingitr;
        ++bindingsrcitr;
    }

    free(mesh->indices);
    mesh->indices = newindices;
    mesh->numindices = newnumindices;

    free(mesh->faces);
    mesh->faces = newfaces;
    mesh->numfaces = newnumfaces;

    free(mesh->bindings);
    mesh->bindings = newbindings;
}

#undef taa_FORMAT_LOOP
#undef taa_FORMAT_NEWT
#undef taa_FORMAT_SRCT
#undef taa_FORMAT
