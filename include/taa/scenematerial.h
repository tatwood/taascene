/**
 * @brief     material processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_SCENEMATERIAL_H_
#define taa_SCENEMATERIAL_H_

#include <taa/system.h>

//****************************************************************************
// constants

enum { taa_SCENEMATERIAL_NAMESIZE = 32 };

//****************************************************************************
// typedefs

typedef struct taa_scenematerial_s taa_scenematerial;

//****************************************************************************
// structs

struct taa_scenematerial_s
{
    char name[taa_SCENEMATERIAL_NAMESIZE];
    int32_t diffusetexture;
};

//****************************************************************************
// functions

void taa_scenematerial_create(
    const char* name,
    taa_scenematerial* mat_out);

void taa_scenematerial_destroy(
    taa_scenematerial* mat);


#endif // taa_SCENEMATERIAL_H_
