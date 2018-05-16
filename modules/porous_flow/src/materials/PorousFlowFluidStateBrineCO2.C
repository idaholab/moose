//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBrineCO2.h"
#include "PorousFlowCapillaryPressure.h"
#include "PorousFlowBrineCO2.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateBrineCO2);

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlashBase>();
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2::PorousFlowFluidStateBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateFlashBase(parameters),
    _Xnacl(_nodal_material ? coupledNodalValue("xnacl") : coupledValue("xnacl")),
    _grad_Xnacl_qp(coupledGradient("xnacl")),
    _Xnacl_varnum(coupled("xnacl")),
    _Xvar(_dictator.isPorousFlowVariable(_Xnacl_varnum)
              ? _dictator.porousFlowVariableNum(_Xnacl_varnum)
              : 0),
    _fs_uo(getUserObject<PorousFlowBrineCO2>("fluid_state")),
    _salt_component(_fs_uo.saltComponentIndex())
{
  // Check that a valid Brine-CO2 FluidState has been supplied in fluid_state
  if (_fs_uo.fluidStateName() != "brine-co2")
    paramError("fluid_state", "Only a valid Brine-CO2 FluidState can be used");
}

void
PorousFlowFluidStateBrineCO2::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  _fs_uo.thermophysicalProperties(_gas_porepressure[_qp], Tk, _Xnacl[_qp], (*_Z[0])[_qp], _fsp);
}

void
PorousFlowFluidStateBrineCO2::computeQpProperties()
{
  PorousFlowFluidStateFlashBase::computeQpProperties();

  // If Xnacl is a PorousFlow variable, add contribution to material properties
  // due to the Xnacl component that are not included in
  // PorousFlowFluidStateFlashBase::computeQpProperties();
  if (_dictator.isPorousFlowVariable(_Xnacl_varnum))
  {
    // Derivative of saturation wrt variables
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
      _dsaturation_dvar[_qp][ph][_Xvar] = _fsp[ph].dsaturation_dX;

    // Derivative of capillary pressure
    Real dpc = _pc_uo.dCapillaryPressure(_fsp[_aqueous_phase_number].saturation);

    // Derivative of porepressure wrt variables
    if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
    {
      if (!_nodal_material)
        (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_Xvar] =
            -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar];

      // The aqueous phase porepressure is also a function of liquid saturation,
      // which depends on Xnacl
      _dporepressure_dvar[_qp][_aqueous_phase_number][_Xvar] =
          -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar];
    }

    // Calculate derivatives of material properties wrt primary variables
    // Derivative of Xnacl wrt variables
    std::vector<Real> dX_dvar;
    dX_dvar.assign(_num_pf_vars, 0.0);

    if (_dictator.isPorousFlowVariable(_Xvar))
      dX_dvar[_Xvar] = 1.0;

    // Derivatives of properties wrt primary variables
    for (unsigned int v = 0; v < _num_pf_vars; ++v)
      for (unsigned int ph = 0; ph < _num_phases; ++ph)
      {
        // Derivative of density in each phase
        _dfluid_density_dvar[_qp][ph][v] += _fsp[ph].ddensity_dX * dX_dvar[v];

        // Derivative of viscosity in each phase
        _dfluid_viscosity_dvar[_qp][ph][v] += _fsp[ph].dviscosity_dX * dX_dvar[v];
      }

    // The derivative of the mass fractions for each fluid component in each phase.
    // Note: these are all calculated in terms of gas pressuse, so there is no
    // capillary pressure effect, and hence no need to multiply by _dporepressure_dvar
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp][_Xvar] = _fsp[ph].dmass_fraction_dX[comp];

    // If the material properties are being evaluated at the qps, add the contribution
    // to the gradients as well. Note: only nodal properties are evaluated in
    // initQpStatefulProperties(), so no need to check _is_initqp flag for qp
    // properties
    if (!_nodal_material)
    {
      // Second derivative of capillary pressure
      Real d2pc = _pc_uo.d2CapillaryPressure(_fsp[_aqueous_phase_number].saturation);

      (*_grads_qp)[_qp][_aqueous_phase_number] +=
          _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_grads_qp)[_qp][_gas_phase_number] -=
          _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_gradp_qp)[_qp][_aqueous_phase_number] -=
          dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_Xvar] =
          -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
          _dsaturation_dvar[_qp][_aqueous_phase_number][_Xvar];

      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_salt_component] = _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component] +=
          _fsp[_aqueous_phase_number].dmass_fraction_dX[_aqueous_fluid_component] *
          _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] -=
          _fsp[_aqueous_phase_number].dmass_fraction_dX[_aqueous_fluid_component] *
          _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] +=
          _fsp[_gas_phase_number].dmass_fraction_dX[_aqueous_fluid_component] * _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_gas_fluid_component] -=
          _fsp[_gas_phase_number].dmass_fraction_dX[_aqueous_fluid_component] * _grad_Xnacl_qp[_qp];
    }
  }
}
