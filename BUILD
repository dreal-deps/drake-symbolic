# -*- python -*-
# This file contains rules for Bazel; see https://bazel.io/ .

cc_library(
    name = "drake_symbolic",
    srcs = [
        "drake/common/constants.h",
        "drake/common/drake_assert.h",
        "drake/common/drake_assert_and_throw.cc",
        "drake/common/drake_assertion_error.h",
        "drake/common/drake_copyable.h",
        "drake/common/drake_throw.h",
        "drake/common/eigen_types.h",
        "drake/common/never_destroyed.h",
        "drake/common/symbolic_environment.cc",
        "drake/common/symbolic_expression.cc",
        "drake/common/symbolic_expression_cell.cc",
        "drake/common/symbolic_expression_cell.h",
        "drake/common/symbolic_expression_visitor.cc",
        "drake/common/symbolic_formula.cc",
        "drake/common/symbolic_formula_cell.cc",
        "drake/common/symbolic_formula_cell.h",
        "drake/common/symbolic_formula_visitor.cc",
        "drake/common/symbolic_variable.cc",
        "drake/common/symbolic_variables.cc",
    ],
    hdrs = [
        "drake/common/hash.h",
        "drake/common/symbolic_environment.h",
        "drake/common/symbolic_expression.h",
        "drake/common/symbolic_expression_visitor.h",
        "drake/common/symbolic_formula.h",
        "drake/common/symbolic_formula_visitor.h",
        "drake/common/symbolic_variable.h",
        "drake/common/symbolic_variables.h",
        "drake/common/test/symbolic_test_util.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@eigen",
    ],
)
