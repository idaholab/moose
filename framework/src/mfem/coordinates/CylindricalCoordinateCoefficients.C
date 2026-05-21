//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "CylindricalCoordinateCoefficients.h"

registerMooseObject("MooseApp", Cylindrical);

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
    return 1.0 / (r + _eps);
  }

private:
  mfem::Coefficient & _base;
  const mfem::real_t _eps;
};

}

InputParameters
Cylindrical::validParams()
{
  InputParameters params = MFEMCoordinateCoefficients::validParams();
  params.addClassDescription(
      "Cylindrical coordinate coefficients with built-in MFEM coefficients.");
  return params;
}

Cylindrical::Cylindrical(const InputParameters & parameters)
  : MFEMCoordinateCoefficients(parameters)
{
}

void
Cylindrical::build()
{
  if (_r_coeff)
    return;

  _r_coeff = std::unique_ptr<mfem::Coefficient>(new mfem::CylindricalRadialCoefficient());

  _inv_r_coeff = std::unique_ptr<mfem::Coefficient>(
      new InvShiftedCoefficient(*_r_coeff, static_cast<mfem::real_t>(_inv_r_eps)));

  constexpr mfem::real_t two_pi = 6.283185307179586476925286766559;

  _two_pi_r_coeff = std::unique_ptr<mfem::Coefficient>(new mfem::TransformedCoefficient(
      _r_coeff.get(), [](mfem::real_t a) -> mfem::real_t { return two_pi * a; }));

  _measure_weight = std::unique_ptr<mfem::Coefficient>(new mfem::TransformedCoefficient(
      _r_coeff.get(), [](mfem::real_t a) -> mfem::real_t { return a; }));
}

const mfem::Coefficient *
Cylindrical::getBuiltinCoefficient(const std::string & name) const
{
  if (name == "r")
    return _r_coeff.get();
  if (name == "inv_r")
    return _inv_r_coeff.get();
  if (name == "two_pi_r")
    return _two_pi_r_coeff.get();
  if (name == "measure_weight")
    return _measure_weight.get();

  return nullptr;
}

#endif
