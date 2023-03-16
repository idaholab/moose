//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalDiffusivity.h"

registerMooseObject("PhaseFieldApp", PolycrystalDiffusivity);

InputParameters
PolycrystalDiffusivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Generates a diffusion coefficient to distinguish between the bulk, pore, grain "
      "boundaries, and surfaces");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addParam<Real>("Dbulk", 1.0, "Diffusion coefficient for volumetric diffusion in solid");
  params.addParam<Real>(
      "Dvoid", 1.0, "Diffusion coefficient for diffusion within void/pore/bubble ");
  params.addParam<Real>("Dsurf", 1.0, "Diffusion coefficient for surface diffusion");
  params.addParam<Real>("Dgb", 1.0, "Diffusion coefficient for grain boundary diffusion");
  params.addRequiredCoupledVar("c", "Vacancy phase variable");
  params.addParam<Real>("surf_weight", 1.0, "Surface diffusion weight");
  params.addParam<Real>("gb_weight", 1.0, "Grain boundary diffusion weight");
  params.addParam<Real>("bulk_weight", 1.0, "Bulk diffusion weight");
  params.addParam<Real>("void_weight", 1.0, "Void diffusion weight");
  params.addParam<MaterialPropertyName>(
      "void_switch", "hb", "Switching Function Materials for the void");
  params.addParam<MaterialPropertyName>(
      "solid_switch", "hm", "Switching Function Materials for the solid");
  params.addParam<MaterialPropertyName>("diffusivity", "D", "The name of the diffusion property");
  return params;
}

PolycrystalDiffusivity::PolycrystalDiffusivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _c(coupledValue("c")),
    _c_name(coupledName("c", 0)),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _diff_name(getParam<MaterialPropertyName>("diffusivity")),
    _diff(declareProperty<Real>(_diff_name)),
    _dDdc(isCoupledConstant(_c_name) ? nullptr
                                     : &declarePropertyDerivative<Real>(_diff_name, _c_name)),
    _hb(getMaterialProperty<Real>("void_switch")),
    _hm(getMaterialProperty<Real>("solid_switch")),
    _dhbdc(getMaterialPropertyDerivative<Real>("void_switch", _c_name)),
    _dhmdc(getMaterialPropertyDerivative<Real>("solid_switch", _c_name)),
    _dhbdv(_op_num),
    _dhmdv(_op_num),
    _diff_bulk(getParam<Real>("Dbulk")),
    _diff_void(getParam<Real>("Dvoid")),
    _diff_surf(getParam<Real>("Dsurf")),
    _diff_gb(getParam<Real>("Dgb")),
    _s_weight(getParam<Real>("surf_weight")),
    _gb_weight(getParam<Real>("gb_weight")),
    _b_weight(getParam<Real>("bulk_weight")),
    _v_weight(getParam<Real>("void_weight"))
{
  if (_op_num == 0)
    paramError("op_num", "Model requires a non zero number of order parameters.");

  _dDdv.resize(_op_num);
  for (MooseIndex(_op_num) op_index = 0; op_index < _op_num; ++op_index)
  {
    _vals[op_index] = &coupledValue("v", op_index);
    const VariableName op_name = coupledName("v", op_index);
    if (!isCoupledConstant("v"))
      _dDdv[op_index] = &declarePropertyDerivative<Real>(_diff_name, coupledName("v", op_index));
    _dhbdv[op_index] = &getMaterialPropertyDerivative<Real>("void_switch", op_index);
    _dhmdv[op_index] = &getMaterialPropertyDerivative<Real>("solid_switch", op_index);
  }
}

void
PolycrystalDiffusivity::computeQpProperties()
{
  Real SumEtaij = 0.0;
  Real SumEtaj = 0.0;
  for (const auto i : make_range(_op_num))
    for (const auto j : make_range(i + 1, _op_num))
    {
      SumEtaij += 18.0 * (*_vals[i])[_qp] * (*_vals[i])[_qp] * (*_vals[j])[_qp] * (*_vals[j])[_qp];
      SumEtaj += 18.0 * ((*_vals[i])[_qp] * (*_vals[j])[_qp] * (*_vals[j])[_qp] +
                         (*_vals[j])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp]);
    }
  Real c = _c[_qp];
  c = std::abs(c) > 1.0 ? 1.0 : (c < 0.0 ? std::abs(c) : c);
  const Real mc = 1.0 - c;

  // Compute diffusion function
  _diff[_qp] = _b_weight * _diff_bulk * _hm[_qp] + _v_weight * _diff_void * _hb[_qp] +
               30.0 * _diff_surf * _s_weight * c * c * mc * mc + _diff_gb * SumEtaij * _gb_weight;

  if (_dDdc)
    (*_dDdc)[_qp] = _b_weight * _diff_bulk * _dhmdc[_qp] + _v_weight * _diff_void * _dhbdc[_qp] +
                    30.0 * _diff_surf * _s_weight * (2.0 * c * mc * mc - 2.0 * c * c * mc);
  for (const auto op_index : make_range(_op_num))
    if (_dDdv[op_index])
      (*_dDdv[op_index])[_qp] = _b_weight * _diff_bulk * (*_dhmdv[op_index])[_qp] +
                                _v_weight * _diff_void * (*_dhbdv[op_index])[_qp] +
                                _diff_gb * SumEtaj * _gb_weight;
}
