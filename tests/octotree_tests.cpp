#include <algorithm>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "geometry/aabb.hpp"
#include "geometry/octotree.hpp"
#include "geometry/vector3.hpp"

using triangles::AABB;
using triangles::Octotree;
using triangles::OctotreeConfig;
using triangles::Vector3;

namespace {

AABB MakeBox(
    const Vector3& min,
    const Vector3& max
) {
    auto bounds = AABB::Create(min, max);
    EXPECT_TRUE(bounds.has_value());
    return *bounds;
}

AABB MakeDomain() {
    return MakeBox(
        Vector3{-8.0, -8.0, -8.0},
        Vector3{8.0, 8.0, 8.0}
    );
}

OctotreeConfig MakeConfig(
    std::size_t max_depth = 4,
    std::size_t max_objects_per_node = 1
) {
    return OctotreeConfig{
        .max_depth = max_depth,
        .max_objects_per_node = max_objects_per_node
    };
}

bool ContainsId(
    const std::vector<std::size_t>& ids,
    std::size_t id
) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

}

TEST(OctotreeTest, CreateWithValidParameters) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());

    ASSERT_TRUE(tree.has_value());
}

TEST(OctotreeTest, RejectsDegenerateDomain) {
    auto domain = MakeBox(
        Vector3{-8.0, -8.0, -8.0},
        Vector3{8.0, 8.0, -8.0}
    );

    auto tree = Octotree::Create(domain, MakeConfig());

    EXPECT_FALSE(tree.has_value());
}

TEST(OctotreeTest, RejectsZeroNodeCapacity) {
    auto tree = Octotree::Create(
        MakeDomain(),
        MakeConfig(4, 0)
    );

    EXPECT_FALSE(tree.has_value());
}

TEST(OctotreeTest, CreatesTreeWithZeroMaxDepth) {
    auto tree = Octotree::Create(
        MakeDomain(),
        MakeConfig(0, 1)
    );

    EXPECT_TRUE(tree.has_value());
}

TEST(OctotreeTest, InsertsObjectInsideDomain) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto bounds = MakeBox(
        Vector3{-2.0, -2.0, -2.0},
        Vector3{-1.0, -1.0, -1.0}
    );

    EXPECT_TRUE(tree->insert(1, bounds));

    const auto result = tree->query(bounds);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 1);
}

TEST(OctotreeTest, RejectsObjectOutsideDomain) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto bounds = MakeBox(
        Vector3{9.0, 9.0, 9.0},
        Vector3{10.0, 10.0, 10.0}
    );

    EXPECT_FALSE(tree->insert(1, bounds));
    EXPECT_TRUE(tree->query(bounds).empty());
}

TEST(OctotreeTest, RejectsDuplicateObjectId) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-2.0, -2.0, -2.0},
        Vector3{-1.0, -1.0, -1.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{1.0, 1.0, 1.0},
        Vector3{2.0, 2.0, 2.0}
    );

    EXPECT_TRUE(tree->insert(1, first_bounds));
    EXPECT_FALSE(tree->insert(1, second_bounds));

    const auto result = tree->query(second_bounds);
    EXPECT_TRUE(result.empty());
}

TEST(OctotreeTest, QueryReturnsEmptyForEmptyTree) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto query_bounds = MakeBox(
        Vector3{-2.0, -2.0, -2.0},
        Vector3{2.0, 2.0, 2.0}
    );

    EXPECT_TRUE(tree->query(query_bounds).empty());
}

TEST(OctotreeTest, FindsObjectInChildNode) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-7.0, -7.0, -7.0},
        Vector3{-6.0, -6.0, -6.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{6.0, 6.0, 6.0},
        Vector3{7.0, 7.0, 7.0}
    );

    ASSERT_TRUE(tree->insert(1, first_bounds));
    ASSERT_TRUE(tree->insert(2, second_bounds));

    const auto result = tree->query(second_bounds);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 2);
}

