# -*- python -*-
# This file contains rules for Bazel; see https://bazel.io/ .

# Headers to expose.
filegroup(
    name = "headers",
    srcs = [
        "dreal/symbolic/hash.h",
        "dreal/symbolic/symbolic_environment.h",
        "dreal/symbolic/symbolic_expression.h",
        "dreal/symbolic/symbolic_expression_visitor.h",
        "dreal/symbolic/symbolic_formula.h",
        "dreal/symbolic/symbolic_formula_visitor.h",
        "dreal/symbolic/symbolic_variable.h",
        "dreal/symbolic/symbolic_variables.h",
        "dreal/symbolic/test/symbolic_test_util.h",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "drake_symbolic",
    srcs = [
        "dreal/symbolic/never_destroyed.h",
        "dreal/symbolic/symbolic_environment.cc",
        "dreal/symbolic/symbolic_expression.cc",
        "dreal/symbolic/symbolic_expression_cell.cc",
        "dreal/symbolic/symbolic_expression_cell.h",
        "dreal/symbolic/symbolic_expression_visitor.cc",
        "dreal/symbolic/symbolic_formula.cc",
        "dreal/symbolic/symbolic_formula_cell.cc",
        "dreal/symbolic/symbolic_formula_cell.h",
        "dreal/symbolic/symbolic_formula_visitor.cc",
        "dreal/symbolic/symbolic_variable.cc",
        "dreal/symbolic/symbolic_variables.cc",
    ],
    hdrs = [
        "dreal/symbolic/hash.h",
        "dreal/symbolic/symbolic_environment.h",
        "dreal/symbolic/symbolic_expression.h",
        "dreal/symbolic/symbolic_expression_visitor.h",
        "dreal/symbolic/symbolic_formula.h",
        "dreal/symbolic/symbolic_formula_visitor.h",
        "dreal/symbolic/symbolic_variable.h",
        "dreal/symbolic/symbolic_variables.h",
        "dreal/symbolic/test/symbolic_test_util.h",
    ],
    visibility = ["//visibility:public"],
)
