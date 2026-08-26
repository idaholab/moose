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

#include <cmath>

registerMooseObject("MooseApp", MFEMCoordinateTransformations);

InputParameters
MFEMCoordinateTransformations::validParams()
{
  InputParameters params = MooseParsedFunction::validParams();
  params.addClassDescription(
      "Function object that declares MFEM coordinate-dependent coefficients.");
  params.addRequiredParam<MooseEnum>(
      "coord_type", MooseEnum("RZ"), "Coordinate system type. Currently only RZ is supported.");
  params.addParam<Real>(
      "inv_r_eps", 1e-12, "Regularization parameter used in inv_r = 1/sqrt(r^2 + eps^2).");

  params.set<std::string>("expression") = "0";
  return params;
}

MFEMCoordinateTransformations::MFEMCoordinateTransformations(const InputParameters & parameters)
  : MooseParsedFunction(parameters),
    _mfem_problem(static_cast<MFEMProblem &>(_pfb_feproblem)),
    _coord_type(getParam<MooseEnum>("coord_type")),
    _inv_r_eps(getParam<Real>("inv_r_eps"))
{
  if (_coord_type != "RZ")
    mooseError("MFEMCoordinateTransformations currently supports only coord_type = RZ.");

  const std::string r_name = name() + "_r";
  const std::string inv_r_name = name() + "_inv_r";
  const std::string two_pi_r_name = name() + "_two_pi_r";

  const mfem::real_t eps = static_cast<mfem::real_t>(_inv_r_eps);
  constexpr mfem::real_t two_pi = 6.283185307179586476925286766559;

  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      r_name,
      [](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        return std::sqrt(x * x + y * y);
      });

  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      inv_r_name,
      [eps](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        const mfem::real_t r = std::sqrt(x * x + y * y);
        return 1.0 / std::sqrt(r * r + eps * eps);
      });

  _mfem_problem.getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      two_pi_r_name,
      [](const mfem::Vector & p, mfem::real_t /*t*/) -> mfem::real_t
      {
        const mfem::real_t x = p.Size() > 0 ? p[0] : 0.0;
        const mfem::real_t y = p.Size() > 1 ? p[1] : 0.0;
        const mfem::real_t r = std::sqrt(x * x + y * y);
        return two_pi * r;
      });
}

#endif
