#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define NUM_BONES_PER_VERTEX 4
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

struct VertexBoneData {
  glm::ivec4 bone_ids = {0, 0, 0, 0};
  glm::vec4 weights = {0, 0, 0, 0};
};

struct Vertex {
  glm::vec3 position = {0, 0, 0};
  glm::ivec4 bone_ids = {0, 0, 0, 0};
  glm::vec4 weights = {0, 0, 0, 0};

  Vertex() : position({0.0f, 0.0f, 0.0f}){};
  Vertex(Vertex const& src) { *this = src; }

  Vertex& operator=(Vertex const& rhs) {
    if (this != &rhs) {
      this->position = rhs.position;
      this->bone_ids = rhs.bone_ids;
      this->weights = rhs.weights;
    }
    return (*this);
  };
};
