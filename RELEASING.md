# Release 2.0.0

Version 2.0.0 is a major release because integration now uses Bzlmod, exported CMake targets and Conan 2 instead of automatically invoking Conan 1. The C++17 library API is unchanged.

## Verify before tagging

- `bazel test //:regression_test`, then run the renamed-repository consumer in `examples/bazel-consumer`.
- `make test`; `make bench` with Google Benchmark installed; `make coverage` with Clang and matching LLVM tools.
- `conan profile detect` if no profile exists, then `conan create . --build=missing -s compiler.cppstd=17`.
- `BAZEL=bazel python3 tools/test_bcr.py`. This archives the committed HEAD and tests an independent Bazel consumer through a temporary registry and checksum-verified archive, without a source override. Commit all intended release changes first.

CI performs these checks on the reviewed commit. Merging the stacked PRs and publishing/tagging are separate maintainer actions; test/pack commands do not publish anything.

## Bazel Central Registry submission

After the reviewed commit is merged and tagged `v2.0.0`, download the actual GitHub tag archive:

```sh
curl -fL https://github.com/jhurliman/kdtreepp/archive/refs/tags/v2.0.0.tar.gz -o kdtreepp-2.0.0.tar.gz
python3 tools/prepare_bcr.py kdtreepp-2.0.0.tar.gz --output /path/to/bazel-central-registry
```

The generator copies the archive's MODULE.bazel, computes its SHA-256 integrity, and prepares `modules/kdtreepp/metadata.json`, `2.0.0/source.json` and `presubmit.yml`. Use the downloaded release archive, not a locally recompressed copy: integrity must match the bytes served at the public URL. Review the files and run the BCR validation tools from that checkout before opening the registry PR, following [BCR contribution guidance](https://github.com/bazelbuild/bazel-central-registry/blob/main/docs/README.md).

There is no pre-existing registry entry in this release preparation. Until the BCR PR is merged, use the documented source override. After registration, a monorepo can use `bazel_dep(name = "kdtreepp", version = "2.0.0")` and depend on `@kdtreepp//:kdtreepp`.

## Conan 2

The recipe packages headers and the license without cloning another revision or downloading build-time test tools. Eigen is a declared transitive header dependency, and `test_package` compiles/runs a real consumer. ConanCenter submission or upload to a remote is an explicit release action; `conan create` only populates the local cache.
