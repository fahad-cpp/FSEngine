#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include "Model.h"
namespace AssetManager {
Model    loadModel(const std::string &filepath, bool flipYZ = false);
uint8_t *loadTexture(const std::string &filepath, int *texWidth, int *texHeight, int *channelSize);
void     unloadTexture(uint8_t *texture);
} // namespace AssetManager
#endif