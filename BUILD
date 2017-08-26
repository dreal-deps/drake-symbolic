# -*- python -*-
# This file contains rules for Bazel; see https://bazel.io/ .

cc_library(
    name = "drake_symbolic",
    srcs = [
        "symbolic/never_destroyed.h",
        "symbolic/symbolic_environment.cc",
        "symbolic/symbolic_expression.cc",
        "symbolic/symbolic_expression_cell.cc",
        "symbolic/symbolic_expression_cell.h",
        "symbolic/symbolic_expression_visitor.cc",
        "symbolic/symbolic_formula.cc",
        "symbolic/symbolic_formula_cell.cc",
        "symbolic/symbolic_formula_cell.h",
        "symbolic/symbolic_formula_visitor.cc",
        "symbolic/symbolic_variable.cc",
        "symbolic/symbolic_variables.cc",
    ],
    hdrs = [
        "symbolic/hash.h",
        "symbolic/symbolic_environment.h",
        "symbolic/symbolic_expression.h",
        "symbolic/symbolic_expression_visitor.h",
        "symbolic/symbolic_formula.h",
        "symbolic/symbolic_formula_visitor.h",
        "symbolic/symbolic_variable.h",
        "symbolic/symbolic_variables.h",
        "symbolic/test/symbolic_test_util.h",
    ],
    visibility = ["//visibility:public"],
)
