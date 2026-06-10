//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCylindricalCoordinateCoefficients.h"

registerMooseObject("MooseApp", MFEMCylindrical);

namespace
{

class InvShiftedCoefficient : public mfem::Coefficient
{
public:
  InvShiftedCoefficient(mfem::Coefficient & base, mfem::real_t eps) : _base(base), _eps(eps) {}

  virtual mfem::real_t Eval(mfem::ElementTransformation & T,
                            const mfem::IntegrationPoint & ip) override
  {
    const mfem::real_t r = _base.Eval(T, ip);
    return 1.0 / std::sqrt(r*r + _eps*_eps);
  }

private:
  mfem::Coefficient & _base;
  const mfem::real_t _eps;
};

}

InputParameters
MFEMCylindrical::validParams()
{
  InputParameters params = MFEMCoordinateCoefficients::validParams();
  params.addClassDescription(
      "Cylindrical coordinate coefficients with built-in MFEM coefficients.");
  return params;
}

MFEMCylindrical::MFEMCylindrical(const InputParameters & parameters)
  : MFEMCoordinateCoefficients(parameters)
{
}

void
MFEMCylindrical::declareRadialCoefficient(Moose::MFEM::CoefficientManager & coeffs)
{
  coeffs.declareScalar<mfem::CylindricalRadialCoefficient>(coefficientName("r"));
}

void
MFEMCylindrical::declareInverseRadialCoefficient(Moose::MFEM::CoefficientManager & coeffs)
{
  auto & r_coeff = coeffs.getScalarCoefficient(coefficientName("r"));
  coeffs.declareScalar<InvShiftedCoefficient>(
      coefficientName("inv_r"), r_coeff, static_cast<mfem::real_t>(_inv_r_eps));
}

void
MFEMCylindrical::declareTwoPiRCoefficient(Moose::MFEM::CoefficientManager & coeffs)
{
  auto & r_coeff = coeffs.getScalarCoefficient(coefficientName("r"));

  constexpr mfem::real_t two_pi = 6.283185307179586476925286766559;

  coeffs.declareScalar<mfem::TransformedCoefficient>(coefficientName("two_pi_r"),
                                                     &r_coeff,
                                                     [](mfem::real_t a) -> mfem::real_t
                                                     { return two_pi * a; });
}

void
MFEMCylindrical::declareCoefficients(Moose::MFEM::CoefficientManager & coeffs)
{
  this->declareRadialCoefficient(coeffs);
  this->declareInverseRadialCoefficient(coeffs);
  this->declareTwoPiRCoefficient(coeffs);
}

#endif
