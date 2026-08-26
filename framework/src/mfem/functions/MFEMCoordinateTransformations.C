//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCoordinateTransformations.h"

registerMooseObject("MooseApp", MFEMCoordinateTransformations);

InputParameters
MFEMCoordinateTransformations::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function object that declares MFEM coordinate-dependent coefficients.");
  params.addRequiredParam<MooseEnum>(
      "coord_type", MooseEnum("RZ"), "Coordinate system type. Currently only RZ is supported.");
  params.addParam<mfem::real_t>(
      "inv_r_eps", 1e-12, "Regularization parameter used in inv_r = 1/sqrt(r^2 + eps^2).");
  return params;
}

MFEMCoordinateTransformations::MFEMCoordinateTransformations(const InputParameters & parameters)
  : Function(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(
        *parameters.getCheckedPointerParam<SubProblem *>("_subproblem"))),
    _coord_type(getParam<MooseEnum>("coord_type")),
    _inv_r_eps(getParam<mfem::real_t>("inv_r_eps"))
{
  if (_coord_type == "RZ")
    declareRZCoefficients();
  else
    mooseError("MFEMCoordinateTransformations currently supports only coord_type = RZ.");
}

void
MFEMCoordinateTransformations::declareRZCoefficients()
{
  Moose::MFEM::CoefficientManager & coeffs = _mfem_problem.getCoefficients();
  auto & r = coeffs.declareScalar<mfem::CylindricalRadialCoefficient>(name() + "_r");
  coeffs.declareScalar<mfem::CylindricalAzimuthalCoefficient>(name() + "_p");
  coeffs.declareScalar<mfem::CylindricalZCoefficient>(name() + "_z");
  coeffs.declareScalar<mfem::ProductCoefficient>(name() + "_two_pi_r", 8 * atan(1), r);
  coeffs.declareScalar<mfem::TransformedCoefficient>(name() + "_inv_r",
                                                     &r,
                                                     [eps = _inv_r_eps](mfem::real_t r)
                                                     { return 1. / std::sqrt(r * r + eps * eps); });
}

#endif
