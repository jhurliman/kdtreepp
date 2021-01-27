# kdtreepp

### A C++ k-d tree implementation

[![Build Status](https://travis-ci.com/jhurliman/kdtreepp.svg?branch=master)](https://travis-ci.com/jhurliman/kdtreepp)
[![codecov](https://codecov.io/gh/jhurliman/kdtreepp/branch/master/graph/badge.svg)](https://codecov.io/gh/jhurliman/kdtreepp)

## Usage

```cpp
TODO
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
export LDFLAGS="-fsanitize=address,undefined,integer"
export CXXFLAGS="-fsanitize=address,undefined,integer"

make
```

# License

kdtreepp is licensed under [MIT](https://opensource.org/licenses/MIT).

[![badge](https://mapbox.s3.amazonaws.com/cpp-assets/hpp-skel-badge_blue.svg)](https://github.com/mapbox/hpp-skel)
