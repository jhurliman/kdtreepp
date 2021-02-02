from conans import ConanFile, CMake


class KdTreePpConan(ConanFile):
    name = "kdtreepp"
    version = "1.0.0"
    url = "https://github.com/jhurliman/kdtreepp"
    homepage = "https://github.com/jhurliman/kdtreepp"
    description = "A C++ k-d tree implementation"
    license = ("MIT")
    topics = ("data-structures", "spatial", "eigen")
    settings = "os", "compiler", "build_type", "arch"
    requires = "catch2/2.13.4", "benchmark/1.5.2"
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/jhurliman/kdtreepp.git")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="kdtreepp")
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=self._source_subfolder)
        cmake.install()

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "kdtreepp"
        self.cpp_info.names["pkg_config"] = "kdtreepp"
