//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFEBase.h"

// Hermite FEEvaluator specializations — NOT YET IMPLEMENTED.
//
// Hermite elements require C1 continuity and a more complex DOF layout
// (function values and derivatives at nodes). They are not supported by
// the native Kokkos FE path and will fall back to the libMesh evaluator.
//
// These stubs provide compile-time errors if a Hermite specialization is
// accidentally instantiated, making mis-use easier to diagnose.

namespace Moose::Kokkos
{

template <>
struct FEEvaluator<HermiteTag, Edge2Tag>
{
  static_assert(sizeof(HermiteTag) == 0,
    "FEEvaluator<HermiteTag, Edge2Tag>: Hermite elements are not supported "
    "by the native Kokkos FE path. Use the libMesh fallback path instead.");
};

template <>
struct FEEvaluator<HermiteTag, Tri3Tag>
{
  static_assert(sizeof(HermiteTag) == 0,
    "FEEvaluator<HermiteTag, Tri3Tag>: Hermite elements are not supported "
    "by the native Kokkos FE path. Use the libMesh fallback path instead.");
};

template <>
struct FEEvaluator<HermiteTag, Quad4Tag>
{
  static_assert(sizeof(HermiteTag) == 0,
    "FEEvaluator<HermiteTag, Quad4Tag>: Hermite elements are not supported "
    "by the native Kokkos FE path. Use the libMesh fallback path instead.");
};

template <>
struct FEEvaluator<HermiteTag, Tet4Tag>
{
  static_assert(sizeof(HermiteTag) == 0,
    "FEEvaluator<HermiteTag, Tet4Tag>: Hermite elements are not supported "
    "by the native Kokkos FE path. Use the libMesh fallback path instead.");
};

template <>
struct FEEvaluator<HermiteTag, Hex8Tag>
{
  static_assert(sizeof(HermiteTag) == 0,
    "FEEvaluator<HermiteTag, Hex8Tag>: Hermite elements are not supported "
    "by the native Kokkos FE path. Use the libMesh fallback path instead.");
};

} // namespace Moose::Kokkos
