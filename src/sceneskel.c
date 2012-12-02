/**
 * @brief     skeleton processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/sceneskel.h>
#include <taa/mat44.h>
#include <taa/quat.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//****************************************************************************
uint32_t taa_sceneskel_add_joint(
    taa_sceneskel* skel,
    int32_t parent,
    int32_t nodeid)
{
    uint32_t jointid = skel->numjoints;
    taa_sceneskel_joint* joint;
    taa_sceneskel_resize_joints(skel, jointid + 1);
    joint = skel->joints + jointid;
    joint->parent = parent;
    joint->nodeid = nodeid;
    return jointid;
}

//****************************************************************************
void taa_sceneskel_calc_rotate(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_quat* q_out)
{
    const taa_sceneskel_joint* joint = skel->joints + jointid;
    int32_t nodeid = joint->nodeid;
    int32_t nodeend= (joint->parent>=0)?skel->joints[joint->parent].nodeid:-1;
    taa_quat_identity(q_out);
    do
    {
        const taa_scenenode* node = nodes + nodeid;
        assert( joint->nodeid >= 0); // invalid joint hierarchy
        if(node->type == taa_SCENENODE_TRANSFORM_ROTATE)
        {
            taa_quat q = *q_out;
            float rad;
            taa_vec4 axis;
            taa_quat qn;
            axis = node->value.rotate;
            rad = axis.w;
            axis.w = 0.0f;
            taa_quat_axisangle(rad, &axis, &qn);
            taa_quat_multiply(&qn, &q, q_out);
        }
        else if(node->type == taa_SCENENODE_TRANSFORM_MATRIX)
        {
            assert(0); // TODO
        }
        nodeid = node->parent;
    }
    while(nodeid != nodeend);
}

//****************************************************************************
void taa_sceneskel_calc_scale(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_vec4* v_out)
{
    taa_mat44 M;
    taa_mat44 T;
    taa_sceneskel_calc_transform(skel, nodes, jointid, &M);
    taa_vec4_set(0.0f, 0.0f, 0.0f, 1.0f, &M.w);
    taa_mat44_transpose(&M, &T);
    v_out->x = taa_vec4_length(&T.x);
    v_out->y = taa_vec4_length(&T.y);
    v_out->z = taa_vec4_length(&T.z);
    v_out->w = 1.0f;
}

//****************************************************************************
void taa_sceneskel_calc_transform(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_mat44* m_out)
{
    const taa_sceneskel_joint* joint = skel->joints + jointid;
    int32_t nodeid = joint->nodeid;
    int32_t nodeend= (joint->parent>=0)?skel->joints[joint->parent].nodeid:-1;
    taa_mat44_identity(m_out);
    do
    {
        const taa_scenenode* node = nodes + nodeid;
        taa_mat44 M = *m_out;
        taa_mat44 N;
        taa_vec4 v;
        float f;
        assert(nodeid >= 0); // invalid joint hierarchy
        switch(node->type)
        {
        case taa_SCENENODE_TRANSFORM_MATRIX:
            taa_mat44_multiply(&node->value.matrix, &M, m_out);
            break;
        case taa_SCENENODE_TRANSFORM_ROTATE:
            v = node->value.rotate;
            f = v.w;
            v.w = 0.0f;
            taa_mat44_axisangle(f, &v, &N);
            taa_mat44_multiply(&N, &M, m_out);
            break;
        case taa_SCENENODE_TRANSFORM_SCALE:
            v = node->value.translate;
            taa_mat44_from_scale(&v, &N);
            taa_mat44_multiply(&N, &M, m_out);
            break;
        case taa_SCENENODE_TRANSFORM_TRANSLATE:
            v = node->value.scale;
            taa_mat44_from_translate(&v, &N);
            taa_mat44_multiply(&N, &M, m_out);
            break;
        default:
            break;
        }
        nodeid = node->parent;
    }
    while(nodeid != nodeend);
}

//****************************************************************************
void taa_sceneskel_calc_translate(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    int32_t jointid,
    taa_vec4* v_out)
{
    taa_mat44 M;
    taa_sceneskel_calc_transform(skel, nodes, jointid, &M);
    v_out->x = M.w.x;
    v_out->y = M.w.y;
    v_out->z = M.w.z;
    v_out->w = 0.0f;
}

//****************************************************************************
void taa_sceneskel_create(
    const char* name,
    taa_sceneskel* skel_out)
{
    memset(skel_out, 0, sizeof(*skel_out));
    strncpy(skel_out->name, name, sizeof(skel_out->name)-1);
}

//****************************************************************************
void taa_sceneskel_destroy(
    taa_sceneskel* skel)
{
    free(skel->joints);
}

//****************************************************************************
int32_t taa_sceneskel_find_joint(
    const taa_sceneskel* skel,
    const taa_scenenode* nodes,
    const char* name)
{
    int32_t jointid = -1;
    const taa_sceneskel_joint* jitr = skel->joints;
    const taa_sceneskel_joint* jend = jitr + skel->numjoints;
    while(jitr != jend)
    {
        if(!strcmp(nodes[jitr->nodeid].name, name))
        {
            jointid = (int32_t)(ptrdiff_t) (jitr - skel->joints);
            break;
        }
        ++jitr;
    }
    return jointid;
}

//****************************************************************************
void taa_sceneskel_resize_joints(
    taa_sceneskel* skel,
    uint32_t numjoints)
{
    uint32_t oldnum = skel->numjoints;
    taa_sceneskel_joint* joint = skel->joints;
    taa_sceneskel_joint* jointend;
    if(numjoints > oldnum)
    {
        uint32_t cap = (oldnum   +31) & ~31;
        uint32_t ncap= (numjoints+31) & ~31;
        if(cap != ncap)
        {
            joint = (taa_sceneskel_joint*) realloc(
                joint,
                ncap * sizeof(*joint));
            skel->joints = joint;
        }
        jointend = joint + numjoints;
        joint += oldnum;
        while(joint != jointend)
        {
            joint->parent = -1;
            joint->nodeid = -1;
            ++joint;
        }
    }
    skel->numjoints = numjoints;
}
