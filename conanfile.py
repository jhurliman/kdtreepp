import os
from conan import ConanFile
from conan.tools.files import copy

class KdTreePpConan(ConanFile):
    name = "kdtreepp"
    version = "2.0.0"
    package_type = "header-library"
    license = "MIT"
    url = "https://github.com/jhurliman/kdtreepp"
    homepage = url
    description = "A header-only C++17 k-d tree for Eigen"
    exports_sources = "include/*", "LICENSE"
    no_copy_source = True

    def requirements(self):
        self.requires("eigen/3.4.0", transitive_headers=True)

    def package(self):
        copy(self, "*.hpp", src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "include"))
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "kdtreepp")
        self.cpp_info.set_property("cmake_target_name", "kdtreepp::kdtreepp")
