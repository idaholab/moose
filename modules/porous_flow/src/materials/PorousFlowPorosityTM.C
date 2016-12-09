/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityTM.h"

template<>
InputParameters validParams<PorousFlowPorosityTM>()
{
  InputParameters params = validParams<PorousFlowPorosityBase>();
  params.addRequiredCoupledVar("porosity_zero", "The porosity at zero volumetric strain and zero temperature");
  params.addRequiredParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient of the drained porous solid skeleton");
  params.addRequiredRangeCheckedParam<Real>("solid_bulk", "solid_bulk>0", "Bulk modulus of the drained porous solid skeleton");
  params.addRequiredCoupledVar("displacements", "The solid-mechanics displacement variables");
  params.addClassDescription("This Material calculates the porosity for hydro-mechanical simulations");
  return params;
}

PorousFlowPorosityTM::PorousFlowPorosityTM(const InputParameters & parameters) :
    PorousFlowPorosityBase(parameters),

    _phi0(coupledValue("porosity_zero")),
    _exp_coeff(getParam<Real>("thermal_expansion_coeff")),
    _solid_bulk(getParam<Real>("solid_bulk")),

    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),

    _vol_strain_qp(getMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_strain_qp_dvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_total_volumetric_strain_qp_dvar")),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal") : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar(_nodal_material ? getMaterialProperty<std::vector<Real> >("dPorousFlow_temperature_nodal_dvar") : getMaterialProperty<std::vector<Real> >("dPorousFlow_temperature_qp_dvar"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}

void
PorousFlowPorosityTM::initQpStatefulProperties()
{
  _porosity[_qp] = _phi0[_qp];
}

void
PorousFlowPorosityTM::computeQpProperties()
{
  // Note that in the following _strain[_qp] is evaluated at the quadpoint.
  // _temperature_nodal[_qp] is actually the nodal value (it is just stored at the quadpoint).
  // So _porosity_nodal[_qp], which should be the nodal value of porosity (but
  // stored at the quadpoint) actually uses the strain at the quadpoint.  This
  // is OK for LINEAR elements, as strain is constant over the element anyway.
  _porosity[_qp] = 1.0 + (_phi0[_qp] - 1.0) * std::exp(-_vol_strain_qp[_qp] + _exp_coeff * _temperature[_qp]);

  _dporosity_dvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    _dporosity_dvar[_qp][v] = _exp_coeff * _dtemperature_dvar[_qp][v] * (_porosity[_qp] - 1.0);

  _dporosity_dgradvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    _dporosity_dgradvar[_qp][v] = - (_porosity[_qp] - 1.0) * _dvol_strain_qp_dvar[_qp][v];
}
