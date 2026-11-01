#pragma once

#include "aabb.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

namespace triangles {

struct OctotreeConfig {
    std::size_t max_depth{8};
    std::size_t max_objects_per_node{8};
};

class Octotree {
  private:
    struct Entry {
        std::size_t object_id;
        AABB bounds;
    };

    struct Node {
        AABB bounds;

        std::vector<Entry> entries;

        std::array<std::unique_ptr<Node>, 8> children;

        explicit Node(const AABB &bounds);

        bool is_leaf() const noexcept;
    };

    AABB domain_;
    OctotreeConfig config_;

    std::unique_ptr<Node> root_;

    std::unordered_set<std::size_t> object_ids_;

    Octotree(const AABB &domain, const OctotreeConfig &config);

    bool subdivide(Node &node);

    std::optional<std::size_t> find_containing_child(const Node &node,
                                                     const AABB &bounds) const;

    void insert(Node &node, Entry entry, std::size_t depth);

    void query(const Node &node, const AABB &bounds,
               std::vector<std::size_t> &result) const;

  public:
    static std::optional<Octotree> Create(const AABB &domain,
                                          const OctotreeConfig &config = {});

    Octotree(const Octotree &) = delete;
    Octotree &operator=(const Octotree &) = delete;

    Octotree(Octotree &&) noexcept = default;
    Octotree &operator=(Octotree &&) noexcept = default;

    bool insert(std::size_t object_id, const AABB &bounds);

    std::vector<std::size_t> query(const AABB &bounds) const;
};

} // namespace triangles
