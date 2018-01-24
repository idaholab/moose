//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityTM.h"

template <>
InputParameters
validParams<PorousFlowPorosityTM>()
{
  InputParameters params = validParams<PorousFlowPorosityExponentialBase>();
  params.addRequiredCoupledVar("porosity_zero",
                               "The porosity at zero volumetric strain and reference temperature");
  params.addRequiredParam<Real>(
      "thermal_expansion_coeff",
      "Volumetric thermal expansion coefficient of the drained porous solid skeleton");
  params.addRequiredCoupledVar("displacements", "The solid-mechanics displacement variables");
  params.addCoupledVar(
      "reference_temperature", 0.0, "porosity = porosity_zero at reference temperature");
  params.addClassDescription(
      "This Material calculates the porosity for hydro-mechanical simulations");
  return params;
}

PorousFlowPorosityTM::PorousFlowPorosityTM(const InputParameters & parameters)
  : PorousFlowPorosityExponentialBase(parameters),

    _phi0(_nodal_material ? coupledNodalValue("porosity_zero") : coupledValue("porosity_zero")),
    _exp_coeff(getParam<Real>("thermal_expansion_coeff")),

    _t_reference(_nodal_material ? coupledNodalValue("reference_temperature")
                                 : coupledValue("reference_temperature")),

    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),

    _vol_strain_qp(getMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_strain_qp_dvar(getMaterialProperty<std::vector<RealGradient>>(
        "dPorousFlow_total_volumetric_strain_qp_dvar")),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}
Real
PorousFlowPorosityTM::atNegInfinityQp() const
{
  return 1.0;
}

Real
PorousFlowPorosityTM::atZeroQp() const
{
  return _phi0[_qp];
}

Real
PorousFlowPorosityTM::decayQp() const
{
  // Note that in the following _strain[_qp] is evaluated at q quadpoint
  // So _porosity_nodal[_qp], which should be the nodal value of porosity
  // actually uses the strain at a quadpoint.  This
  // is OK for LINEAR elements, as strain is constant over the element anyway.
  const unsigned qp_to_use =
      (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
  return -_vol_strain_qp[qp_to_use] + _exp_coeff * (_temperature[_qp] - _t_reference[_qp]);
}

Real
PorousFlowPorosityTM::ddecayQp_dvar(unsigned pvar) const
{
  return _exp_coeff * _dtemperature_dvar[_qp][pvar];
}

RealGradient
PorousFlowPorosityTM::ddecayQp_dgradvar(unsigned pvar) const
{
  const unsigned qp_to_use =
      (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
  return -_dvol_strain_qp_dvar[qp_to_use][pvar];
}
