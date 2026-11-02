#include "geometry/octotree.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace triangles {

Octotree::Node::Node(const AABB &bounds) : bounds(bounds) {}

bool Octotree::Node::is_leaf() const noexcept {
    return std::all_of(children.begin(), children.end(),
                       [](const auto &child) { return child == nullptr; });
}

Octotree::Octotree(const AABB &domain, const OctotreeConfig &config)
    : domain_(domain), config_(config), root_(std::make_unique<Node>(domain)) {}

std::optional<Octotree> Octotree::Create(const AABB &domain,
                                         const OctotreeConfig &config,
                                         std::string *error) {
    if (error) {
        error->clear();
    }

    const auto fail = [error](const char *message) -> std::optional<Octotree> {
        if (error) {
            *error = message;
        }

        return std::nullopt;
    };

    if (config.max_objects_per_node == 0) {
        return fail("Maximum objects per node must be positive.");
    }

    const Vector3 &min = domain.get_min();
    const Vector3 &max = domain.get_max();

    if (min.x >= max.x || min.y >= max.y || min.z >= max.z) {
        return fail("Domain must have positive extent along every axis.");
    }

    return Octotree(domain, config);
}

bool Octotree::subdivide(Node &node) {
    if (!node.is_leaf()) {
        return false;
    }

    const Vector3 min = node.bounds.get_min();
    const Vector3 max = node.bounds.get_max();

    const Vector3 center = node.bounds.center();

    if (center.x <= min.x || center.x >= max.x || center.y <= min.y ||
        center.y >= max.y || center.z <= min.z || center.z >= max.z) {
        return false;
    }

    std::array<std::unique_ptr<Node>, 8> children;

    for (std::size_t index = 0; index < children.size(); ++index) {
        const bool upper_x = (index & 1u) != 0;
        const bool upper_y = (index & 2u) != 0;
        const bool upper_z = (index & 4u) != 0;

        const Vector3 child_min{upper_x ? center.x : min.x,
                                upper_y ? center.y : min.y,
                                upper_z ? center.z : min.z};

        const Vector3 child_max{upper_x ? max.x : center.x,
                                upper_y ? max.y : center.y,
                                upper_z ? max.z : center.z};

        const auto child_bounds = AABB::Create(child_min, child_max);

        if (!child_bounds) {
            return false;
        }

        children[index] = std::make_unique<Node>(*child_bounds);
    }

    node.children = std::move(children);

    return true;
}

Octotree::Node *Octotree::find_containing_child(Node &node,
                                                const AABB &bounds) const {
    for (auto &child : node.children) {
        if (child && child->bounds.contains(bounds)) {
            return child.get();
        }
    }

    return nullptr;
}

void Octotree::insert(Node &node, Entry entry, std::size_t depth) {
    if (!node.is_leaf()) {
        if (auto *child = find_containing_child(node, entry.bounds)) {
            insert(*child, std::move(entry), depth + 1);
        } else {
            node.entries.push_back(std::move(entry));
        }

        return;
    }

    node.entries.push_back(std::move(entry));

    if (node.entries.size() <= config_.max_objects_per_node ||
        depth >= config_.max_depth) {
        return;
    }

    if (!subdivide(node)) {
        return;
    }

    std::vector<Entry> remaining;
    remaining.reserve(node.entries.size());

    for (auto &existing : node.entries) {
        if (auto *child = find_containing_child(node, existing.bounds)) {
            insert(*child, std::move(existing), depth + 1);
        } else {
            remaining.push_back(std::move(existing));
        }
    }

    node.entries = std::move(remaining);
}

bool Octotree::insert(std::size_t object_id, const AABB &bounds) {
    if (!domain_.contains(bounds)) {
        return false;
    }

    if (!object_ids_.insert(object_id).second) {
        return false;
    }

    insert(*root_, Entry{object_id, bounds}, 0);

    return true;
}

void Octotree::query(const Node &node, const AABB &bounds,
                     std::vector<std::size_t> &result) const {
    if (!node.bounds.intersects(bounds)) {
        return;
    }

    for (const auto &entry : node.entries) {
        if (entry.bounds.intersects(bounds)) {
            result.push_back(entry.object_id);
        }
    }

    for (const auto &child : node.children) {
        if (child) {
            query(*child, bounds, result);
        }
    }
}

std::vector<std::size_t> Octotree::query(const AABB &bounds) const {
    std::vector<std::size_t> result;
    query(*root_, bounds, result);

    return result;
}

} // namespace triangles
