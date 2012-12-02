#ifndef taa_SCENEFILE_H_
#define taa_SCENEFILE_H_

#include "scene.h"
#include <taa/filestream.h>

/**
 * @return 0 on success, -1 on error
 */
taa_SCENE_LINKAGE int32_t taa_scenefile_deserialize(
    taa_filestream* fs,
    taa_scene* scene);

taa_SCENE_LINKAGE void taa_scenefile_serialize(
    const taa_scene* scene,
    taa_filestream* fs);


#endif // taa_SCENEFILE_H_
