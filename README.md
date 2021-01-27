# kdtreepp

### A C++ k-d tree implementation

[![Build Status](https://travis-ci.com/jhurliman/kdtreepp.svg?branch=main)](https://travis-ci.com/jhurliman/kdtreepp)
[![codecov](https://codecov.io/gh/jhurliman/kdtreepp/branch/main/graph/badge.svg)](https://codecov.io/gh/jhurliman/kdtreepp)

## Usage

### Search for the closest 3D point

```cpp
#include <Eigen/StdVector>
#include <iostream>
#include <random>
#include <vector>

#include "kdtreepp.hpp"

using Vector3 = Eigen::Vector3d;
using AlignedBox3 = Eigen::AlignedBox3d;

int main() {
  std::vector<Vector3, Eigen::aligned_allocator<Vector3>> points;
  std::mt19937_64 randGen{size_t(42)};
  std::uniform_real_distribution<double> dist{-1000.0, 1000.0};

  // Make random points
  points.resize(size_t(5000));
  for (auto& point : points) {
    point << dist(randGen), dist(randGen), dist(randGen);
  }

  // Construct a k-d tree from 3d points
  const auto node = kdtreepp::MakeEigenKdTreeNode<double, 3>(
      points.begin(), points.end(), [](const Vector3& p) { return p; },
      [](const Vector3& p) { return p; });

  // Create a random query point
  const Vector3 queryPoint{dist(randGen), dist(randGen), dist(randGen)};

  // Find the closest point to the given query point
  double minDistSq = std::numeric_limits<double>::max();
  Vector3 closestPoint;
  node.visit(
      [&minDistSq, queryPoint](const AlignedBox3& bounds) {
        return bounds.squaredExteriorDistance(queryPoint) < minDistSq;
      },
      [&minDistSq, &closestPoint, queryPoint](const Vector3& point) {
        const double rSq = (point - queryPoint).squaredNorm();
        if (rSq < minDistSq) {
          minDistSq = rSq;
          closestPoint = point;
        }
      });

  std::cout << "Closest point to " << queryPoint << " is " << closestPoint << "\n";
}
```

## Test

```shell
# build test binaries
make

# run tests
make test

# run bench tests
make bench
```

The default test binaries will be built in release mode. You can make Debug test binaries as well:

```shell
make clean
make debug
make test
```

Enable additional sanitizers to catch hard-to-find bugs, for example:

```shell
export LDFLAGS="-fsanitize=address,undefined"
export CXXFLAGS="-fsanitize=address,undefined"

make
```

# License

kdtreepp is licensed under [MIT](https://opensource.org/licenses/MIT).

Made with [hpp-skel](https://github.com/mapbox/hpp-skel).
