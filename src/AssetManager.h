#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include "Model.h"
namespace AssetManager {
ModelHandle loadModel(const std::string &filepath,const std::string& texturePath, bool flipYZ = false);
uint8_t *loadTexture(const std::string &filepath, int *texWidth, int *texHeight, int *channelSize);
void unloadTexture(uint8_t *texture);
Model getModel(uint32_t index);
const std::vector<Model>& getModels();
uint32_t getModelCount();
} // namespace AssetManager
#endif