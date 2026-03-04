//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterpolationMethod.h"

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
FVInterpolationMethod::interpolate(const FaceInfo &, Real, Real) const
{
  mooseError("Interpolation method '",
             name(),
             "' (",
             type(),
             ") does not define face interpolation.");
}

FVInterpolationMethod::AdvectedSystemContribution
FVInterpolationMethod::advectedInterpolate(
    const FaceInfo &, Real, Real, const VectorValue<Real> *, const VectorValue<Real> *, Real) const
{
  mooseError("Interpolation method '",
             name(),
             "' (",
             type(),
             ") does not define advected interpolation.");
}

Real
FVInterpolationMethod::advectedInterpolateValue(const FaceInfo & face,
                                                const Real elem_value,
                                                const Real neighbor_value,
                                                const VectorValue<Real> * elem_grad,
                                                const VectorValue<Real> * neighbor_grad,
                                                const Real mass_flux) const
{
  const auto result = advectedInterpolate(
      face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
  const Real phi_matrix =
      result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
  return phi_matrix - result.rhs_face_value;
}
