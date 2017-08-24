# -*- python -*-
# This file contains rules for Bazel; see https://bazel.io/ .

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "eigen",
    hdrs = glob(
        include = [
            "Eigen/*",
            "Eigen/**/*.h",
            "unsupported/Eigen/*",
            "unsupported/Eigen/**/*.h",
        ],
        exclude = ["**/CMakeLists.txt"],
    ),
    defines = ["EIGEN_MPL2_ONLY"],
    includes = ["."],
)
