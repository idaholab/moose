//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeDiffuseInterfaceMixtureEnthalpy.h"

#include "SinglePhaseFluidProperties.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "ElemInfo.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", ConservativeDiffuseInterfaceMixtureEnthalpy);

namespace
{
/// Relative temperature change at which the Newton recovery of T stops, near round-off for the
/// temperatures of order 1e2-1e3 K this is used at
constexpr Real temperature_relative_tolerance = 1e-12;
/// Newton iterations allowed per cell; h_k(T) is smooth and monotonic, so the recovery converges
/// quadratically in a few iterations from the lagged temperature
constexpr unsigned int max_newton_iterations = 50;
}

InputParameters
ConservativeDiffuseInterfaceMixtureEnthalpy::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Relates the conserved mixture enthalpy density of a two-phase diffuse-interface flow to its "
      "temperature, sets it initially and recovers the temperature from it.");
  params.addRequiredParam<VariableName>("enthalpy_variable",
                                        "The solved mixture enthalpy density rho*h.");
  params.addRequiredParam<VariableName>("temperature",
                                        "The temperature auxiliary variable recovered from rho*h.");
  params.addRequiredParam<MooseFunctorName>("volume_fraction", "The liquid volume fraction.");
  params.addRequiredParam<MooseFunctorName>("liquid_density", "The constant liquid density.");
  params.addRequiredParam<MooseFunctorName>("gas_density", "The constant gas density.");
  params.addRequiredParam<UserObjectName>("liquid_fluid_properties",
                                          "Fluid properties giving the liquid enthalpy h(p, T).");
  params.addRequiredParam<UserObjectName>("gas_fluid_properties",
                                          "Fluid properties giving the gas enthalpy h(p, T).");
  params.addRequiredParam<Real>("thermodynamic_pressure",
                                "Pressure at which the phase enthalpies are evaluated.");

  // Only the initial rho*h is set on execution; the solve recovers the temperature explicitly
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  // Before the initial postprocessors, which may read rho*h
  params.set<int>("execution_order_group") = -1;
  return params;
}

ConservativeDiffuseInterfaceMixtureEnthalpy::ConservativeDiffuseInterfaceMixtureEnthalpy(
    const InputParameters & params)
  : GeneralUserObject(params),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    _enthalpy_name(getParam<VariableName>("enthalpy_variable")),
    _temperature_name(getParam<VariableName>("temperature")),
    _alpha(getFunctor<Real>("volume_fraction")),
    _density({&getFunctor<Real>("liquid_density"), &getFunctor<Real>("gas_density")}),
    _fp({&getUserObject<SinglePhaseFluidProperties>("liquid_fluid_properties"),
         &getUserObject<SinglePhaseFluidProperties>("gas_fluid_properties")}),
    _thermodynamic_pressure(getParam<Real>("thermodynamic_pressure"))
{
}

void
ConservativeDiffuseInterfaceMixtureEnthalpy::initialSetup()
{
  cacheVariables();
}

void
ConservativeDiffuseInterfaceMixtureEnthalpy::meshChanged()
{
  cacheVariables();
}

void
ConservativeDiffuseInterfaceMixtureEnthalpy::cacheVariables()
{
  _enthalpy_var = dynamic_cast<MooseLinearVariableFVReal *>(
      &UserObject::_subproblem.getVariable(_tid, _enthalpy_name));
  if (!_enthalpy_var)
    paramError("enthalpy_variable", "The enthalpy variable must be a MooseLinearVariableFVReal.");

  _temperature_var = dynamic_cast<MooseLinearVariableFVReal *>(
      &UserObject::_subproblem.getVariable(_tid, _temperature_name));
  if (!_temperature_var)
    paramError("temperature", "The temperature variable must be a MooseLinearVariableFVReal.");
}

std::pair<Real, Real>
ConservativeDiffuseInterfaceMixtureEnthalpy::phaseEnthalpy(const Phase phase,
                                                           const Real temperature) const
{
  Real h, dh_dp, dh_dT;
  _fp[phase]->h_from_p_T(_thermodynamic_pressure, temperature, h, dh_dp, dh_dT);
  return {h, dh_dT};
}

Real
ConservativeDiffuseInterfaceMixtureEnthalpy::phaseDensity(const Phase phase,
                                                          const Moose::FaceArg & face) const
{
  return (*_density[phase])(face, Moose::currentState());
}

