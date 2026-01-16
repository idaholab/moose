//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterpolationMethod.h"

#include "MooseFunctor.h"
#include "MooseFunctorArguments.h"
#include "MooseLinearVariableFV.h"
#include "GradientLimiterType.h"

InputParameters
FVInterpolationMethod::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("FVInterpolationMethod");
  params.registerSystemAttributeName("FVInterpolationMethod");
  params.addClassDescription(
      "Base class for defining face interpolation schemes used by finite volume objects.");
  return params;
}

FVInterpolationMethod::FVInterpolationMethod(const InputParameters & params) : MooseObject(params)
{
}

Real
FVInterpolationMethod::FaceInterpolator::operator()(const Moose::FunctorBase<Real> & functor,
                                                    const FaceInfo & face,
                                                    const Moose::StateArg & state) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  const Moose::ElemArg elem_arg{&face.elem(), false};
  const Moose::ElemArg neighbor_arg{&face.neighbor(), false};
  const Real elem_value = functor(elem_arg, state);
  const Real neighbor_value = functor(neighbor_arg, state);

  return (*this)(face, elem_value, neighbor_value);
}

Real
FVInterpolationMethod::FaceInterpolator::operator()(const MooseLinearVariableFV<Real> & var,
                                                    const FaceInfo & face,
                                                    const Moose::StateArg & state) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  const Real elem_value = var.getElemValue(*face.elemInfo(), state);
  const Real neighbor_value = var.getElemValue(*face.neighborInfo(), state);

  return (*this)(face, elem_value, neighbor_value);
}

Real
FVInterpolationMethod::AdvectedValueInterpolator::operator()(
    const Moose::FunctorBase<Real> & functor,
    const FaceInfo & face,
    const Moose::StateArg & state,
    const Real mass_flux) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  mooseAssert(!needsLimitedGradients(),
              "Limited gradient reconstruction is not available for generic functors.");
  const Moose::ElemArg elem_arg{&face.elem(), false};
  const Moose::ElemArg neighbor_arg{&face.neighbor(), false};

  const Real elem_value = functor(elem_arg, state);
  const Real neighbor_value = functor(neighbor_arg, state);

  VectorValue<Real> elem_grad_storage;
  VectorValue<Real> neighbor_grad_storage;
  const VectorValue<Real> * elem_grad = nullptr;
  const VectorValue<Real> * neighbor_grad = nullptr;

  if (needsGradients())
  {
    elem_grad_storage = functor.gradient(elem_arg, state);
    elem_grad = &elem_grad_storage;
    neighbor_grad_storage = functor.gradient(neighbor_arg, state);
    neighbor_grad = &neighbor_grad_storage;
  }

  return (*this)(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

Real
FVInterpolationMethod::AdvectedValueInterpolator::operator()(
    const MooseLinearVariableFV<Real> & var,
    const FaceInfo & face,
    const Moose::StateArg & state,
    const Real mass_flux) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  const Real elem_value = var.getElemValue(*face.elemInfo(), state);
  const Real neighbor_value = var.getElemValue(*face.neighborInfo(), state);

  VectorValue<Real> elem_grad_storage;
  VectorValue<Real> neighbor_grad_storage;
  const VectorValue<Real> * elem_grad = nullptr;
  const VectorValue<Real> * neighbor_grad = nullptr;

  if (needsGradients())
  {
    const auto limiter_type = gradientLimiter();

    elem_grad_storage = var.gradSln(*face.elemInfo(), limiter_type);
    elem_grad = &elem_grad_storage;
    neighbor_grad_storage = var.gradSln(*face.neighborInfo(), limiter_type);
    neighbor_grad = &neighbor_grad_storage;
  }

  return (*this)(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

FVInterpolationMethod::AdvectedSystemContribution
FVInterpolationMethod::AdvectedSystemContributionCalculator::operator()(
    const Moose::FunctorBase<Real> & functor,
    const FaceInfo & face,
    const Moose::StateArg & state,
    const Real mass_flux) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  mooseAssert(!needsLimitedGradients(),
              "Limited gradient reconstruction is not available for generic functors.");
  const Moose::ElemArg elem_arg{&face.elem(), false};
  const Moose::ElemArg neighbor_arg{&face.neighbor(), false};

  const Real elem_value = functor(elem_arg, state);
  const Real neighbor_value = functor(neighbor_arg, state);

  VectorValue<Real> elem_grad_storage;
  VectorValue<Real> neighbor_grad_storage;
  const VectorValue<Real> * elem_grad = nullptr;
  const VectorValue<Real> * neighbor_grad = nullptr;

  if (needsGradients())
  {
    elem_grad_storage = functor.gradient(elem_arg, state);
    elem_grad = &elem_grad_storage;
    neighbor_grad_storage = functor.gradient(neighbor_arg, state);
    neighbor_grad = &neighbor_grad_storage;
  }

  return (*this)(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

FVInterpolationMethod::AdvectedSystemContribution
FVInterpolationMethod::AdvectedSystemContributionCalculator::operator()(
    const MooseLinearVariableFV<Real> & var,
    const FaceInfo & face,
    const Moose::StateArg & state,
    const Real mass_flux) const
{
  mooseAssert(face.neighborPtr(),
              "This convenience overload assumes internal faces (neighbor exists)");
  const Real elem_value = var.getElemValue(*face.elemInfo(), state);
  const Real neighbor_value = var.getElemValue(*face.neighborInfo(), state);

  VectorValue<Real> elem_grad_storage;
  VectorValue<Real> neighbor_grad_storage;
  const VectorValue<Real> * elem_grad = nullptr;
  const VectorValue<Real> * neighbor_grad = nullptr;

  if (needsGradients())
  {
    const auto limiter_type = gradientLimiter();

    elem_grad_storage = var.gradSln(*face.elemInfo(), limiter_type);
    elem_grad = &elem_grad_storage;
    neighbor_grad_storage = var.gradSln(*face.neighborInfo(), limiter_type);
    neighbor_grad = &neighbor_grad_storage;
  }

  return (*this)(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}
