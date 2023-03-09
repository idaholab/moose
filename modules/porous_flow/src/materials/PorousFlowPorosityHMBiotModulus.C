//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityHMBiotModulus.h"
#include "libmesh/utility.h"

registerMooseObject("PorousFlowApp", PorousFlowPorosityHMBiotModulus);

InputParameters
PorousFlowPorosityHMBiotModulus::validParams()
{
  InputParameters params = PorousFlowPorosity::validParams();
  params.set<bool>("mechanical") = true;
  params.set<bool>("fluid") = true;
  params.addRequiredRangeCheckedParam<Real>("constant_biot_modulus",
                                            "constant_biot_modulus>0",
                                            "Biot modulus, which is constant for this Material");
  params.addRequiredRangeCheckedParam<Real>(
      "constant_fluid_bulk_modulus",
      "constant_fluid_bulk_modulus>0",
      "Fluid bulk modulus, which is constant for this Material");
  params.addClassDescription(
      "This Material calculates the porosity for hydro-mechanical simulations, assuming that the "
      "Biot modulus and the fluid bulk modulus are both constant.  This is useful for comparing "
      "with solutions from poroelasticity theory, but is less accurate than PorousFlowPorosity");
  return params;
}

PorousFlowPorosityHMBiotModulus::PorousFlowPorosityHMBiotModulus(const InputParameters & parameters)
  : PorousFlowPorosity(parameters),
    _porosity_old(_nodal_material ? getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")
                                  : getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _biot_modulus(getParam<Real>("constant_biot_modulus")),
    _fluid_bulk_modulus(getParam<Real>("constant_fluid_bulk_modulus")),
    _pf_old(_nodal_material
                ? getMaterialPropertyOld<Real>("PorousFlow_effective_fluid_pressure_nodal")
                : getMaterialPropertyOld<Real>("PorousFlow_effective_fluid_pressure_qp")),
    _vol_strain_qp_old(getMaterialPropertyOld<Real>("PorousFlow_total_volumetric_strain_qp")),
    _vol_strain_rate_qp(getMaterialProperty<Real>("PorousFlow_volumetric_strain_rate_qp")),
    _dvol_strain_rate_qp_dvar(getMaterialProperty<std::vector<RealGradient>>(
        "dPorousFlow_volumetric_strain_rate_qp_dvar"))
{
}

void
PorousFlowPorosityHMBiotModulus::computeQpProperties()
{
  // Note that in the following _strain[_qp] is evaluated at q quadpoint
  // So _porosity_nodal[_qp], which should be the nodal value of porosity
  // actually uses the strain at a quadpoint.  This
  // is OK for LINEAR elements, as strain is constant over the element anyway.

  const unsigned qp_to_use =
      (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);

  const Real denom = (1.0 + _vol_strain_rate_qp[qp_to_use] * _dt + _vol_strain_qp_old[qp_to_use]);
  const Real s_p = _porosity_old[_qp] * (1 + _vol_strain_qp_old[qp_to_use]);
  _porosity[_qp] = (s_p * std::exp(-((*_pf)[_qp] - _pf_old[_qp]) / _fluid_bulk_modulus) +
                    ((*_pf)[_qp] - _pf_old[_qp]) / _biot_modulus +
                    _biot * ((*_vol_strain_qp)[qp_to_use] - _vol_strain_qp_old[qp_to_use])) /
                   denom;

  (*_dporosity_dvar)[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    (*_dporosity_dvar)[_qp][v] =
        (*_dpf_dvar)[_qp][v] *
        (-s_p * std::exp(-((*_pf)[_qp] - _pf_old[_qp]) / _fluid_bulk_modulus) /
             _fluid_bulk_modulus +
         1.0 / _biot_modulus) /
        denom;

  (*_dporosity_dgradvar)[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
    (*_dporosity_dgradvar)[_qp][v] =
        _biot * (*_dvol_strain_qp_dvar)[qp_to_use][v] / denom -
        _porosity[_qp] / denom * _dvol_strain_rate_qp_dvar[qp_to_use][v] * _dt;
}
