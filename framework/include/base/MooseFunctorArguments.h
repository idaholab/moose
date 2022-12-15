//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <tuple>

#include "Limiter.h"
#include "FaceInfo.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/quadrature.h"

class FaceArgProducerInterface;
class FaceArgConsumerInterface;

namespace Moose
{
/**
 * A structure that is used to evaluate Moose functors logically at an element/cell center
 */
struct ElemArg
{
  const libMesh::Elem * elem;
  bool correct_skewness;

  friend bool operator<(const ElemArg & l, const ElemArg & r)
  {
    return std::make_tuple(l.elem, l.correct_skewness) <
           std::make_tuple(r.elem, r.correct_skewness);
  }
};

/**
 * A structure that is used to evaluate Moose functors at an arbitrary physical point contained
 * within an element
 */
struct ElemPointArg
{
  const libMesh::Elem * elem;
  libMesh::Point point;
  bool correct_skewness;

  friend bool operator<(const ElemPointArg & l, const ElemPointArg & r)
  {
    return std::make_tuple(l.elem, l.point, l.correct_skewness) <
           std::make_tuple(r.elem, r.point, r.correct_skewness);
  }

  /**
   * Make a \p ElemArg from our data
   */
  ElemArg makeElem() const { return {elem, correct_skewness}; }
};

/**
 * A structure defining a "face" evaluation calling argument for Moose functors
 */
class FaceArg
{
public:
  /// a face information object which defines our location in space
  const FaceInfo * fi;

  /// a limiter which defines how the functor evaluated on either side of the face should be
  /// interpolated to the face
  Moose::FV::LimiterType limiter_type;

  /// a boolean which states whether the face information element is upwind of the face
  bool elem_is_upwind;

  /// Whether to perform skew correction
  bool correct_skewness;

  /// A member that can be used to indicate whether there is a sidedness to this face. If null, then
  /// this means we're evaluating "right on" the face
  const Elem * face_side;

  /**
   * Make a \p ElemArg from our data using the face information element
   */
  ElemArg makeElem() const { return {&fi->elem(), correct_skewness}; }

  /**
   * Make a \p ElemArg from our data using the face information neighbor
   */
  ElemArg makeNeighbor() const { return {fi->neighborPtr(), correct_skewness}; }

  friend bool operator<(const FaceArg & l, const FaceArg & r)
  {
    return std::make_tuple(
               l.fi, l.limiter_type, l.elem_is_upwind, l.correct_skewness, l.face_side) <
           std::make_tuple(r.fi, r.limiter_type, r.elem_is_upwind, r.correct_skewness, r.face_side);
  }

private:
  FaceArg(const FaceInfo * const fi_in,
          const Moose::FV::LimiterType limiter_type_in,
          const bool elem_is_upwind_in,
          const bool correct_skewness_in,
          const Elem * const face_side_in)
    : fi(fi_in),
      limiter_type(limiter_type_in),
      elem_is_upwind(elem_is_upwind_in),
      correct_skewness(correct_skewness_in),
      face_side(face_side_in)
  {
  }

  friend class ::FaceArgProducerInterface;
  friend class ::FaceArgConsumerInterface;
};

/**
 * Argument for requesting functor evaluation at a quadrature point location in an element. Data
 * in the argument:
 * - The element containing the quadrature point
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the ith point
 * - The quadrature rule that can be used to initialize the functor on the given element
 */
using ElemQpArg = std::tuple<const libMesh::Elem *, unsigned int, const QBase *>;

/**
 * Argument for requesting functor evaluation at quadrature point locations on an element side.
 * Data in the argument:
 * - The element
 * - The element side on which the quadrature points are located
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the ith point
 * - The quadrature rule that can be used to initialize the functor on the given element and side
 */
using ElemSideQpArg = std::tuple<const libMesh::Elem *, unsigned int, unsigned int, const QBase *>;
}
