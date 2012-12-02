/**
 * @brief     scene graph node processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/scenenode.h>
#include <taa/mat44.h>

//****************************************************************************
void taa_scenenode_calc_transform(
    const taa_scenenode* nodes,
    int nodeid,
    taa_mat44* m_out)
{
    taa_mat44_identity(m_out);
    do
    {
        const taa_scenenode* node = nodes + nodeid;
        taa_mat44 M = *m_out;
        taa_mat44 N;
        taa_vec4 v;
        float f;
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
    while(nodeid != -1);
}

//****************************************************************************
void taa_scenenode_create(
    const char* name,
    taa_scenenode_type type,
    int parentid,
    taa_scenenode* node_out)
{
    memset(node_out, 0, sizeof(*node_out));
    strncpy(node_out->name, name, sizeof(node_out->name)-1);
    node_out->type = type;
    node_out->parent = parentid;
}

//****************************************************************************
void taa_scenenode_destroy(
    taa_scenenode* node)
{
}

//****************************************************************************
void taa_scenenode_rotate_upaxis(
    taa_scenenode* node,
    int32_t dir)
{
    float fdir = (float) dir;
    switch(node->type)
    {
    case taa_SCENENODE_EMPTY:
        break;
    case taa_SCENENODE_REF_JOINT:
        break;
    case taa_SCENENODE_REF_MESH:
        break;
    case taa_SCENENODE_REF_SKEL:
        break;
    case taa_SCENENODE_TRANSFORM_MATRIX:
        {
            taa_mat44 pitch;
            taa_mat44 invpitch;
            taa_mat44 m;
            taa_mat44_pitch(dir * taa_radians(90.0f), &pitch);
            taa_mat44_transpose(&pitch, &invpitch);
            taa_mat44_multiply(&invpitch, &node->value.matrix, &m);
            taa_mat44_multiply(&m, &pitch, &node->value.matrix);
        }
        break;
    case taa_SCENENODE_TRANSFORM_ROTATE:
        {
            float tmp = node->value.rotate.y;
            node->value.rotate.y = fdir * node->value.rotate.z;
            node->value.rotate.z = fdir * -tmp;
        }
        break;
    case taa_SCENENODE_TRANSFORM_SCALE:
        {
            float tmp = node->value.scale.y;
            node->value.scale.y = node->value.scale.z;
            node->value.scale.z = tmp;
        }
        break;
    case taa_SCENENODE_TRANSFORM_TRANSLATE:
        {
            float tmp = node->value.translate.y;
            node->value.translate.y = fdir * node->value.translate.z;
            node->value.translate.z = fdir * -tmp;
        }
        break;
    }
}
