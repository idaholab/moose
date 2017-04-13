/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityTM.h"

template <>
InputParameters
validParams<PorousFlowPorosityTM>()
{
  InputParameters params = validParams<PorousFlowPorosityExponentialBase>();
  params.addRequiredCoupledVar("porosity_zero",
                               "The porosity at zero volumetric strain and zero temperature");
  params.addRequiredParam<Real>(
      "thermal_expansion_coeff",
      "Thermal expansion coefficient of the drained porous solid skeleton");
  params.addRequiredRangeCheckedParam<Real>(
      "solid_bulk", "solid_bulk>0", "Bulk modulus of the drained porous solid skeleton");
  params.addRequiredCoupledVar("displacements", "The solid-mechanics displacement variables");
  params.addClassDescription(
      "This Material calculates the porosity for hydro-mechanical simulations");
  return params;
}

PorousFlowPorosityTM::PorousFlowPorosityTM(const InputParameters & parameters)
  : PorousFlowPorosityExponentialBase(parameters),

    _phi0(_nodal_material ? coupledNodalValue("porosity_zero") : coupledValue("porosity_zero")),
    _exp_coeff(getParam<Real>("thermal_expansion_coeff")),
    _solid_bulk(getParam<Real>("solid_bulk")),

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
  return -_vol_strain_qp[qp_to_use] + _exp_coeff * _temperature[_qp];
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
