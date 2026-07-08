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

#include "FEProblemBase.h"

#include <cmath>

registerMooseObject("MooseApp", MFEMCoordinateTransformations);

InputParameters
MFEMCoordinateTransformations::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function object that declares MFEM coordinate-dependent coefficients.");
  params.addRequiredParam<MooseEnum>(
      "coord_type", MooseEnum("RZ"), "Coordinate system type. Currently only RZ is supported.");
  params.addParam<Real>(
      "inv_r_eps", 1e-12, "Regularization parameter used in inv_r = 1/sqrt(r^2 + eps^2).");
  return params;
}

MFEMCoordinateTransformations::MFEMCoordinateTransformations(const InputParameters & parameters)
  : Function(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(
        *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))),
    _coord_type(getParam<MooseEnum>("coord_type")),
    _inv_r_eps(getParam<Real>("inv_r_eps"))
{
  if (_coord_type == "RZ")
    declareRZCoefficients();
  else
    mooseError("MFEMCoordinateTransformations currently supports only coord_type = RZ.");
}

void
MFEMCoordinateTransformations::declareRZCoefficients()
{
  declareRZRadialCoefficient();
  declareRZInverseRadialCoefficient();
  declareRZTwoPiRCoefficient();
}

void
MFEMCoordinateTransformations::declareRZRadialCoefficient()
{
  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      name() + "_r",
      [](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        return std::sqrt(x * x + y * y);
      });
}

void
MFEMCoordinateTransformations::declareRZInverseRadialCoefficient()
{
  const mfem::real_t eps = static_cast<mfem::real_t>(_inv_r_eps);

  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      name() + "_inv_r",
      [eps](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        const mfem::real_t r = std::sqrt(x * x + y * y);
        return 1.0 / std::sqrt(r * r + eps * eps);
      });
}

void
MFEMCoordinateTransformations::declareRZTwoPiRCoefficient()
{
  constexpr mfem::real_t two_pi = 6.283185307179586476925286766559;

  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      name() + "_two_pi_r",
      [](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        const mfem::real_t r = std::sqrt(x * x + y * y);
        return two_pi * r;
      });
}

#endif