TEST(OctotreeTest, FindsObjectStoredInParentNode) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-7.0, -7.0, -7.0},
        Vector3{-6.0, -6.0, -6.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{6.0, 6.0, 6.0},
        Vector3{7.0, 7.0, 7.0}
    );

    ASSERT_TRUE(tree->insert(1, first_bounds));
    ASSERT_TRUE(tree->insert(2, second_bounds));

    const auto spanning_bounds = MakeBox(
        Vector3{-1.0, -1.0, -1.0},
        Vector3{1.0, 1.0, 1.0}
    );

    ASSERT_TRUE(tree->insert(3, spanning_bounds));

    const auto query_bounds = MakeBox(
        Vector3{-0.5, -0.5, -0.5},
        Vector3{0.5, 0.5, 0.5}
    );

    const auto result = tree->query(query_bounds);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 3);
}

TEST(OctotreeTest, DoesNotReturnNonIntersectingObjects) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-7.0, -7.0, -7.0},
        Vector3{-6.0, -6.0, -6.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{6.0, 6.0, 6.0},
        Vector3{7.0, 7.0, 7.0}
    );

    ASSERT_TRUE(tree->insert(1, first_bounds));
    ASSERT_TRUE(tree->insert(2, second_bounds));

    const auto query_bounds = MakeBox(
        Vector3{5.0, 5.0, 5.0},
        Vector3{5.5, 5.5, 5.5}
    );

    EXPECT_TRUE(tree->query(query_bounds).empty());
}

TEST(OctotreeTest, QueryOutsideDomainReturnsEmpty) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto object_bounds = MakeBox(
        Vector3{-2.0, -2.0, -2.0},
        Vector3{-1.0, -1.0, -1.0}
    );

    ASSERT_TRUE(tree->insert(1, object_bounds));

    const auto query_bounds = MakeBox(
        Vector3{20.0, 20.0, 20.0},
        Vector3{21.0, 21.0, 21.0}
    );

    EXPECT_TRUE(tree->query(query_bounds).empty());
}

TEST(OctotreeTest, BoundaryTouchCountsAsIntersection) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto object_bounds = MakeBox(
        Vector3{0.0, 0.0, 0.0},
        Vector3{1.0, 1.0, 1.0}
    );

    ASSERT_TRUE(tree->insert(1, object_bounds));

    const auto query_bounds = MakeBox(
        Vector3{1.0, 1.0, 1.0},
        Vector3{2.0, 2.0, 2.0}
    );

    const auto result = tree->query(query_bounds);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 1);
}

TEST(OctotreeTest, QueryReturnsAllIntersectingObjects) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-2.0, -2.0, -2.0},
        Vector3{-1.0, -1.0, -1.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{1.0, 1.0, 1.0},
        Vector3{2.0, 2.0, 2.0}
    );

    const auto third_bounds = MakeBox(
        Vector3{5.0, 5.0, 5.0},
        Vector3{6.0, 6.0, 6.0}
    );

    ASSERT_TRUE(tree->insert(1, first_bounds));
    ASSERT_TRUE(tree->insert(2, second_bounds));
    ASSERT_TRUE(tree->insert(3, third_bounds));

    const auto query_bounds = MakeBox(
        Vector3{-3.0, -3.0, -3.0},
        Vector3{3.0, 3.0, 3.0}
    );

    const auto result = tree->query(query_bounds);

    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(ContainsId(result, 1));
    EXPECT_TRUE(ContainsId(result, 2));
    EXPECT_FALSE(ContainsId(result, 3));
}

TEST(OctotreeTest, QueryDoesNotReturnDuplicateIds) {
    auto tree = Octotree::Create(MakeDomain(), MakeConfig());
    ASSERT_TRUE(tree.has_value());

    const auto first_bounds = MakeBox(
        Vector3{-7.0, -7.0, -7.0},
        Vector3{-6.0, -6.0, -6.0}
    );

    const auto second_bounds = MakeBox(
        Vector3{6.0, 6.0, 6.0},
        Vector3{7.0, 7.0, 7.0}
    );

    const auto spanning_bounds = MakeBox(
        Vector3{-1.0, -1.0, -1.0},
        Vector3{1.0, 1.0, 1.0}
    );

    ASSERT_TRUE(tree->insert(1, first_bounds));
    ASSERT_TRUE(tree->insert(2, second_bounds));
    ASSERT_TRUE(tree->insert(3, spanning_bounds));

    const auto query_bounds = MakeBox(
        Vector3{-8.0, -8.0, -8.0},
        Vector3{8.0, 8.0, 8.0}
    );

    const auto result = tree->query(query_bounds);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(
        std::count(result.begin(), result.end(), 3),
        1
    );
}

