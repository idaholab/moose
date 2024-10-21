//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Function.h"

using namespace Moose;

InputParameters
Function::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();

  // Functions should be executed on the fly
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.registerBase("Function");

  return params;
}

Function::Function(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(this, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this),
    Moose::FunctorBase<Real>(name())
{
}

Function::~Function() {}

Real
Function::value(Real /*t*/, const Point & /*p*/) const
{
  mooseError("value method not implemented");
  return 0.0;
}

ADReal
Function::value(const ADReal & t, const ADPoint & p) const
{
  const auto rt = MetaPhysicL::raw_value(t);
  const auto rp = MetaPhysicL::raw_value(p);
  const auto grad = gradient(rt, rp);
  ADReal ret = value(rt, rp);
  ret.derivatives() = grad(0) * p(0).derivatives()
#if LIBMESH_DIM > 1
                      + grad(1) * p(1).derivatives()
#endif
#if LIBMESH_DIM > 2
                      + grad(2) * p(2).derivatives()
#endif
                      + timeDerivative(rt, rp) * t.derivatives();
  return ret;
}

ChainedReal
Function::value(const ChainedReal & t) const
{
  static const Point p;
  return {value(t.value(), p), timeDerivative(t.value(), p) * t.derivatives()};
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("gradient method not implemented");
  return RealGradient(0, 0, 0);
}

Real
Function::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  mooseError("timeDerivative method not implemented");
  return 0;
}

Real
Function::timeIntegral(Real /*t1*/, Real /*t2*/, const Point & /*p*/) const
{
  mooseError("timeIntegral() not implemented.");
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/) const
{
  mooseError("vectorValue method not implemented");
  return RealVectorValue(0, 0, 0);
}

RealVectorValue
Function::curl(Real /*t*/, const Point & /*p*/) const
{
  mooseError("curl method not implemented");
  return RealVectorValue(0, 0, 0);
}

Real
Function::div(Real /*t*/, const Point & /*p*/) const
{
  mooseError("div method not implemented");
  return 0;
}

Real
Function::integral() const
{
  mooseError("Integral method not implemented for function ", name());
  return 0;
}

Real
Function::average() const
{
  mooseError("Average method not implemented for function ", name());
  return 0;
}

template <typename R>
typename Function::ValueType
Function::evaluateHelper(const R & r, const Moose::StateArg & state) const
{
  return value(_ti_feproblem.getTimeFromStateArg(state), r.getPoint());
}

typename Function::ValueType
Function::evaluate(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return evaluateHelper(elem_arg, state);
}

typename Function::ValueType
Function::evaluate(const FaceArg & face, const Moose::StateArg & state) const
{
  if (face.face_side && face.fi->neighborPtr() &&
      (face.fi->elem().subdomain_id() != face.fi->neighbor().subdomain_id()))
  {
    // Some users like to put discontinuities in their functions at subdomain changes in which case
    // in order to always get the proper discontinuous effect we should evaluate ever so slightly
    // off the face. Consider evaluation of: if(x < 0, -1, 1) if the face centroid is right at x ==
    // 0 for example. The user likely doesn't want you to return 1 if they've asked for a 0-
    // evaluation
    //
    // I can't quite tell but I think the tolerance for comparing coordinates (x, y, z, t) in
    // fparser is ~1e-9 so we need to use something larger than that. The comparison is absolute
    static constexpr Real offset_tolerance = 1e-8;
    auto offset = offset_tolerance * face.fi->normal();
    if (face.face_side == face.fi->elemPtr())
      offset *= -1;
    return value(_ti_feproblem.getTimeFromStateArg(state), face.getPoint() + offset);
  }
  else
    return value(_ti_feproblem.getTimeFromStateArg(state), face.getPoint());
}

typename Function::ValueType
Function::evaluate(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  return evaluateHelper(elem_qp, state);
}

typename Function::ValueType
Function::evaluate(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  return evaluateHelper(elem_side_qp, state);
}

typename Function::ValueType
Function::evaluate(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return evaluateHelper(elem_point_arg, state);
}

typename Function::ValueType
Function::evaluate(const NodeArg & node_arg, const Moose::StateArg & state) const
{
  return evaluateHelper(node_arg, state);
}

template <typename R>
typename Function::GradientType
Function::evaluateGradientHelper(const R & r, const Moose::StateArg & state) const
{
  return gradient(_ti_feproblem.getTimeFromStateArg(state), r.getPoint());
}

typename Function::GradientType
Function::evaluateGradient(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(elem_arg, state);
}

typename Function::GradientType
Function::evaluateGradient(const FaceArg & face, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(face, state);
}

typename Function::GradientType
Function::evaluateGradient(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(elem_qp, state);
}

typename Function::GradientType
Function::evaluateGradient(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(elem_side_qp, state);
}

typename Function::GradientType
Function::evaluateGradient(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(elem_point_arg, state);
}

typename Function::GradientType
Function::evaluateGradient(const NodeArg & node_arg, const Moose::StateArg & state) const
{
  return evaluateGradientHelper(node_arg, state);
}

template <typename R>
typename Function::DotType
Function::evaluateDotHelper(const R & r, const Moose::StateArg & state) const
{
  return timeDerivative(_ti_feproblem.getTimeFromStateArg(state), r.getPoint());
}

typename Function::DotType
Function::evaluateDot(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return evaluateDotHelper(elem_arg, state);
}

typename Function::DotType
Function::evaluateDot(const FaceArg & face, const Moose::StateArg & state) const
{
  return evaluateDotHelper(face, state);
}

typename Function::DotType
Function::evaluateDot(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  return evaluateDotHelper(elem_qp, state);
}

typename Function::DotType
Function::evaluateDot(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  return evaluateDotHelper(elem_side_qp, state);
}

typename Function::DotType
Function::evaluateDot(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return evaluateDotHelper(elem_point_arg, state);
}

typename Function::DotType
Function::evaluateDot(const NodeArg & node_arg, const Moose::StateArg & state) const
{
  return evaluateDotHelper(node_arg, state);
}

void
Function::timestepSetup()
{
  FunctorBase<Real>::timestepSetup();
}

void
Function::residualSetup()
{
  FunctorBase<Real>::residualSetup();
}

void
Function::jacobianSetup()
{
  FunctorBase<Real>::jacobianSetup();
}

void
Function::customSetup(const ExecFlagType & exec_type)
{
  FunctorBase<Real>::customSetup(exec_type);
}
