//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeDiffuseInterfaceGeometryFunctorMaterial.h"

#include "MooseFunctorArguments.h"
#include "MathFVUtils.h"

#include <cmath>

registerMooseObject("NavierStokesApp", ConservativeDiffuseInterfaceGeometryFunctorMaterial);

InputParameters
ConservativeDiffuseInterfaceGeometryFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addClassDescription(
      "Create the face-oriented interface unit normal of a diffuse interface from the gradient of "
      "its volume fraction.");

  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction_functor",
      "The phase / volume-fraction functor used to construct interface gradients and normals.");
  params.addParam<Real>(
      "delta_n", 1e-8, "Regularization added to |grad(alpha)| when constructing unit normals.");
  params.addParam<MooseFunctorName>("interface_unit_normal_name",
                                    "interface_unit_normal_face",
                                    "Output name for the face-oriented unit normal functor.");
  return params;
}

ConservativeDiffuseInterfaceGeometryFunctorMaterial::
    ConservativeDiffuseInterfaceGeometryFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _volume_fraction(getFunctor<Real>("volume_fraction_functor")),
    _delta_n(getParam<Real>("delta_n"))
{
  if (_delta_n <= 0)
    paramError("delta_n", "delta_n must be positive.");

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  addFunctorProperty<RealVectorValue>(
      getParam<MooseFunctorName>("interface_unit_normal_name"),
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        const RealVectorValue grad_alpha = alphaGradient(r, t);
        const Real mag_grad_alpha = grad_alpha.norm();

        return grad_alpha / (mag_grad_alpha + _delta_n);
      },
      clearance_schedule);
}

template <typename SpaceArg>
RealVectorValue
ConservativeDiffuseInterfaceGeometryFunctorMaterial::alphaGradient(const SpaceArg & r,
                                                                   const Moose::StateArg & t) const
{
  return MetaPhysicL::raw_value(_volume_fraction.gradient(r, t));
}

RealVectorValue
ConservativeDiffuseInterfaceGeometryFunctorMaterial::alphaGradient(const Moose::FaceArg & r,
                                                                   const Moose::StateArg & t) const
{
  const FaceInfo * const fi = r.fi;
  if (!fi || !fi->neighborPtr() || !_volume_fraction.hasBlocks(fi->elemSubdomainID()) ||
      !_volume_fraction.hasBlocks(fi->neighborSubdomainID()))
    return MetaPhysicL::raw_value(_volume_fraction.gradient(r, t));

  const RealVectorValue elem_gradient =
      MetaPhysicL::raw_value(_volume_fraction.gradient(r.makeElem(), t));
  const RealVectorValue neighbor_gradient =
      MetaPhysicL::raw_value(_volume_fraction.gradient(r.makeNeighbor(), t));

  RealVectorValue face_gradient = 0.0;
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, face_gradient, elem_gradient, neighbor_gradient, *fi, true);
  return face_gradient;
}
