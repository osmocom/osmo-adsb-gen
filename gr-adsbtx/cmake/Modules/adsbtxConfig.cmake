INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ADSBTX adsbtx)

FIND_PATH(
    ADSBTX_INCLUDE_DIRS
    NAMES adsbtx/api.h
    HINTS $ENV{ADSBTX_DIR}/include
        ${PC_ADSBTX_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ADSBTX_LIBRARIES
    NAMES gnuradio-adsbtx
    HINTS $ENV{ADSBTX_DIR}/lib
        ${PC_ADSBTX_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ADSBTX DEFAULT_MSG ADSBTX_LIBRARIES ADSBTX_INCLUDE_DIRS)
MARK_AS_ADVANCED(ADSBTX_LIBRARIES ADSBTX_INCLUDE_DIRS)

