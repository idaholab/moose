/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowPorosityHM.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowPorosityHM>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("porosity_zero", "The porosity at zero volumetric strain and zero effective porepressure");
  params.addRangeCheckedParam<Real>("biot_coefficient", 1, "biot_coefficient>=0&biot_coefficient<=1", "Biot coefficient");
  params.addRequiredRangeCheckedParam<Real>("solid_bulk", "solid_bulk>0", "Bulk modulus of the drained porous solid skeleton");
  params.addRequiredCoupledVar("displacements", "The solid-mechanics displacement variables");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

PorousFlowPorosityHM::PorousFlowPorosityHM(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _phi0(getParam<Real>("porosity_zero")),
    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _biot(getParam<Real>("biot_coefficient")),
    _solid_bulk(getParam<Real>("solid_bulk")),
    _num_var(_dictator_UO.num_variables()),
    _coeff((_biot - 1)/_solid_bulk),

    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),

    _vol_strain_qp(getMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")), // TODO: perhaps can put temperature here at some stage?
    _dvol_strain_qp_dvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_total_volumetric_strain_qp_dvar")),

    _pf_nodal(getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_nodal")),
    _dpf_nodal_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_effective_fluid_pressure_nodal_dvar")),
    _pf_qp(getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_qp")),
    _dpf_qp_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_effective_fluid_pressure_qp_dvar")),

    _porosity_nodal(declareProperty<Real>("PorousFlow_porosity_nodal")),
    _porosity_nodal_old(declarePropertyOld<Real>("PorousFlow_porosity_nodal")),
    _dporosity_nodal_dvar(declareProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_nodal_dgradvar(declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar")),
    _porosity_qp(declareProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_qp_old(declarePropertyOld<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(declareProperty<std::vector<Real> >("dPorousFlow_porosity_qp_dvar")),
    _dporosity_qp_dgradvar(declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_qp_dgradvar"))
{
  for (unsigned i = 0 ; i < _ndisp ; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}

void
PorousFlowPorosityHM::initQpStatefulProperties()
{
  _porosity_nodal[_qp] = _phi0;
  _porosity_qp[_qp] = _phi0;
}

void
PorousFlowPorosityHM::computeQpProperties()
{
  // Note that in the following _strain[_qp] is evaluated at the quadpoint
  // _pf_nodal[_qp] is actually the nodal value (it is just stored at the quadpoint).
  // So _porosity_nodal[_qp], which should be the nodal value of porosity (but
  // stored at the quadpoint) actually uses the strain at the quadpoint.  This
  // is OK for LINEAR elements, as strain is constant over the element anyway.

  _porosity_nodal[_qp] = _biot + (_phi0 - _biot)*std::exp(-_vol_strain_qp[_qp] + _coeff*_pf_nodal[_qp]);
  _porosity_qp[_qp] = _biot + (_phi0 - _biot)*std::exp(-_vol_strain_qp[_qp] + _coeff*_pf_qp[_qp]);

  _dporosity_qp_dvar[_qp].resize(_num_var, 0.0);
  _dporosity_nodal_dvar[_qp].resize(_num_var, 0.0);
  for (unsigned v = 0; v < _num_var; ++v)
  {
    _dporosity_qp_dvar[_qp][v] = _coeff*_dpf_qp_dvar[_qp][v]*(_porosity_qp[_qp] - _biot);
    _dporosity_nodal_dvar[_qp][v] = _coeff*_dpf_nodal_dvar[_qp][v]*(_porosity_nodal[_qp] - _biot);
  }

  _dporosity_qp_dgradvar[_qp].resize(_num_var, RealGradient());
  _dporosity_nodal_dgradvar[_qp].resize(_num_var, RealGradient());
  for (unsigned v = 0; v < _num_var; ++v)
  {
    _dporosity_qp_dgradvar[_qp][v] = -(_porosity_qp[_qp] - _biot)*_dvol_strain_qp_dvar[_qp][v];
    _dporosity_nodal_dgradvar[_qp][v] = -(_porosity_nodal[_qp] - _biot)*_dvol_strain_qp_dvar[_qp][v];
  }
}

