BUILD_DIR ?= build
BENCHMARK_FILTER ?= .
.PHONY: release debug test bench clean format coverage tidy
release:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DKDTREEPP_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR)
debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DKDTREEPP_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR)
test: release
	ctest --test-dir $(BUILD_DIR) --output-on-failure
bench:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DKDTREEPP_BUILD_BENCHMARKS=ON
	cmake --build $(BUILD_DIR) --target bench-tests
	$(BUILD_DIR)/bench-tests --benchmark_filter="$(BENCHMARK_FILTER)" --benchmark_min_time=0.01s
clean:
	cmake -E rm -rf $(BUILD_DIR)
format:
	./scripts/format.sh

coverage:
	BUILD_DIR="$(BUILD_DIR)-coverage" ./scripts/coverage.sh

tidy: debug
	BUILD_DIR="$(BUILD_DIR)" ./scripts/clang-tidy.sh
