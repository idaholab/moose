/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#include "PolycrystalDiffusivity.h"
#include "libmesh/quadrature.h"

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
  params.addParam<Real>("surfindex", 1.0, "Surface diffusion index weight");
  params.addParam<Real>("gbindex", 1.0, "Grain boundary diffusion index weight");
  params.addParam<Real>("bulkindex", 1.0, "Bulk diffusion index weight");
  params.addParam<Real>("voidindex", 1.0, "Void diffusion index weight");
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
    _c_name(getVar("c", 0)->name()),
    _diff_name(getParam<MaterialPropertyName>("diffusivity")),
    _diff(declareProperty<Real>(_diff_name)),
    _dDdc(declarePropertyDerivative<Real>(_diff_name, _c_name)),
    _hb(getMaterialProperty<Real>("void_switch")),
    _hm(getMaterialProperty<Real>("solid_switch")),
    _dhbdc(getMaterialPropertyDerivative<Real>("void_switch", _c_name)),
    _dhmdc(getMaterialPropertyDerivative<Real>("solid_switch", _c_name)),
    _diff_bulk(getParam<Real>("Dbulk")),
    _diff_void(getParam<Real>("Dvoid")),
    _diff_surf(getParam<Real>("Dsurf")),
    _diff_gb(getParam<Real>("Dgb")),
    _s_index(getParam<Real>("surfindex")),
    _gb_index(getParam<Real>("gbindex")),
    _b_index(getParam<Real>("bulkindex")),
    _v_index(getParam<Real>("voidindex")),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _dDdv(_op_num),
    _dhbdv(_op_num),
    _dhmdv(_op_num)

{
  if (_op_num == 0)
    mooseError("Model requires op_num > 0");

  for (MooseIndex(_op_num) op_index = 0; op_index < _op_num; ++op_index)
  {
    _vals[op_index] = &coupledValue("v", op_index);
    const VariableName op_name = getVar("v", op_index)->name();
    _dDdv[op_index] = &declarePropertyDerivative<Real>(_diff_name, getVar("v", op_index)->name());
    _dhbdv[op_index] = &getMaterialPropertyDerivative<Real>("void_switch", op_index);
    _dhmdv[op_index] = &getMaterialPropertyDerivative<Real>("solid_switch", op_index);
  }
}

void
PolycrystalDiffusivity::computeQpProperties()
{
  Real SumEtaij = 0.0;
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != i)
      {
        SumEtaij += 9.0 * (*_vals[i])[_qp] * (*_vals[i])[_qp] * (*_vals[j])[_qp] * (*_vals[j])[_qp];
        SumEtaj += 2.0 * 9.0 * (*_vals[i])[_qp] * (*_vals[j])[_qp] * (*_vals[j])[_qp];
      }

  Real c = _c[_qp];
  c = std::abs(c) > 1.0 ? 1.0 : (c < 0.0 ? std::abs(c) : c);
  Real mc = 1.0 - c;

  // Compute diffusion function
  _diff[_qp] = _b_index * _diff_bulk * _hm[_qp] + _v_index * _diff_void * _hb[_qp] +
               30.0 * _diff_surf * _s_index * c * c * mc * mc + _diff_gb * SumEtaij * _gb_index;

  _dDdc[_qp] = _b_index * _diff_bulk * _dhmdc[_qp] + _v_index * _diff_void * _dhbdc[_qp] +
               30.0 * _diff_surf * _s_index * (2.0 * c * mc * mc - 2.0 * c * c * mc);
  for (MooseIndex(_op_num) op_index = 0; op_index < _op_num; ++op_index)
    (*_dDdv[op_index])[_qp] = _b_index * _diff_bulk * (*_dhmdv[op_index])[_qp] +
                              _v_index * _diff_void * (*_dhbdv[op_index])[_qp] +
                              _diff_gb * SumEtaj * _gb_index;
}
