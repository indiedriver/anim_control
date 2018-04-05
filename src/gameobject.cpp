#include "gameobject.hpp"

GameObject::GameObject(void){};

GameObject::GameObject(GameObject* parent, Model* model, glm::vec3 pos,
                       glm::vec3 rot, glm::vec3 sca)
    : parent(parent) {
  _renderAttrib.model = model->attrib.model;
  _renderAttrib.shader_key = model->attrib.shader_key;
  _renderAttrib.vaos = model->attrib.vaos;
  _renderAttrib.bones = model->attrib.bones;
  _renderAttrib.state = model->attrib.state;
  if (model->skeleton != nullptr) {
    animationComponent =
        std::unique_ptr<AnimationComponent>(new AnimationComponent());
    animationComponent->skeleton = Skeleton(*model->skeleton);
  }

  transform.position = pos;
  transform.rotation = rot;
  transform.scale = sca;
}

GameObject::GameObject(GameObject const& src) { *this = src; }

GameObject::~GameObject(void) {}

GameObject& GameObject::operator=(GameObject const& rhs) {
  if (this != &rhs) {
    /*
    this->_renderAttrib = rhs._renderAttrib;
    this->_renderAttrib.shader = rhs._renderAttrib.shader;
    this->_renderAttrib.vao = rhs._renderAttrib.vao;
    this->_renderAttrib.transforms = rhs._renderAttrib.transforms;
    this->_renderAttrib.texture = rhs._renderAttrib.texture;
    this->inputComponent =
        rhs.inputComponent ? new InputComponent(*rhs.inputComponent) : nullptr;
    this->physicsComponent = rhs.physicsComponent
                                 ? new PhysicsComponent(*rhs.physicsComponent)
                                 : nullptr;*/
    transform.position = rhs.transform.position;
    transform.rotation = rhs.transform.rotation;
    transform.scale = rhs.transform.scale;
    parent = rhs.parent;
  }
  return (*this);
}

void GameObject::update(Env& env, AnimData* data) {
  if (inputComponent) {
    inputComponent->update(*this, env.inputHandler);
  }
  if (physicsComponent != nullptr) {
    physicsComponent->update(*this, env.getDeltaTime());
  }
  // this->positionRelative = this->transform.position - oldPosition;
  _renderAttrib.model = getWorldTransform();
  if (animationComponent) {
    // animationComponent->update();
    animationComponent->updateBones(env.getAbsoluteTime(), _renderAttrib.bones,
                                    data);
    animationComponent->updateAnimDebugAttrib(_renderAttrib.model);
  }
}

glm::mat4 GameObject::getWorldTransform() {
  glm::mat4 worldTransform = transform.toModelMatrix();
  if (this->parent != nullptr) {
    worldTransform = this->parent->getWorldTransform() * worldTransform;
  }
  return (worldTransform);
}

const render::Attrib GameObject::getRenderAttrib() const {
  return (this->_renderAttrib);
}

const render::Attrib GameObject::getDebugRenderAttrib() const {
  if (animationComponent) {
    return (animationComponent->skel_attrib);
  }
  render::Attrib dummy;
  return (dummy);
}
void GameObject::setAsContralable() {
  inputComponent = std::unique_ptr<InputComponent>(new InputComponent());
  physicsComponent = std::unique_ptr<PhysicsComponent>(new PhysicsComponent());
}

AnimationComponent::AnimationComponent(void) {}

AnimationComponent::AnimationComponent(AnimationComponent const& src) {
  *this = src;
}

AnimationComponent::~AnimationComponent(void) {}

