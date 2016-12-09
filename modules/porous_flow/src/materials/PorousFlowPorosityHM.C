/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityHM.h"

template<>
InputParameters validParams<PorousFlowPorosityHM>()
{
  InputParameters params = validParams<PorousFlowPorosityBase>();
  params.addRequiredCoupledVar("porosity_zero", "The porosity at zero volumetric strain and zero effective porepressure");
  params.addRangeCheckedParam<Real>("biot_coefficient", 1, "biot_coefficient>=0 & biot_coefficient<=1", "Biot coefficient");
  params.addRequiredRangeCheckedParam<Real>("solid_bulk", "solid_bulk>0", "Bulk modulus of the drained porous solid skeleton");
  params.addRequiredCoupledVar("displacements", "The solid-mechanics displacement variables");
  params.addClassDescription("This Material calculates the porosity for hydro-mechanical simulations");
  return params;
}

PorousFlowPorosityHM::PorousFlowPorosityHM(const InputParameters & parameters) :
    PorousFlowPorosityBase(parameters),

    _phi0(coupledValue("porosity_zero")),
    _biot(getParam<Real>("biot_coefficient")),
    _solid_bulk(getParam<Real>("solid_bulk")),
    _coeff((_biot - 1.0) / _solid_bulk),

    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),

    _vol_strain_qp(getMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_strain_qp_dvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_total_volumetric_strain_qp_dvar")),

    _pf(_nodal_material ? getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_nodal") : getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_qp")),
    _dpf_dvar(_nodal_material ? getMaterialProperty<std::vector<Real> >("dPorousFlow_effective_fluid_pressure_nodal_dvar") : getMaterialProperty<std::vector<Real> >("dPorousFlow_effective_fluid_pressure_qp_dvar"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}

void
PorousFlowPorosityHM::initQpStatefulProperties()
{
  _porosity[_qp] = _phi0[_qp];
}

void
PorousFlowPorosityHM::computeQpProperties()
{
  // Note that in the following _strain[_qp] is evaluated at the quadpoint
  // _pf_nodal[_qp] is actually the nodal value (it is just stored at the quadpoint).
  // So _porosity_nodal[_qp], which should be the nodal value of porosity (but
  // stored at the quadpoint) actually uses the strain at the quadpoint.  This
  // is OK for LINEAR elements, as strain is constant over the element anyway.

  _porosity[_qp] = _biot + (_phi0[_qp] - _biot) * std::exp(-_vol_strain_qp[_qp] + _coeff * _pf[_qp]);

  _dporosity_dvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    _dporosity_dvar[_qp][v] = _coeff * _dpf_dvar[_qp][v] * (_porosity[_qp] - _biot);

  _dporosity_dgradvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    _dporosity_dgradvar[_qp][v] = -(_porosity[_qp] - _biot) * _dvol_strain_qp_dvar[_qp][v];
}
