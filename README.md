# kdtreepp

[![CI](https://github.com/jhurliman/kdtreepp/actions/workflows/ci.yml/badge.svg)](https://github.com/jhurliman/kdtreepp/actions/workflows/ci.yml)

**Build spatial queries over your own data.** A header-only C++17 k-d tree with Eigen geometry, custom point and bounds accessors, and a visitor API for pruning the search space.

Use it for nearest-neighbor searches, region queries, or spatial filtering of objects that have a position or bounding box. You supply the data and query logic; the tree supplies a hierarchy of bounds that lets you skip irrelevant regions.

- **Keep your object type.** Accessor functions describe where each object is and which bounds contain it.
- **Control the query.** A bounds predicate prunes branches, and a visitor examines candidate items.
- **Integrate as a library.** CMake exports a target; Bzlmod declares Eigen transitively. Tests and benchmarks are optional.

The tree partitions the input range **in place** and retains iterators into it. Keep the underlying storage alive and its iterators valid for the tree's lifetime. If positions or bounds change, rebuild the tree before querying. Nodes allocate memory; the tree does not make a separate owning copy of your objects.

## Find the closest point

This complete example builds a 3D tree and uses squared distances to prune nodes farther away than the best point found so far:

```cpp
#include <kdtreepp.hpp>
#include <Eigen/StdVector>
#include <iostream>
#include <limits>
#include <vector>

int main() {
  using Point = Eigen::Vector3d;
  std::vector<Point, Eigen::aligned_allocator<Point>> points{
      Point{0, 0, 0}, Point{4, 0, 0}, Point{0, 3, 0}};

  const auto tree = kdtreepp::MakeEigenKdTreeNode<double, 3>(
      points.begin(), points.end(),
      [](const Point& p) { return p; },  // sort point
      [](const Point& p) { return p; }); // bounds contributor

  const Point query{3, 0, 0};
  double bestDistanceSq = std::numeric_limits<double>::infinity();
  Point closest = Point::Zero();
  bool found = false;

  tree.visit(
      [&](const Eigen::AlignedBox3d& bounds) {
        return bounds.squaredExteriorDistance(query) < bestDistanceSq;
      },
      [&](const Point& point) {
        const double distanceSq = (point - query).squaredNorm();
        if (distanceSq < bestDistanceSq) {
          bestDistanceSq = distanceSq;
          closest = point;
          found = true;
        }
      });

  if (found) std::cout << closest.transpose() << '\n'; // 4 0 0
}
```

For custom objects, return an Eigen point from the sort accessor and a point or aligned box from the bounds accessor. A region query follows the same pattern: reject nonintersecting node bounds, then test each visited item against the exact region. Leaf visitors receive candidates, so the item-level test remains your responsibility.

## Bazel / Bzlmod

Use Bzlmod to consume a local checkout in your monorepo. This example does not require a Bazel Central Registry entry:

```starlark
# MODULE.bazel
bazel_dep(name = "kdtreepp", version = "2.0.0", repo_name = "spatial")
local_path_override(module_name = "kdtreepp", path = "third_party/kdtreepp")
```

Add `@spatial//:kdtreepp` to your target's `deps`. Eigen is a transitive dependency, and the renamed repository is covered by the [independent consumer example](examples/bazel-consumer).

For a remote dependency, replace the local override with `git_override(module_name = "kdtreepp", remote = "https://github.com/jhurliman/kdtreepp.git", commit = "<reviewed full commit SHA>")`. Once BCR registration is accepted, the override can be removed.

Configure C++17 or newer in your monorepo toolchain—for GCC/Clang, `--cxxopt=-std=c++17`. The repository tests use Bazel 9.2. See [RELEASING.md](RELEASING.md) for archive verification and BCR preparation.

## CMake

Install Eigen's CMake package first. Then either use `add_subdirectory` and link `kdtreepp::kdtreepp`, or install the header package:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/prefix
cmake --install build
```

In the consuming project:

```cmake
find_package(kdtreepp 2 CONFIG REQUIRED)
target_link_libraries(my_application PRIVATE kdtreepp::kdtreepp)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix. The target propagates Eigen and the C++17 requirement. Installation is relocatable and architecture-independent; the library does not set global compiler flags. A complete [installed consumer](examples/cmake-consumer) is included.

### Conan 2

The recipe packages the headers and declares Eigen as a dependency:

```sh
conan profile detect
conan create . --build=missing -s compiler.cppstd=17
```

This creates and tests a local Conan package; it does not publish one. CMake does not invoke Conan automatically. Conan 1 is no longer supported.

## Tree construction and traversal

`MakeEigenKdTreeNode<T, N>(begin, end, sortPointGetter, boundsGetter, maxPerLeaf = 8, maxSubDivs = 16)` builds a tree from a mutable random-access range. `T` is the coordinate scalar type and `N` is the dimension.

| Member | Use |
| --- | --- |
| `bounds()` | Inspect a node's surrounding aligned box. |
| `isLeaf()` / `isBranch()` | Inspect the node type. |
| `visit(boundsTest, visitor)` | Visit items under nodes whose bounds pass your predicate. |

Use finite coordinates, a positive `maxPerLeaf`, and a nonnegative `maxSubDivs`. Smaller leaves can enable tighter pruning at the cost of more nodes; the best configuration depends on your data and queries. The library does not provide dynamic insertion/removal or a dedicated nearest-neighbor method—the example implements the search through traversal.

## Development

```sh
bazelisk test //:regression_test --cxxopt=-std=c++17
cmake -S . -B build -DKDTREEPP_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Regression tests compare nearest-neighbor results with brute force, including empty, singleton, duplicate and boundary-sized inputs. CI also checks sanitizers, installed CMake consumers, Conan, Make targets and a checksum-verified archive consumed through a temporary Bazel registry.

The standard regression suite also covers version reporting, spatial pruning, region queries, and in-place visitor mutation; no separate test framework is required. Enable `KDTREEPP_BUILD_BENCHMARKS` with Google Benchmark installed to run benchmarks. [RELEASING.md](RELEASING.md) documents Make, coverage and release commands; [CHANGELOG.md](CHANGELOG.md) describes the version 2 build migration. No general performance improvement is claimed by that migration.

## License

[MIT](LICENSE). Originally scaffolded with [hpp-skel](https://github.com/mapbox/hpp-skel).