AnimationComponent& AnimationComponent::operator=(
    AnimationComponent const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

void AnimationComponent::updateBones(float timestamp,
                                     std::vector<glm::mat4>& bones,
                                     AnimData* data) {
  for (auto node_it : skeleton.node_ids) {
    std::string node_name = node_it.first;
    unsigned short bone_index = node_it.second;
    skeleton.local_poses[bone_index] = animate(data, node_name, timestamp);
  }

  skeleton.local_to_global();
  bones.resize(skeleton.joint_count);
  for (unsigned short i = 0; i < skeleton.joint_count; i++) {
    bones[i] = skeleton.global_inverse * skeleton.global_poses[i] *
               skeleton.offsets[i];
  }
}

void AnimationComponent::updateAnimDebugAttrib(glm::mat4 parent_model) {
  skel_attrib.state.primitiveMode = render::PrimitiveMode::Lines;
  skel_attrib.state.depthTestFunc = render::DepthTestFunc::Always;
  skel_attrib.model = parent_model;
  skel_attrib.shader_key = "anim_debug";
  std::vector<glm::vec3> positions;
  for (unsigned short i = 0; i < skeleton.joint_count; i++) {
    const unsigned short parent_joint = skeleton.hierarchy[i];
    glm::mat4 bone_offset = skeleton.global_inverse * skeleton.global_poses[i];
    glm::mat4 parent_bone_offset =
        skeleton.global_inverse * skeleton.global_poses[parent_joint];

    glm::vec3 point_offset =
        glm::vec3(bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 parent_point_offset =
        glm::vec3(parent_bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    positions.push_back(point_offset);
    positions.push_back(parent_point_offset);
  }
  if (skel_attrib.vaos.size() && skel_attrib.vaos[0] != nullptr) {
    skel_attrib.vaos[0]->update(positions);
  } else {
    skel_attrib.vaos.push_back(new VAO(positions));
  }
}

InputComponent::InputComponent(void){};

InputComponent::~InputComponent(void){};

InputComponent::InputComponent(InputComponent const& src) { *this = src; }

InputComponent& InputComponent::operator=(InputComponent const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

void InputComponent::update(GameObject& go, InputHandler& inputHandler) {
  glm::vec3 new_pos = go.transform.position;
  InputDirection direction = InputDirection::None;
  if (inputHandler.keys[GLFW_KEY_A]) {
    inputHandler.keys[GLFW_KEY_A] = false;
    direction = InputDirection::East;
  } else if (inputHandler.keys[GLFW_KEY_D]) {
    inputHandler.keys[GLFW_KEY_D] = false;
    direction = InputDirection::West;
  } else if (inputHandler.keys[GLFW_KEY_W]) {
    inputHandler.keys[GLFW_KEY_W] = false;
    direction = InputDirection::North;
  } else if (inputHandler.keys[GLFW_KEY_S]) {
    inputHandler.keys[GLFW_KEY_S] = false;
    direction = InputDirection::South;
  }
  if (direction != InputDirection::None) {
    directions.push(direction);
  }
  if (go.physicsComponent) {
    if (directions.empty() == false && go.physicsComponent->reached_target) {
      go.physicsComponent->reached_target = false;
      glm::vec3 dir_vec[4] = {{0.0f, 0.0f, 1.0f},
                              {1.0f, 0.0f, 0.0f},
                              {0.0f, 0.0f, -1.0f},
                              {-1.0f, 0.0f, 0.0f}};
      unsigned int direction_index =
          static_cast<unsigned int>(directions.front());
      go.physicsComponent->target_position =
          go.transform.position + dir_vec[direction_index];
    }
  }
}

PhysicsComponent::PhysicsComponent(void){};

PhysicsComponent::~PhysicsComponent(void){};

PhysicsComponent::PhysicsComponent(PhysicsComponent const& src) { *this = src; }

PhysicsComponent& PhysicsComponent::operator=(PhysicsComponent const& rhs) {
  if (this != &rhs) {
    this->velocity = rhs.velocity;
  }
  return (*this);
}

void PhysicsComponent::update(GameObject& go, float dt) {
  float dist = glm::distance(go.transform.position, target_position);
  if (dist > 0.01f) {
    glm::vec3 dir = glm::normalize(target_position - go.transform.position);
    go.transform.position += dir * dt * 2.5f * speed;
    glm::mat4 rot = glm::lookAt(go.transform.position, target_position,
                                glm::vec3(0.0f, 1.0f, 0.0f));
    go.transform.rotation = glm::eulerAngles(glm::quat_cast(rot));
  } else {
    reached_target = true;
    go.transform.position = glm::round(go.transform.position);
    target_position = go.transform.position;
    if (go.inputComponent) {
      if (go.inputComponent->directions.empty() == false) {
        go.inputComponent->directions.pop();
      }
    }
  }
}
