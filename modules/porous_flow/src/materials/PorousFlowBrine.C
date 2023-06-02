//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrine.h"

registerMooseObject("PorousFlowApp", PorousFlowBrine);

InputParameters
PorousFlowBrine::validParams()
{
  InputParameters params = PorousFlowFluidPropertiesBase::validParams();
  params.addParam<UserObjectName>("water_fp",
                                  "The name of the FluidProperties UserObject for water");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription(
      "This Material calculates fluid properties for brine at the quadpoints or nodes");
  return params;
}

PorousFlowBrine::PorousFlowBrine(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),
    _ddensity_dX(_compute_rho_mu
                     ? (_nodal_material ? &declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _mass_fraction_variable_name)
                                        : &declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_qp" + _phase,
                                              _mass_fraction_variable_name))
                     : nullptr),
    _dviscosity_dX(
        _compute_rho_mu
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>("PorousFlow_viscosity_nodal" + _phase,
                                                      _mass_fraction_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_viscosity_qp" + _phase,
                                                      _mass_fraction_variable_name))
            : nullptr),
    _dinternal_energy_dX(_compute_internal_energy
                             ? (_nodal_material
                                    ? &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _mass_fraction_variable_name)
                                    : &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _mass_fraction_variable_name))
                             : nullptr),
    _denthalpy_dX(_compute_enthalpy
                      ? (_nodal_material ? &declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                               _mass_fraction_variable_name)
                                         : &declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                               _mass_fraction_variable_name))
                      : nullptr),
    _is_xnacl_nodal(isCoupled("xnacl") ? getVar("xnacl", 0)->isNodal() : false),
    _xnacl(_nodal_material && _is_xnacl_nodal ? coupledDofValues("xnacl") : coupledValue("xnacl"))
{
  if (parameters.isParamSetByUser("water_fp"))
  {
    _water_fp = &getUserObject<SinglePhaseFluidProperties>("water_fp");

    // Check that a water userobject has actually been supplied
    if (_water_fp->fluidName() != "water")
      paramError("water_fp", "A water FluidProperties UserObject must be supplied");
  }

  // BrineFluidProperties UserObject
  const std::string brine_name = name() + ":brine";
  {
    const std::string class_name = "BrineFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);

    if (parameters.isParamSetByUser("water_fp"))
      params.set<UserObjectName>("water_fp") = _water_fp->name();

    if (_tid == 0)
      _fe_problem.addUserObject(class_name, brine_name, params);
  }
  _brine_fp = &_fe_problem.getUserObject<BrineFluidProperties>(brine_name);
}

void
PorousFlowBrine::initQpStatefulProperties()
{
  if (_compute_rho_mu)
    (*_density)[_qp] = _brine_fp->rho_from_p_T_X(
        _porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  if (_compute_internal_energy)
    (*_internal_energy)[_qp] = _brine_fp->e_from_p_T_X(
        _porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  if (_compute_enthalpy)
    (*_enthalpy)[_qp] = _brine_fp->h_from_p_T_X(
        _porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
}

void
PorousFlowBrine::computeQpProperties()
{
  const Real Tk = _temperature[_qp] + _t_c2k;

  if (_compute_rho_mu)
  {
    // Density and derivatives wrt pressure and temperature
    Real rho, drho_dp, drho_dT, drho_dx;
    _brine_fp->rho_from_p_T_X(
        _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], rho, drho_dp, drho_dT, drho_dx);
    (*_density)[_qp] = rho;
    (*_ddensity_dp)[_qp] = drho_dp;
    (*_ddensity_dT)[_qp] = drho_dT;
    (*_ddensity_dX)[_qp] = drho_dx;

    // Viscosity and derivatives wrt pressure and temperature
    Real mu, dmu_dp, dmu_dT, dmu_dx;
    _brine_fp->mu_from_p_T_X(
        _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], mu, dmu_dp, dmu_dT, dmu_dx);
    (*_viscosity)[_qp] = mu;
    (*_dviscosity_dp)[_qp] = dmu_dp;
    (*_dviscosity_dT)[_qp] = dmu_dT;
    (*_dviscosity_dX)[_qp] = dmu_dx;
  }

  // Internal energy and derivatives wrt pressure and temperature
  if (_compute_internal_energy)
  {
    Real e, de_dp, de_dT, de_dx;
    _brine_fp->e_from_p_T_X(
        _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], e, de_dp, de_dT, de_dx);
    (*_internal_energy)[_qp] = e;
    (*_dinternal_energy_dp)[_qp] = de_dp;
    (*_dinternal_energy_dT)[_qp] = de_dT;
    (*_dinternal_energy_dX)[_qp] = de_dx;
  }

  // Enthalpy and derivatives wrt pressure and temperature
  if (_compute_enthalpy)
  {
    Real h, dh_dp, dh_dT, dh_dx;
    _brine_fp->h_from_p_T_X(
        _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], h, dh_dp, dh_dT, dh_dx);
    (*_enthalpy)[_qp] = h;
    (*_denthalpy_dp)[_qp] = dh_dp;
    (*_denthalpy_dT)[_qp] = dh_dT;
    (*_denthalpy_dX)[_qp] = dh_dx;
  }
}
