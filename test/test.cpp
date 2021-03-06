#include <Eigen/StdVector>
#include <kdtreepp.hpp>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using Vector2 = Eigen::Vector2d;
using Vector3 = Eigen::Vector3d;
using AlignedBox2 = Eigen::AlignedBox2d;
using AlignedBox3 = Eigen::AlignedBox3d;

static void CHECK_VEC3_EQ(const Vector3& a, const Vector3& b) {
  CHECK(a[0] == Approx(b[0]));
  CHECK(a[1] == Approx(b[1]));
  CHECK(a[2] == Approx(b[2]));
}

TEST_CASE("VERSION_STRING") { REQUIRE(KDTREEPP_VERSION_STRING == std::string("1.0.0")); }

TEST_CASE("PointTreeTest") {
  // Build a k-d tree from a list of points

  std::vector<Vector3, Eigen::aligned_allocator<Vector3>> points;
  std::mt19937_64 randGen{size_t(42)};
  std::uniform_real_distribution<double> dist{-1000.0, 1000.0};

  // Make random points
  constexpr size_t count = 1500;
  points.resize(count);
  for (auto& point : points) {
    point << dist(randGen), dist(randGen), dist(randGen);
  }

  const auto node = kdtreepp::MakeEigenKdTreeNode<double, 3>(
      points.begin(), points.end(), [](const Vector3& p) { return p; },
      [](const Vector3& p) { return p; });

  const Vector3 checkPoint{dist(randGen), dist(randGen), dist(randGen)};

  // Find closest point via brute force search
  double bruteMinDistSq = std::numeric_limits<double>::max();
  Vector3 bruteClosestPoint;
  for (const auto& point : points) {
    const double rSq = (point - checkPoint).squaredNorm();
    if (rSq < bruteMinDistSq) {
      bruteMinDistSq = rSq;
      bruteClosestPoint = point;
    }
  }

  // Find closest point using kdtree
  double minDistSq = std::numeric_limits<double>::max();
  Vector3 closestPoint;
  size_t numBoundsChecks = 0;
  size_t numPointChecks = 0;
  node.visit(
      [&minDistSq, checkPoint, &numBoundsChecks](const AlignedBox3& bounds) {
        ++numBoundsChecks;
        return bounds.squaredExteriorDistance(checkPoint) < minDistSq;
      },

      [&minDistSq, &closestPoint, checkPoint, &numPointChecks](const Vector3& point) {
        ++numPointChecks;
        const double rSq = (point - checkPoint).squaredNorm();
        if (rSq < minDistSq) {
          minDistSq = rSq;
          closestPoint = point;
        }
      });

  CHECK_VEC3_EQ(bruteClosestPoint, closestPoint);
  CHECK(numBoundsChecks < count);
  CHECK(numPointChecks < count);

  // Count points within a bounding box
  AlignedBox3 searchBounds{Vector3{-500.0, -500.0, -500.0}, Vector3{500.0, 500.0, 500.0}};
  size_t foundPoints = 0;
  node.visit([&searchBounds](const AlignedBox3& bounds) { return searchBounds.intersects(bounds); },
             [&searchBounds, &foundPoints](const Vector3& point) {
               if (searchBounds.contains(point)) {
                 ++foundPoints;
               }
             });

  CHECK(foundPoints > count / 10);
  CHECK(foundPoints < count / 2);
}

TEST_CASE("ModifyInPlace") {
  // Build a k-d tree from a list of points

  std::vector<Vector2, Eigen::aligned_allocator<Vector2>> points;
  std::mt19937_64 randGen{size_t(42)};
  std::uniform_real_distribution<double> dist{0.1, 1.0};

  // Make random points
  constexpr size_t count = 100;
  points.resize(count);
  for (auto& point : points) {
    point << dist(randGen), dist(randGen);
  }

  auto node = kdtreepp::MakeEigenKdTreeNode<double, 2>(
      points.begin(), points.end(), [](const Vector2& p) { return p; },
      [](const Vector2& p) { return p; });

  size_t numBoundsChecks = 0;
  size_t numPointChecks = 0;
  node.visit(
      [&numBoundsChecks](const AlignedBox2& bounds) {
        (void)bounds;
        ++numBoundsChecks;
        return true;
      },

      [&numPointChecks](Vector2& point) {
        ++numPointChecks;
        point *= -1.0;
      });

  CHECK(numBoundsChecks < count);
  CHECK(numPointChecks == count);

  for (const auto& point : points) {
    CHECK(point.x() < 0.0);
    CHECK(point.y() < 0.0);
  }
}
