//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MooseTypes.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/quadrature.h"

#include <set>

namespace Moose
{
/**
 * A structure that is used to evaluate Moose functors logically at an element/cell center
 */
struct ElemArg
{
  const libMesh::Elem * elem;
  bool correct_skewness;

  /**
   * @returns The conceptual physical location of this data structure
   */
  Point getPoint() const { return elem->vertex_average(); }

  /**
   * friend function that allows this structure to be used as keys in ordered containers like sets
   * and maps
   */
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

  /**
   * @returns The conceptual physical location of this data structure
   */
  Point getPoint() const { return point; }

  /**
   * friend function that allows this structure to be used as keys in ordered containers like sets
   * and maps
   */
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
 * Argument for requesting functor evaluation at quadrature point locations on an element side.
 * Data in the argument:
 * - The element
 * - The element side on which the quadrature points are located
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the i-th point
 * - The quadrature rule that can be used to initialize the functor on the given element and side
 */
struct ElemSideQpArg
{
  /// The element
  const libMesh::Elem * elem;

  /// The local side index
  unsigned int side;

  /// The quadrature point index
  unsigned int qp;

  /// The quadrature rule
  const libMesh::QBase * qrule;

  /// The physical location of the quadrature point
  Point point;

  /**
   * @returns The conceptual physical location of this data structure
   */
  Point getPoint() const { return point; }
};

/**
 * State argument for evaluating functors. The iteration type indicates whether you want to evaluate
 * a functor based on some iterate state of a transient calculation, nonlinear solve, etc. The state
 * indicates which iterate of the iterate type we want to evaluate on. A state of 0 indicates
 * "current", e.g. the current time or the current nonlinear iteration (which should actually be
 * equivalent); a state of 1 indicates the most-recent "old" time or the most recent previous
 * nonlinear iteration, etc.
 */

struct StateArg
{
  /**
   * Prevent implicit conversions from boolean to avoid users accidentally constructing a time
   * argument when they meant to construct a skewness argument, etc.
   */
  StateArg(bool) = delete;

  StateArg(unsigned int state_in) : state(state_in), iteration_type(SolutionIterationType::Time) {}

  StateArg(unsigned int state_in, SolutionIterationType iteration_type_in)
    : state(state_in), iteration_type(iteration_type_in)
  {
  }

  /// The state. Zero represents the most recent state, so for any kind of iteration type, a zero
  /// state represents the current state, e.g. current solution
  /// One may represent the 'old' value (one before, in the iteration_type specified), and two an 'older' or two steps away state
  unsigned int state;

  /// The solution iteration type, e.g. time or nonlinear
  SolutionIterationType iteration_type;

private:
  StateArg() : state(0), iteration_type(SolutionIterationType::Time) {}

  friend StateArg currentState();
};

inline StateArg
currentState()
{
  return {};
}

inline StateArg
oldState()
{
  return {(unsigned int)1};
}

inline StateArg
previousNonlinearState()
{
  return {(unsigned int)1, SolutionIterationType::Nonlinear};
}

/**
 * A structure defining a "face" evaluation calling argument for Moose functors
 */
struct FaceArg
{
  /// a face information object which defines our location in space
  const FaceInfo * fi;

  /// a limiter which defines how the functor evaluated on either side of the face should be
  /// interpolated to the face
  Moose::FV::LimiterType limiter_type;

  /// a boolean which states whether the face information element is upwind of the face
  bool elem_is_upwind;

  /// Whether to perform skew correction
  bool correct_skewness;

  /// A member that can be used to indicate whether there is a sidedness to this face. For example,
  /// a block restricted diffusion kernel may use this to specify that a diffusion coefficient
  /// should be evaluated on the elem side of the face, ignoring possible jumps in the diffusion
  /// coefficient between the elem and neighbor sides of the face. If this is null, then a functor
  /// that is itself block restricted may modify the value to indicate \emph its sidedness. If there
  /// is ever a mismatch between the specified sidedness of a physics object and the sidedness of a
  /// functor, then we will error
  /// If unspecified (nullptr), the evaluation will be two-sided, unless the functor is not defined
  /// on one side of the face.
  const libMesh::Elem * face_side;

  /// A member that can be used to define the instance in which the limiters are executed
  const Moose::StateArg * state_limiter;

  /**
   * @returns The conceptual physical location of this data structure
   */
  libMesh::Point getPoint() const { return fi->faceCentroid(); }

  /**
   * Make a \p ElemArg from our data using the face information element
   */
  ElemArg makeElem() const { return {&fi->elem(), correct_skewness}; }

  /**
   * Make a \p ElemArg from our data using the face information neighbor
   */
  ElemArg makeNeighbor() const { return {fi->neighborPtr(), correct_skewness}; }

  /**
   * friend function that allows this structure to be used as keys in ordered containers like sets
   * and maps
   */
  friend bool operator<(const FaceArg & l, const FaceArg & r)
  {
    return std::make_tuple(
               l.fi, l.limiter_type, l.elem_is_upwind, l.correct_skewness, l.face_side) <
           std::make_tuple(r.fi, r.limiter_type, r.elem_is_upwind, r.correct_skewness, r.face_side);
  }
};

struct NodeArg
{
  /// The node which defines our location in space
  const libMesh::Node * node;

  /**
   * Indicates what subdomains this argument should be associated with. If a single subdomain is
   * given, then there is no ambiguity when this argument is used to evaluate functors at the
   * intersection of different blocks. If multiple subdomains are given at such an intersection
   * point, it is up to the functor whether it can be evaluated unambiguously, perform an average,
   * or error
   */
  const std::set<SubdomainID> * subdomain_ids;

  libMesh::Point getPoint() const { return *node; }

  /// A static member that can be used when the connection of a node to subdomains is unknown.
  /// Functors may still be able to evaluate a NodeArg with this as the provided \p subdomain_ids
  /// if the functor has no "sidedness", e.g. like a H1 finite element family variable
  static const std::set<SubdomainID> undefined_subdomain_connection;
};

/**
 * Argument for requesting functor evaluation at a quadrature point location in an element. Data
 * in the argument:
 * - The element containing the quadrature point
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the i-th point
 * - The quadrature rule that can be used to initialize the functor on the given element
 */
struct ElemQpArg
{
  /// The element
  const libMesh::Elem * elem;

  /// The quadrature point index
  unsigned int qp;

  /// The quadrature rule
  const libMesh::QBase * qrule;

  /// The physical location of the quadrature point
  libMesh::Point point;

  /**
   * @returns The conceptual physical location of this data structure
   */
  libMesh::Point getPoint() const { return point; }
};
}
