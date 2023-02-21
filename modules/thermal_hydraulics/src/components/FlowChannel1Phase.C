//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "HeatTransfer1PhaseBase.h"
#include "Closures1PhaseBase.h"
#include "ThermalHydraulicsApp.h"
#include "SlopeReconstruction1DInterface.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1Phase);

InputParameters
FlowChannel1Phase::validParams()
{
  InputParameters params = FlowChannelBase::validParams();

  params.addParam<FunctionName>("initial_p", "Initial pressure in the flow channel [Pa]");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the flow channel [m/s]");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the flow channel [K]");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter [m]");
  params.addParam<MooseEnum>(
      "rdg_slope_reconstruction",
      SlopeReconstruction1DInterface<true>::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction type for rDG spatial discretization");
  std::vector<Real> sf_1phase(3, 1.0);
  params.addParam<std::vector<Real>>(
      "scaling_factor_1phase",
      sf_1phase,
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");

  params.declareControllable("initial_p initial_T initial_vel D_h");
  params.addParamNamesToGroup("initial_p initial_T initial_vel", "Variable initialization");
  params.addParamNamesToGroup("rdg_slope_reconstruction scaling_factor_1phase", "Numerical scheme");
  params.addClassDescription("1-phase 1D flow channel");

  return params;
}

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params)
  : FlowChannelBase(params),

    _numerical_flux_name(genName(name(), "numerical_flux")),
    _rdg_slope_reconstruction(getParam<MooseEnum>("rdg_slope_reconstruction"))
{
}

void
FlowChannel1Phase::init()
{
  FlowChannelBase::init();

  const UserObject & fp = getTHMProblem().getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) == nullptr)
    logError("Supplied fluid properties must be for 1-phase fluids.");
}

std::shared_ptr<FlowModel>
FlowChannel1Phase::buildFlowModel()
{
  const std::string class_name = "FlowModelSinglePhase";
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<THMProblem *>("_thm_problem") = &getTHMProblem();
  pars.set<FlowChannelBase *>("_flow_channel") = this;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<bool>("output_vector_velocity") = getTHMProblem().getVectorValuedVelocity();
  pars.applyParameters(parameters());
  return _factory.create<FlowModel>(class_name, name(), pars, 0);
}

void
FlowChannel1Phase::check() const
{
  FlowChannelBase::check();

  // only 1-phase flow compatible heat transfers are allowed
  for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
  {
    if (!hasComponentByName<HeatTransfer1PhaseBase>(_heat_transfer_names[i]))
      logError("Coupled heat source '",
               _heat_transfer_names[i],
               "' is not compatible with single phase flow channel. Use single phase heat transfer "
               "component instead.");
  }

  bool ics_set =
      getTHMProblem().hasInitialConditionsFromFile() ||
      (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel"));

  if (!ics_set && !_app.isRestarting())
  {
    // create a list of the missing IC parameters
    const std::vector<std::string> ic_params{"initial_p", "initial_T", "initial_vel"};
    std::ostringstream oss;
    for (const auto & ic_param : ic_params)
      if (!isParamValid(ic_param))
        oss << " " << ic_param;

    logError("The following initial condition parameters are missing:", oss.str());
  }
}

void
FlowChannel1Phase::addMooseObjects()
{
  FlowChannelBase::addMooseObjects();

  if (!_pipe_pars_transferred)
    addHydraulicDiameterMaterial();
}

void
FlowChannel1Phase::addHydraulicDiameterMaterial()
{
  const std::string mat_name = genName(name(), "D_h_material");

  if (isParamValid("D_h"))
  {
    const FunctionName & D_h_fn_name = getParam<FunctionName>("D_h");

    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {FlowModelSinglePhase::HYDRAULIC_DIAMETER};
    params.set<std::vector<FunctionName>>("prop_values") = {D_h_fn_name};
    getTHMProblem().addMaterial(class_name, mat_name, params);

    makeFunctionControllableIfConstant(D_h_fn_name, "D_h");
  }
  else
  {
    const std::string class_name = "ADHydraulicDiameterCircularMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<MaterialPropertyName>("D_h_name") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    getTHMProblem().addMaterial(class_name, mat_name, params);
  }
}

void
FlowChannel1Phase::getHeatTransferVariableNames()
{
  FlowChannelBase::getHeatTransferVariableNames();

  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const HeatTransfer1PhaseBase & heat_transfer =
        getComponentByName<HeatTransfer1PhaseBase>(_heat_transfer_names[i]);

    _Hw_1phase_names.push_back(heat_transfer.getWallHeatTransferCoefficient1PhaseName());
  }
}
