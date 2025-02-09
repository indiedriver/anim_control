#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <cstring>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include "anim.hpp"
#include "animation.hpp"
#include "model.hpp"
#include "renderer.hpp"

struct BoneInfo {
  glm::mat4 offset;
};

struct TextureInfo {
  std::string type;
  Texture* texture;
};

class MeshLoader {
 public:
  MeshLoader(void);
  MeshLoader(MeshLoader const& src);
  ~MeshLoader(void);
  MeshLoader& operator=(MeshLoader const& rhs);
  Model* loadScene(std::string filename);
  Model* loadModel(std::vector<glm::vec3> vertices);
  Model* loadModel(std::vector<glm::vec3> vertices,
                   std::vector<unsigned int> indices);

 private:
  Model* current_model;
  std::unordered_map<std::string, BoneInfo> bone_map;
  std::unordered_map<std::string, unsigned short> node_map;
  std::unordered_map<std::string, TextureInfo> texture_map;
  std::string file_name;

  void processHierarchy(const aiScene* scene, aiNode* node,
                        unsigned short parent_id,
                        std::vector<unsigned short>& hierarchy);
  void parseNodeHierarchy(const aiScene* scene, aiNode* node,
                          std::queue<unsigned short>& node_stack);
  void parseBoneHierarchy(const aiScene* scene, aiNode* node);
  void processMesh(const aiScene* scene, const aiMesh* mesh);
  void loadAnimations(const aiScene* scene);
  void loadMeshBones(const aiMesh* mesh);
  void populateVerticesBoneInfo(const aiMesh* mesh,
                                std::vector<Vertex>& vertices);
  void setBoneData(Vertex& vertex, unsigned int bone_id, float weight);
  void loadTextures(const aiMaterial* material, aiTextureType type);
  void setupSkeleton(const aiScene* scene);
  void reset();
};