Real
ConservativeDiffuseInterfaceMixtureEnthalpy::enthalpyDensity(const Real alpha,
                                                             const Real temperature,
                                                             const Moose::FaceArg & face) const
{
  return alpha * phaseDensity(LIQUID, face) * phaseEnthalpy(LIQUID, temperature).first +
         (1.0 - alpha) * phaseDensity(GAS, face) * phaseEnthalpy(GAS, temperature).first;
}

ConservativeDiffuseInterfaceMixtureEnthalpy::CellLinearization
ConservativeDiffuseInterfaceMixtureEnthalpy::cellLinearization(const ElemInfo & elem_info) const
{
  const auto state = Moose::currentState();
  const auto elem_arg = makeElemArg(elem_info.elem());
  const Real alpha = _alpha(elem_arg, state);
  const std::array<Real, num_phases> fraction{alpha, 1.0 - alpha};

  CellLinearization lin{_temperature_var->getElemValue(elem_info, state), 0.0, 0.0, 0.0};
  Real resistivity = 0.0;
  for (const auto phase : {LIQUID, GAS})
  {
    const auto [h, cp] = phaseEnthalpy(phase, lin.temperature);
    const Real alpha_rho = fraction[phase] * (*_density[phase])(elem_arg, state);
    lin.enthalpy_density += alpha_rho * h;
    lin.heat_capacity += alpha_rho * cp;
    resistivity +=
        fraction[phase] / _fp[phase]->k_from_p_T(_thermodynamic_pressure, lin.temperature);
  }
  // The phases conduct in series across the interface. Summed over the cells of a diffuse
  // interface, the cell resistances then add up to the two layers' own, sum_k H_k / k_k, however
  // many cells the interface spreads over; volume-weighting k instead lets heat leak through it.
  lin.conductivity = 1.0 / resistivity;
  return lin;
}

void
ConservativeDiffuseInterfaceMixtureEnthalpy::recoverTemperature()
{
  const auto state = Moose::currentState();
  const auto t_sys = _temperature_var->sys().number();
  const auto t_var = _temperature_var->number();
  auto & temperature_solution = _temperature_var->sys().solution();

  for (const auto * elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (elem_info->elem()->processor_id() != processor_id() ||
        !hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto elem_arg = makeElemArg(elem_info->elem());
    const Real alpha = _alpha(elem_arg, state);
    const std::array<Real, num_phases> alpha_rho{alpha * (*_density[LIQUID])(elem_arg, state),
                                                 (1.0 - alpha) * (*_density[GAS])(elem_arg, state)};
    const Real enthalpy_density = _enthalpy_var->getElemValue(*elem_info, state);

    Real temperature = _temperature_var->getElemValue(*elem_info, state);
    bool converged = false;
    for (unsigned int it = 0; it < max_newton_iterations && !converged; ++it)
    {
      Real residual = -enthalpy_density;
      Real heat_capacity = 0.0;
      for (const auto phase : {LIQUID, GAS})
      {
        const auto [h, cp] = phaseEnthalpy(phase, temperature);
        residual += alpha_rho[phase] * h;
        heat_capacity += alpha_rho[phase] * cp;
      }
      const Real delta = -residual / heat_capacity;
      temperature += delta;
      converged = std::abs(delta) <= temperature_relative_tolerance * std::abs(temperature);
    }
    if (!converged)
      mooseError("The temperature recovery from rho*h = ",
                 enthalpy_density,
                 " did not converge in element ",
                 elem_info->elem()->id(),
                 " (volume fraction ",
                 alpha,
                 ").");

    temperature_solution.set(elem_info->dofIndices()[t_sys][t_var], temperature);
  }

  temperature_solution.close();
  _temperature_var->sys().system().update();
}

void
ConservativeDiffuseInterfaceMixtureEnthalpy::execute()
{
  // A recovered run restores rho*h from the checkpoint
  if (_app.isRecovering())
    return;

  const auto h_sys = _enthalpy_var->sys().number();
  const auto h_var = _enthalpy_var->number();
  auto & enthalpy_solution = _enthalpy_var->sys().solution();

  for (const auto * elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (elem_info->elem()->processor_id() != processor_id() ||
        !hasBlocks(elem_info->subdomain_id()))
      continue;

    enthalpy_solution.set(elem_info->dofIndices()[h_sys][h_var],
                          cellLinearization(*elem_info).enthalpy_density);
  }

  enthalpy_solution.close();
  _enthalpy_var->sys().system().update();
}
