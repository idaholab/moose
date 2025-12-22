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

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params) : FlowChannel1PhaseBase(params)
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
FlowChannel1Phase::addMooseObjects()
{
  FlowChannel1PhaseBase::addMooseObjects();

  if (getParam<bool>("create_flux_vpp"))
    addNumericalFluxVectorPostprocessor();

  addFlowChannel1PhaseFunctorMaterial();

  const std::vector<std::pair<std::string, Real>> var_norm_pairs{
      {THM::PRESSURE, getParam<Real>("p_ref")},
      {THM::TEMPERATURE, getParam<Real>("T_ref")},
      {THM::VELOCITY, getParam<Real>("vel_ref")}};
  for (const auto & [var, norm] : var_norm_pairs)
  {
    addNonlinearStepFunctorMaterial(
        THM::functorMaterialPropertyName<false>(var), var + "_change", false);
    addMaximumFunctorPostprocessor(
        var + "_change", genName(name(), var + "_rel_step"), norm, getSubdomainNames());
  }

  const std::vector<std::pair<std::string, std::string>> var_eq_pairs{
      {THM::RHOA, "mass"}, {THM::RHOUA, "momentum"}, {THM::RHOEA, "energy"}};
  for (const auto & [var, eq] : var_eq_pairs)
    addNormalized1PhaseResidualNorm(var, eq);

  addMultiPostprocessorConvergence(
      {genName(name(), "p_rel_step"),
       genName(name(), "T_rel_step"),
       genName(name(), "vel_rel_step"),
       genName(name(), "mass_res"),
       genName(name(), "momentum_res"),
       genName(name(), "energy_res")},
      {"step: p", "step: T", "step: vel", "res: mass", "res: momentum", "res: energy"},
      {getParam<Real>("p_rel_step_tol"),
       getParam<Real>("T_rel_step_tol"),
       getParam<Real>("vel_rel_step_tol"),
       getParam<Real>("mass_res_tol"),
       getParam<Real>("momentum_res_tol"),
       getParam<Real>("energy_res_tol")});
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
FlowChannel1Phase::addFlowChannel1PhaseFunctorMaterial()
{
  const std::string class_name = "FlowModel1PhaseFunctorMaterial";
  const std::string obj_name = genName(name(), "fm1phase_fmat");
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  getTHMProblem().addFunctorMaterial(class_name, obj_name, params);
}

void
FlowChannel1Phase::addNormalized1PhaseResidualNorm(const VariableName & variable,
                                                   const std::string & equation)
{
  const std::string class_name = "Normalized1PhaseResidualNorm";
  InputParameters params = _factory.getValidParams(class_name);
  params.applyParameters(parameters());
  params.set<VariableName>("variable") = variable;
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<MooseEnum>("norm_type") = "l_inf";
  const Point mid_point = 0.5 * (getStartPoint() + getEndPoint());
  params.set<Point>("point") = mid_point;
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<Real>("min_elem_size") = getMinimumElemSize();
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR_CONVERGENCE;
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  getTHMProblem().addPostprocessor(class_name, genName(name(), equation + "_res"), params);
}

Convergence *
FlowChannel1Phase::getNonlinearConvergence() const
{
  return &getTHMProblem().getConvergence(nonlinearConvergenceName());
}
