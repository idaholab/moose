//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalDiffusivityTensorBase.h"
#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", PolycrystalDiffusivityTensorBase);

InputParameters
PolycrystalDiffusivityTensorBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Generates a diffusion tensor to distinguish between the bulk, grain "
                             "boundaries, and surfaces");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addCoupledVar("T", "Temperature variable in Kelvin");
  params.addRequiredParam<Real>("D0", "Diffusion prefactor for vacancies in m^2/s");
  params.addRequiredParam<Real>("Em", "Vacancy migration energy in eV");
  params.addRequiredCoupledVar("c", "Vacancy phase variable");
  params.addParam<std::string>(
      "diffusivity_name", "D", "Name for the diffusivity material property");
  params.addParam<Real>("surfindex", 1.0, "Surface diffusion index weight");
  params.addParam<Real>("gbindex", 1.0, "Grain boundary diffusion index weight");
  params.addParam<Real>("bulkindex", 1.0, "Bulk diffusion index weight");
  return params;
}

PolycrystalDiffusivityTensorBase::PolycrystalDiffusivityTensorBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _T(coupledValue("T")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_name(coupledName("c", 0)),
    _diffusivity_name(getParam<std::string>("diffusivity_name")),
    _D(declareProperty<RealTensorValue>(_diffusivity_name)),
    _dDdc(isCoupledConstant("_c_name")
              ? nullptr
              : &declarePropertyDerivative<RealTensorValue>(_diffusivity_name, _c_name)),
    _D0(getParam<Real>("D0")),
    _Em(getParam<Real>("Em")),
    _s_index(getParam<Real>("surfindex")),
    _gb_index(getParam<Real>("gbindex")),
    _b_index(getParam<Real>("bulkindex")),
    _kb(8.617343e-5), // Boltzmann constant in eV/K
    _op_num(coupledComponents("v"))
{
  if (_op_num == 0)
    mooseError("Model requires op_num > 0");

  _vals.resize(_op_num);
  _grad_vals.resize(_op_num);
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _grad_vals[i] = &coupledGradient("v", i);
  }
}

void
PolycrystalDiffusivityTensorBase::computeProperties()
{
  RealTensorValue I(1, 0, 0, 0, 1, 0, 0, 0, 1);

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real c = _c[_qp];
    Real mc = 1.0 - c;

    // Compute grain boundary diffusivity
    RealTensorValue Dgb(0.0);

    for (unsigned int i = 0; i < _op_num; ++i)
      for (unsigned int j = i + 1; j < _op_num; ++j)
      {
        RealGradient ngb = (*_grad_vals[i])[_qp] - (*_grad_vals[j])[_qp];
        if (ngb.norm() > 1.0e-10)
          ngb /= ngb.norm();
        else
          ngb = 0.0;

        RealTensorValue Tgb;
        for (unsigned int a = 0; a < 3; ++a)
          for (unsigned int b = a; b < 3; ++b)
          {
            Tgb(a, b) = I(a, b) - ngb(a) * ngb(b);
            Tgb(b, a) = I(b, a) - ngb(b) * ngb(a);
          }

        Dgb += (*_vals[i])[_qp] * (*_vals[j])[_qp] * Tgb;
        Dgb += (*_vals[j])[_qp] * (*_vals[i])[_qp] * Tgb;
      }

    // Compute surface diffusivity matrix
    RealGradient ns(0), dns(0);
    if (_grad_c[_qp].norm() > 1.0e-10)
      ns = _grad_c[_qp] / _grad_c[_qp].norm();

    RealTensorValue Ts;
    RealTensorValue dTs;
    for (unsigned int a = 0; a < 3; ++a)
      for (unsigned int b = 0; b < 3; ++b)
      {
        Ts(a, b) = I(a, b) - ns(a) * ns(b);
        dTs(a, b) = -2.0 * dns(a) * ns(b);
      }

    RealTensorValue Dsurf = c * c * mc * mc * Ts;
    RealTensorValue dDsurfdc = (2.0 * c * mc * mc - 2.0 * c * c * mc) * Ts + c * c * mc * mc * dTs;

    // Compute bulk properties
    _Dbulk = _D0 * std::exp(-_Em / _kb / _T[_qp]);
    const Real mult_bulk = 1.0;
    const Real dmult_bulk = 0.0;

    // Compute diffusion tensor
    _D[_qp] = _Dbulk * (_b_index * mult_bulk * I + _gb_index * Dgb + _s_index * Dsurf);
    if (_dDdc)
      (*_dDdc)[_qp] = _Dbulk * (_b_index * dmult_bulk * I + _s_index * dDsurfdc);
  }
}
