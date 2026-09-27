#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include "Model.h"
namespace AssetManager {
    Model loadModel(const std::string& filepath,bool flipYZ = false);
}
#endif