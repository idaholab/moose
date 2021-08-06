//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseFunctionBase.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "ScalarCoupleable.h"
#include "MooseFunctor.h"

// libMesh
#include "libmesh/vector_value.h"

// libMesh forward declarations
namespace libMesh
{
class Point;
}

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
template <typename T>
class FunctionTempl : public MooseFunctionBase,
                      public TransientInterface,
                      public PostprocessorInterface,
                      public UserObjectInterface,
                      public Restartable,
                      public MeshChangedInterface,
                      public ScalarCoupleable,
                      public Moose::FunctorBase<T>
{
public:
  /**
   * Class constructor
   * \param parameters The input parameters for the function
   */
  static InputParameters validParams();

  FunctionTempl(const InputParameters & parameters);

  /**
   * Function destructor
   */
  virtual ~FunctionTempl();

  /**
   * Override this to evaluate the scalar function at point (t,x,y,z), by default
   * this returns zero, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A scalar of the function evaluated at the time and location
   */
  virtual Real value(Real t, const Point & p) const;

  /**
   * Override this to evaluate the vector function at a point (t,x,y,z), by default
   * this returns a zero vector, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A vector of the function evaluated at the time and location
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p) const;

  /**
   * Override this to evaluate the curl of the vector function at a point (t,x,y,z),
   * by default this returns a zero vector, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A vector of the curl of the function evaluated at the time and location
   */
  virtual RealVectorValue vectorCurl(Real t, const Point & p) const;

  using Moose::FunctorBase<T>::gradient;
  /**
   * Function objects can optionally provide a gradient at a point. By default
   * this returns 0, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A gradient of the function evaluated at the time and location
   */
  virtual RealGradient gradient(Real t, const Point & p) const;

  /**
   * Get the time derivative of the function
   * \param t The time
   * \param p The point in space (x,y,z)
   * \return The time derivative of the function at the specified time and location
   */
  virtual Real timeDerivative(Real t, const Point & p) const;

  // Not defined
  virtual Real integral() const;

  // Not defined
  virtual Real average() const;

  void timestepSetup() override;
  void residualSetup() override;
  void jacobianSetup() override;

private:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;

  /**
   * @return the time associated with the requested \p state
   */
  Real getTime(unsigned int state) const;

  using ElemArg = Moose::ElemArg;
  using ElemFromFaceArg = Moose::ElemFromFaceArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using SingleSidedFaceArg = Moose::SingleSidedFaceArg;

  ValueType evaluate(const ElemArg & elem, unsigned int state) const override final;
  ValueType evaluate(const ElemFromFaceArg & elem_from_face,
                     unsigned int state) const override final;
  ValueType evaluate(const FaceArg & face, unsigned int state) const override final;
  ValueType evaluate(const SingleSidedFaceArg & face, unsigned int state) const override final;
  ValueType evaluate(const ElemQpArg & qp, unsigned int state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const override final;

  GradientType evaluateGradient(const ElemArg & elem, unsigned int state) const override final;
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int state) const override final;
  GradientType evaluateGradient(const FaceArg & face, unsigned int state) const override final;
  GradientType evaluateGradient(const SingleSidedFaceArg & face,
                                unsigned int state) const override final;
  GradientType evaluateGradient(const ElemQpArg & qp, unsigned int state) const override final;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                unsigned int state) const override final;

  DotType evaluateDot(const ElemArg & elem, unsigned int state) const override final;
  DotType evaluateDot(const ElemFromFaceArg & elem_from_face,
                      unsigned int state) const override final;
  DotType evaluateDot(const FaceArg & face, unsigned int state) const override final;
  DotType evaluateDot(const SingleSidedFaceArg & face, unsigned int state) const override final;
  DotType evaluateDot(const ElemQpArg & qp, unsigned int state) const override final;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp, unsigned int state) const override final;

  /**
   * Compute \p _current_elem_qp_functor_xyz if we are on a new element
   */
  void determineElemXYZ(const ElemQpArg & elem_qp) const;

  /**
   * Compute \p _current_elem_side_qp_functor_xyz if we are on a new element and side pair
   */
  void determineElemSideXYZ(const ElemSideQpArg & elem_side_qp) const;

  /// Keep track of the current elem-qp functor element in order to enable local caching (e.g. if we
  /// call evaluate on the same element, but just with a different quadrature point, we can return
  /// previously computed results indexed at the different qp)
  mutable const Elem * _current_elem_qp_functor_elem = nullptr;

  /// The location of the quadrature points in physical space for the
  /// \p _current_elem_qp_functor_elem
  mutable std::vector<Point> _current_elem_qp_functor_xyz;

  /// Keep track of the current elem-side-qp functor element-side pair in order to enable local
  /// caching (e.g. if we call evaluate on the same element and side, but just with a different
  /// quadrature point, we can return previously computed results indexed at the different qp)
  mutable std::pair<const Elem *, unsigned int> _current_elem_side_qp_functor_elem_side{
      nullptr, libMesh::invalid_uint};

  /// The location of the quadrature points in physical space for the
  /// \p _current_elem_side_qp_functor_elem_side
  mutable std::vector<Point> _current_elem_side_qp_functor_xyz;
};

class Function : public FunctionTempl<Real>
{
public:
  static InputParameters validParams() { return FunctionTempl<Real>::validParams(); }
  Function(const InputParameters & params) : FunctionTempl<Real>(params) {}
};

template <>
InputParameters validParams<Function>();
