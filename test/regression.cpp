#include <kdtreepp.hpp>
#include <cstdlib>
#include <iostream>
#include <random>

void check(bool value) { if (!value) { std::cerr << "check failed\n"; std::exit(1); } }

int main() {
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
