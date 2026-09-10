# kdtreepp

### A C++ k-d tree implementation

Requires C++17 and Eigen. The library is header-only; tests and benchmarks are optional and are not consumer dependencies.

## Bazel / Bzlmod

Until a release is registered in the Bazel Central Registry, use a checkout override:

```starlark
bazel_dep(name = "kdtreepp", version = "1.0.0")
local_path_override(module_name = "kdtreepp", path = "third_party/kdtreepp")
```

Link `@kdtreepp//:kdtreepp` from your `cc_library`, `cc_binary`, or `cc_test`. Eigen is declared transitively. Set C++17 or newer in your monorepo's toolchain/configuration (for example `--cxxopt=-std=c++17`); dependency `.bazelrc` files do not set consumer compiler options. Repository renaming through `repo_name` is supported.

For a remote checkout, replace the local override with a `git_override` using a reviewed, full commit SHA. This repository is not yet registered in BCR, so `bazel_dep` alone is not sufficient. No dependency downloads occur during C++ compilation.

## CMake

With Eigen installed, use `add_subdirectory` and link `kdtreepp::kdtreepp`, or install and consume the exported package:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/your/prefix
cmake --install build
```

```cmake
find_package(kdtreepp CONFIG REQUIRED)
target_link_libraries(my_application PRIVATE kdtreepp::kdtreepp)
```

CMake no longer invokes Conan automatically or changes global compiler flags. Existing Conan 1 recipes remain legacy and have not been validated with this integration; supply Eigen through a package manager or installed CMake package.

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

```sh
bazel test //:regression_test
cmake -S . -B build -DKDTREEPP_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The regression checks compare nearest-neighbor results with brute force over empty, singleton, boundary-sized and larger trees, and check duplicate points. Enable `KDTREEPP_BUILD_LEGACY_TESTS` with Catch2 2.x installed to run the original tests, or `KDTREEPP_BUILD_BENCHMARKS` with Google Benchmark installed.

For sanitizer checks, configure with `-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined` on a supporting compiler.

# License

kdtreepp is licensed under [MIT](https://opensource.org/licenses/MIT).

Made with [hpp-skel](https://github.com/mapbox/hpp-skel).
