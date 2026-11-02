#ifndef SCENE_CONFIG_HPP_
#define SCENE_CONFIG_HPP_

#include "generator.hpp"
#include "octotree.hpp"

#include <cstddef>

namespace triangles {

struct SceneConfig {
    GeneratorConfig generator;
    OctotreeConfig octotree;
    std::size_t triangles_count{4188};
};

} // namespace triangles

#endif // SCENE_CONFIG_HPP_
