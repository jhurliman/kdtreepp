# A header-only package must accept either consumer pointer size. This checks
# generated version selection; it does not pretend to compile a 32-bit binary.
foreach(pointer_size 4 8)
  set(CMAKE_SIZEOF_VOID_P ${pointer_size})
  set(PACKAGE_FIND_VERSION "2.0.0")
  set(PACKAGE_FIND_VERSION_MAJOR 2)
  unset(PACKAGE_VERSION_UNSUITABLE)
  unset(PACKAGE_VERSION_COMPATIBLE)
  include("${VERSION_FILE}")
  if(PACKAGE_VERSION_UNSUITABLE OR NOT PACKAGE_VERSION_COMPATIBLE)
    message(FATAL_ERROR "Header package rejected pointer size ${pointer_size}")
  endif()
endforeach()
