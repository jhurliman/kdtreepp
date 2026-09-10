#include <kdtreepp.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <cmath>

void check(bool value) { if (!value) { std::cerr << "check failed\n"; std::exit(1); } }

using Vector2 = Eigen::Vector2d;
using Vector3 = Eigen::Vector3d;
using AlignedBox2 = Eigen::AlignedBox2d;
using AlignedBox3 = Eigen::AlignedBox3d;

static void CHECK_VEC3_EQ(const Vector3& a, const Vector3& b) {
  check(std::abs(a[0] - b[0]) < 1e-12);
  check(std::abs(a[1] - b[1]) < 1e-12);
  check(std::abs(a[2] - b[2]) < 1e-12);
}

static void checkVersion() { check(KDTREEPP_VERSION_STRING == std::string("2.0.0")); }

static void checkPointTree() {
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
  check(numBoundsChecks < count);
  check(numPointChecks < count);

  // Count points within a bounding box
  AlignedBox3 searchBounds{Vector3{-500.0, -500.0, -500.0}, Vector3{500.0, 500.0, 500.0}};
  size_t foundPoints = 0;
  node.visit([&searchBounds](const AlignedBox3& bounds) { return searchBounds.intersects(bounds); },
             [&searchBounds, &foundPoints](const Vector3& point) {
               if (searchBounds.contains(point)) {
                 ++foundPoints;
               }
             });

  check(foundPoints > count / 10);
  check(foundPoints < count / 2);
}

static void checkModifyInPlace() {
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

  check(numBoundsChecks < count);
  check(numPointChecks == count);

  for (const auto& point : points) {
    check(point.x() < 0.0);
    check(point.y() < 0.0);
  }
}

int main() {
  checkVersion();
  checkPointTree();
  checkModifyInPlace();
  using Point = Eigen::Vector3d;
  for (int count : {0, 1, 2, 8, 9, 1500}) {
    std::mt19937 random(42);
    std::uniform_real_distribution<double> distribution(-100, 100);
    std::vector<Point> points;
    for (int i = 0; i < count; ++i) points.emplace_back(distribution(random), distribution(random), distribution(random));
    const auto tree = kdtreepp::MakeEigenKdTreeNode<double, 3>(
        points.begin(), points.end(), [](const Point& p) { return p; }, [](const Point& p) { return p; });
    int visited = 0;
    tree.visit([](const auto&) { return true; }, [&](const Point&) { ++visited; });
    check(visited == count);
    if (!count) { check(tree.bounds().isEmpty()); continue; }
    for (int query = 0; query < 100; ++query) {
      const Point target(distribution(random), distribution(random), distribution(random));
      double brute = std::numeric_limits<double>::infinity();
      for (const auto& point : points) brute = std::min(brute, (point - target).squaredNorm());
      double found = std::numeric_limits<double>::infinity();
      tree.visit([&](const auto& box) { return box.squaredExteriorDistance(target) <= found; },
                 [&](const Point& point) { found = std::min(found, (point - target).squaredNorm()); });
      check(std::abs(brute - found) < 1e-9);
    }
  }
  std::vector<Point> identical(100, Point::Zero());
  const auto tree = kdtreepp::MakeEigenKdTreeNode<double, 3>(identical.begin(), identical.end(), [](const Point& p) { return p; }, [](const Point& p) { return p; });
  int visited = 0;
  tree.visit([](const auto&) { return true; }, [&](const auto&) { ++visited; });
  check(visited == 100);
}
