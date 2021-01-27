#include <benchmark/benchmark.h>

#include <Eigen/StdVector>
#include <kdtreepp.hpp>
#include <random>

using Vector3 = Eigen::Vector3d;
using AlignedBox3 = Eigen::AlignedBox3d;

static void BM_Points3dConstruction(benchmark::State& state) {
  std::vector<Vector3, Eigen::aligned_allocator<Vector3>> points;
  std::mt19937_64 randGen{size_t(state.thread_index)};
  std::uniform_real_distribution<double> dist{-1000.0, 1000.0};

  // Make random points
  const auto count = size_t(state.range(0));
  points.resize(count);
  for (auto& point : points) {
    point << dist(randGen), dist(randGen), dist(randGen);
  }

  while (state.KeepRunning()) {
    const auto node = kdtreepp::MakeEigenKdTreeNode<double, 3>(
        points.begin(), points.end(), [](const Vector3& p) { return p; },
        [](const Vector3& p) { return p; });
    benchmark::DoNotOptimize(node);
    benchmark::ClobberMemory();
  }
}

static void BM_Points3dClosest(benchmark::State& state) {
  std::vector<Vector3, Eigen::aligned_allocator<Vector3>> points;
  std::mt19937_64 randGen{size_t(state.thread_index)};
  std::uniform_real_distribution<double> dist{-1000.0, 1000.0};

  // Make random points
  points.resize(size_t(state.range(0)));
  for (auto& point : points) {
    point << dist(randGen), dist(randGen), dist(randGen);
  }

  const auto node = kdtreepp::MakeEigenKdTreeNode<double, 3>(
      points.begin(), points.end(), [](const Vector3& p) { return p; },
      [](const Vector3& p) { return p; });

  const auto iterations = size_t(state.range(1));

  while (state.KeepRunning()) {
    for (size_t i = 0; i < iterations; i++) {
      const Vector3 checkPoint{dist(randGen), dist(randGen), dist(randGen)};

      double minDistSq = std::numeric_limits<double>::max();
      Vector3 closestPoint;
      node.visit(
          [&minDistSq, checkPoint](const AlignedBox3& bounds) {
            return bounds.squaredExteriorDistance(checkPoint) < minDistSq;
          },
          [&minDistSq, &closestPoint, checkPoint](const Vector3& point) {
            const double rSq = (point - checkPoint).squaredNorm();
            if (rSq < minDistSq) {
              minDistSq = rSq;
              closestPoint = point;
            }
          });
      benchmark::DoNotOptimize(closestPoint);
      benchmark::ClobberMemory();
    }
  }
}

int main(int argc, char* argv[]) {
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  benchmark::RegisterBenchmark("BM_Points3dConstruction", BM_Points3dConstruction)
      ->Arg(1)
      ->Arg(10)
      ->Arg(100)
      ->Arg(1000)
      ->Arg(10000)
      ->Arg(100000)
      ->Arg(1000000);
  benchmark::RegisterBenchmark("BM_Points3dClosest", BM_Points3dClosest)->Args({100000, 10000});
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();

  return 0;
}
