//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

/**
 * Minimal preprocessor utilities for headers compiled by Kokkos backends.
 *
 * These macros provide token concatenation, mapping an operation over a variadic argument list,
 * and producing a comma-separated mapped list. Keeping the implementation local avoids requiring
 * an additional preprocessor library in every host and accelerator compilation scope.
 */

// Use two expansion stages so macro arguments are expanded before they are joined with ##.
#define MOOSE_KOKKOS_PP_CAT_IMPL(left, right) left##right
#define MOOSE_KOKKOS_PP_CAT(left, right) MOOSE_KOKKOS_PP_CAT_IMPL(left, right)

/**
 * Force repeated rescanning of recursively generated macro output.
 *
 * The preprocessor normally stops expanding a macro while that macro is already active. The map
 * implementation below therefore emits deferred calls that require additional rescans. Each EVAL
 * level expands its argument three times through the preceding level; EVAL5 provides enough
 * expansion depth for practical variadic lists without imposing a manually enumerated item count.
 */
#define MOOSE_KOKKOS_PP_EVAL0(...) __VA_ARGS__
#define MOOSE_KOKKOS_PP_EVAL1(...)                                                                 \
  MOOSE_KOKKOS_PP_EVAL0(MOOSE_KOKKOS_PP_EVAL0(MOOSE_KOKKOS_PP_EVAL0(__VA_ARGS__)))
#define MOOSE_KOKKOS_PP_EVAL2(...)                                                                 \
  MOOSE_KOKKOS_PP_EVAL1(MOOSE_KOKKOS_PP_EVAL1(MOOSE_KOKKOS_PP_EVAL1(__VA_ARGS__)))
#define MOOSE_KOKKOS_PP_EVAL3(...)                                                                 \
  MOOSE_KOKKOS_PP_EVAL2(MOOSE_KOKKOS_PP_EVAL2(MOOSE_KOKKOS_PP_EVAL2(__VA_ARGS__)))
#define MOOSE_KOKKOS_PP_EVAL4(...)                                                                 \
  MOOSE_KOKKOS_PP_EVAL3(MOOSE_KOKKOS_PP_EVAL3(MOOSE_KOKKOS_PP_EVAL3(__VA_ARGS__)))
#define MOOSE_KOKKOS_PP_EVAL5(...)                                                                 \
  MOOSE_KOKKOS_PP_EVAL4(MOOSE_KOKKOS_PP_EVAL4(MOOSE_KOKKOS_PP_EVAL4(__VA_ARGS__)))
#define MOOSE_KOKKOS_PP_EVAL(...) MOOSE_KOKKOS_PP_EVAL5(__VA_ARGS__)

/**
 * Detect the parenthesized sentinel appended to a mapped argument list.
 *
 * MOOSE_KOKKOS_PP_MAP() appends several ()()() sentinels after the user arguments. MAP_GET_END
 * turns a sentinel-shaped peek argument into MAP_END; otherwise MAP_NEXT selects the requested
 * continuation macro. MAP_OUT delays that continuation by one rescan so recursive expansion can
 * proceed under MOOSE_KOKKOS_PP_EVAL().
 */
#define MOOSE_KOKKOS_PP_MAP_END(...)
#define MOOSE_KOKKOS_PP_MAP_OUT
#define MOOSE_KOKKOS_PP_MAP_GET_END2() 0, MOOSE_KOKKOS_PP_MAP_END
#define MOOSE_KOKKOS_PP_MAP_GET_END1(...) MOOSE_KOKKOS_PP_MAP_GET_END2
#define MOOSE_KOKKOS_PP_MAP_GET_END(...) MOOSE_KOKKOS_PP_MAP_GET_END1
#define MOOSE_KOKKOS_PP_MAP_NEXT0(test, next, ...) next MOOSE_KOKKOS_PP_MAP_OUT
#define MOOSE_KOKKOS_PP_MAP_NEXT1(test, next) MOOSE_KOKKOS_PP_MAP_NEXT0(test, next, 0)
#define MOOSE_KOKKOS_PP_MAP_NEXT(test, next)                                                       \
  MOOSE_KOKKOS_PP_MAP_NEXT1(MOOSE_KOKKOS_PP_MAP_GET_END test, next)

/**
 * Apply function(data, value) to every variadic value without inserting separators.
 *
 * MAP0 and MAP1 alternate because a macro cannot directly expand itself while it is active. Each
 * step consumes the current value, peeks at the next value to detect the sentinel, and transfers
 * expansion to the other macro. The public MAP macro appends the sentinels and repeatedly rescans
 * the resulting expansion.
 */
#define MOOSE_KOKKOS_PP_MAP0(function, data, value, peek, ...)                                     \
  function(data, value)                                                                            \
      MOOSE_KOKKOS_PP_MAP_NEXT(peek, MOOSE_KOKKOS_PP_MAP1)(function, data, peek, __VA_ARGS__)
#define MOOSE_KOKKOS_PP_MAP1(function, data, value, peek, ...)                                     \
  function(data, value)                                                                            \
      MOOSE_KOKKOS_PP_MAP_NEXT(peek, MOOSE_KOKKOS_PP_MAP0)(function, data, peek, __VA_ARGS__)
#define MOOSE_KOKKOS_PP_MAP(function, data, ...)                                                   \
  MOOSE_KOKKOS_PP_EVAL(MOOSE_KOKKOS_PP_MAP1(function, data, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Apply function(data, value) to every variadic value and separate the results with commas.
 *
 * The first expansion is emitted without a leading comma by MAP_LIST0. Subsequent expansions
 * alternate between MAP_LIST1 and MAP_LIST2 because a macro cannot directly expand itself while
 * it is active; both continuation macros prefix a comma. This form is suitable when transformed
 * values must become function arguments, initializer entries, template arguments, or another
 * comma-separated sequence.
 */
#define MOOSE_KOKKOS_PP_MAP_LIST0(function, data, value, peek, ...)                                \
  function(data, value)                                                                            \
      MOOSE_KOKKOS_PP_MAP_NEXT(peek, MOOSE_KOKKOS_PP_MAP_LIST1)(function, data, peek, __VA_ARGS__)
#define MOOSE_KOKKOS_PP_MAP_LIST1(function, data, value, peek, ...)                                \
  , function(data, value) MOOSE_KOKKOS_PP_MAP_NEXT(peek, MOOSE_KOKKOS_PP_MAP_LIST2)(               \
        function, data, peek, __VA_ARGS__)
#define MOOSE_KOKKOS_PP_MAP_LIST2(function, data, value, peek, ...)                                \
  , function(data, value) MOOSE_KOKKOS_PP_MAP_NEXT(peek, MOOSE_KOKKOS_PP_MAP_LIST1)(               \
        function, data, peek, __VA_ARGS__)
#define MOOSE_KOKKOS_PP_MAP_LIST(function, data, ...)                                              \
  MOOSE_KOKKOS_PP_EVAL(                                                                            \
      MOOSE_KOKKOS_PP_MAP_LIST0(function, data, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
