//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "THMNames.h"
#include "MooseUtils.h"
#include "ComponentsConvergence.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1Phase);

InputParameters
FlowChannel1Phase::validParams()
{
  InputParameters params = FlowChannel1PhaseBase::validParams();

  MooseEnum wave_speed_formulation("einfeldt davis", "einfeldt");
  params.addParam<MooseEnum>(
      "wave_speed_formulation", wave_speed_formulation, "Method for computing wave speeds");

  std::vector<Real> sf_1phase(3, 1.0);
  params.addParam<std::vector<Real>>(
      "scaling_factor_1phase",
      sf_1phase,
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  params.addParam<bool>(
      "create_flux_vpp",
      false,
      "If true, create a VectorPostprocessor with the the mass, momentum, and energy side fluxes");

  params.addParam<Real>("p_rel_step_tol", 1e-5, "Pressure relative step tolerance");
  params.addParam<Real>("T_rel_step_tol", 1e-5, "Temperature relative step tolerance");
  params.addParam<Real>("vel_rel_step_tol", 1e-5, "Velocity relative step tolerance");
  params.addParam<Real>("mass_res_tol", 1e-5, "Mass equation normalized residual tolerance");
  params.addParam<Real>(
      "momentum_res_tol", 1e-5, "Momentum equation normalized residual tolerance");
  params.addParam<Real>("energy_res_tol", 1e-5, "Energy equation normalized residual tolerance");

  params.addParamNamesToGroup("scaling_factor_1phase", "Numerical scheme");
  params.addClassDescription("1-phase 1D flow channel");

  return params;
}

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params)
  : FlowChannel1PhaseBase(params), _nl_conv_name(genName(name(), "nlconv"))
{
}

void
FlowChannel1Phase::checkFluidProperties() const
{
  const UserObject & fp = getTHMProblem().getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) == nullptr)
    logError("Supplied fluid properties must be for 1-phase fluids.");
}

std::string
FlowChannel1Phase::flowModelClassName() const
{
  return "FlowModelSinglePhase";
}

std::vector<std::string>
FlowChannel1Phase::ICParameters() const
{
  return {"initial_p", "initial_T", "initial_vel"};
}

void
FlowChannel1Phase::check() const
{
  FlowChannel1PhaseBase::check();
  checkScalingFactors();
}

void
FlowChannel1Phase::checkScalingFactors() const
{
  // If using ComponentsConvergence, make sure that all residual scaling factors
  // are set to one, since a normalized residual norm is used.
  const auto & conv_names = getTHMProblem().getNonlinearConvergenceNames();
  mooseAssert(conv_names.size() == 1, "There must be exactly one nonlinear convergence object.");
  if (dynamic_cast<ComponentsConvergence *>(&getTHMProblem().getConvergence(conv_names[0])))
  {
    const auto & scaling_factors = getParam<std::vector<Real>>("scaling_factor_1phase");
    bool all_are_one = true;
    for (const auto factor : scaling_factors)
      if (!MooseUtils::absoluteFuzzyEqual(factor, 1.0))
        all_are_one = false;
    if (!all_are_one)
      logError("When using ComponentsConvergence, 'scaling_factor_1phase' must be set to '1 1 1'.");
  }
}

void
FlowChannel1Phase::addMooseObjects()
{
  FlowChannel1PhaseBase::addMooseObjects();

  if (getParam<bool>("create_flux_vpp"))
    addNumericalFluxVectorPostprocessor();

  addNonlinearConvergence();
}

void
FlowChannel1Phase::addNumericalFluxVectorPostprocessor()
{
  const std::string class_name = "NumericalFlux3EqnInternalValues";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<MooseEnum>("sort_by") = sortBy();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  getTHMProblem().addVectorPostprocessor(class_name, name() + "_flux_vpp", params);
}

void
FlowChannel1Phase::addNonlinearConvergence()
{
  const std::string class_name = "FlowChannel1PhaseConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<PostprocessorName>("p_rel_step") = genName(name(), "p_rel_step");
  params.set<PostprocessorName>("T_rel_step") = genName(name(), "T_rel_step");
  params.set<PostprocessorName>("vel_rel_step") = genName(name(), "vel_rel_step");
  params.set<PostprocessorName>("mass_res") = genName(name(), "mass_res");
  params.set<PostprocessorName>("momentum_res") = genName(name(), "momentum_res");
  params.set<PostprocessorName>("energy_res") = genName(name(), "energy_res");
  params.applyParameters(parameters());
  getTHMProblem().addConvergence(class_name, _nl_conv_name, params);
}

Convergence *
FlowChannel1Phase::getNonlinearConvergence() const
{
  return &getTHMProblem().getConvergence(_nl_conv_name);
}
