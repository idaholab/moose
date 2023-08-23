//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "ScalarCoupleable.h"
#include "MooseFunctor.h"
#include "MooseADWrapper.h"
#include "ChainedReal.h"

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
class Function : public MooseObject,
                 public SetupInterface,
                 public TransientInterface,
                 public PostprocessorInterface,
                 public UserObjectInterface,
                 public Restartable,
                 public MeshChangedInterface,
                 public ScalarCoupleable,
                 public Moose::FunctorBase<Real>
{
public:
  /**
   * Class constructor
   * \param parameters The input parameters for the function
   */
  static InputParameters validParams();

  Function(const InputParameters & parameters);

  /**
   * Function destructor
   */
  virtual ~Function();

  /**
   * Override this to evaluate the scalar function at point (t,x,y,z), by default
   * this returns zero, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A scalar of the function evaluated at the time and location
   */
  virtual Real value(Real t, const Point & p) const;

  /**
   * Override this to evaluate the scalar function at point (t,x,y,z), using dual numbers by default
   * this uses value, gradient, and timeDerivative to assemble a dual number, although the function
   * can be overridden with a custom computation using dual numbers.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A scalar of the function evaluated at the time and location
   */
  virtual ADReal value(const ADReal & t, const ADPoint & p) const;

  ///@{ Helpers to call value(t,x,y,z)
  ChainedReal value(const ChainedReal & t) const;
  template <typename U>
  auto value(const U & t) const;
  template <typename U>
  auto value(const U & t, const U & x, const U & y = 0, const U & z = 0) const;
  ///@}

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

  using Moose::FunctorBase<Real>::gradient;
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

  ///@{ Helpers to call timeDerivative(t,x,y,z)
  template <typename U>
  auto timeDerivative(const U & t) const;
  template <typename U>
  auto timeDerivative(const U & t, const U & x, const U & y = 0, const U & z = 0) const;
  ///@}

  // Not defined
  virtual Real integral() const;

  // Not defined
  virtual Real average() const;

  void timestepSetup() override;
  void residualSetup() override;
  void jacobianSetup() override;
  void customSetup(const ExecFlagType & exec_type) override;

  bool hasBlocks(SubdomainID) const override { return true; }

private:
  using typename Moose::FunctorBase<Real>::ValueType;
  using typename Moose::FunctorBase<Real>::GradientType;
  using typename Moose::FunctorBase<Real>::DotType;

  using ElemArg = Moose::ElemArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using ElemPointArg = Moose::ElemPointArg;

  template <typename R>
  ValueType evaluateHelper(const R & r, const Moose::StateArg & state) const;

  ValueType evaluate(const ElemArg & elem, const Moose::StateArg & state) const override final;
  ValueType evaluate(const FaceArg & face, const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemPointArg & elem_point,
                     const Moose::StateArg & state) const override final;

  template <typename R>
  GradientType evaluateGradientHelper(const R & r, const Moose::StateArg & state) const;

  GradientType evaluateGradient(const ElemArg & elem,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const FaceArg & face,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemQpArg & qp,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemPointArg & elem_point,
                                const Moose::StateArg & state) const override final;

  template <typename R>
  DotType evaluateDotHelper(const R & r, const Moose::StateArg & state) const;
  DotType evaluateDot(const ElemArg & elem, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const FaceArg & face, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp,
                      const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemPointArg & elem_point,
                      const Moose::StateArg & state) const override final;
};

template <typename U>
auto
Function::value(const U & t) const
{
  static const MooseADWrapper<Point, MooseIsADType<U>::value> p;
  return value(t, p);
}

template <typename U>
auto
Function::value(const U & t, const U & x, const U & y, const U & z) const
{
  MooseADWrapper<Point, MooseIsADType<U>::value> p(x, y, z);
  return value(t, p);
}

template <typename U>
auto
Function::timeDerivative(const U & t) const
{
  static const MooseADWrapper<Point, MooseIsADType<U>::value> p;
  return timeDerivative(t, p);
}

template <typename U>
auto
Function::timeDerivative(const U & t, const U & x, const U & y, const U & z) const
{
  MooseADWrapper<Point, MooseIsADType<U>::value> p(x, y, z);
  return timeDerivative(t, p);
}
