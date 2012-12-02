#include <taa/scenematerial.h>
#include <string.h>

//****************************************************************************
void taa_scenematerial_create(
    const char* name,
    taa_scenematerial* mat_out)
{
    memset(mat_out, 0, sizeof(*mat_out));
    strncpy(mat_out->name, name, sizeof(mat_out->name)-1);
    mat_out->diffusetexture = -1;
}

//****************************************************************************
void taa_scenematerial_destroy(
    taa_scenematerial* mat)
{
}
