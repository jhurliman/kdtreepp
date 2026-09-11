# Changelog

## 2.0.0 — 2026-09-10

- Consolidate all regression coverage under `KDTREEPP_BUILD_TESTS`; remove the separate Catch2 2.x suite and unused Conan 1/Eigen find modules.
- Replace temporary release-branch wording in the README with installation guidance.


- Provide a Bzlmod library target with transitive Eigen and independent consumer checks.
- Export CMake package targets without automatically invoking Conan or setting global compiler flags.
- Replace the nonfunctional Conan 1 recipe with a tested Conan 2 header-only package and consumer.
- Restore Make test, benchmark and coverage commands; update Google Benchmark APIs and remove obsolete Travis configuration.
- Prepare checksum-based BCR metadata from the actual release archive and test consumption through a generated registry without local overrides.
- Keep version metadata synchronized across the C++ header, CMake, Conan and Bazel modules.

The C++ API is unchanged; the build and package-manager migration motivates the major version. Bazel Central Registry registration is tracked separately from the source release.
# 01/27/2021

* Initial release
