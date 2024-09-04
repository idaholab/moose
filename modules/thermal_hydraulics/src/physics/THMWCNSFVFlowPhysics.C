//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMWCNSFVFlowPhysics.h"
#include "NSFVBase.h"
#include "THMProblem.h"
#include "FlowChannelBase.h"
#include "SlopeReconstruction1DInterface.h"
#include "PhysicsFlowBoundary.h"

// TODO: consolidate those at the THMPhysics parent class level
typedef THMWCNSFVFlowPhysics WCNSFV;
const std::string WCNSFV::DENSITY = "rho";
const std::string WCNSFV::FRICTION_FACTOR_DARCY = "f_D";
const std::string WCNSFV::DYNAMIC_VISCOSITY = "mu";
const std::string WCNSFV::HEAT_TRANSFER_COEFFICIENT_WALL = "Hw";
const std::string WCNSFV::HYDRAULIC_DIAMETER = "D_h";
const std::string WCNSFV::PRESSURE = "p";
const std::string WCNSFV::RHOA = "rhoA";
const std::string WCNSFV::RHOEA = "rhoEA";
const std::string WCNSFV::RHOUA = "rhouA";
const std::string WCNSFV::SOUND_SPEED = "c";
const std::string WCNSFV::SPECIFIC_HEAT_CONSTANT_PRESSURE = "cp";
const std::string WCNSFV::SPECIFIC_HEAT_CONSTANT_VOLUME = "cv";
const std::string WCNSFV::SPECIFIC_INTERNAL_ENERGY = "e";
const std::string WCNSFV::SPECIFIC_TOTAL_ENTHALPY = "H";
const std::string WCNSFV::SPECIFIC_VOLUME = "v";
const std::string WCNSFV::TEMPERATURE = "T";
const std::string WCNSFV::THERMAL_CONDUCTIVITY = "k";
const std::string WCNSFV::VELOCITY = "vel";
const std::string WCNSFV::VELOCITY_X = "vel_x";
const std::string WCNSFV::VELOCITY_Y = "vel_y";
const std::string WCNSFV::VELOCITY_Z = "vel_z";
const std::string WCNSFV::REYNOLDS_NUMBER = "Re";

registerTHMFlowModelPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVFlowPhysics);
registerNavierStokesPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVFlowPhysics);
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "THMPhysics:add_ic");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_ic");

// From WCNSFVFlowPhysics
// TODO: make sure list is minimal
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_fv_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_fv_bc");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_material");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_corrector");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_postprocessor");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_aux_variable");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_aux_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_user_object");

InputParameters
THMWCNSFVFlowPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();
  params += WCNSFVFlowPhysics::validParams();

  params.addParam<bool>("output_inlet_areas",
                        false,
                        "Whether to output the postprocessors measuring the inlet areas");

  // Suppress direct setting of boundary parameters from the physics, since these will be set by
  // flow boundary components
  params.suppressParameter<std::vector<BoundaryName>>("inlet_boundaries");
  params.suppressParameter<MultiMooseEnum>("momentum_inlet_types");
  params.suppressParameter<std::vector<std::vector<MooseFunctorName>>>("momentum_inlet_functors");
  params.suppressParameter<std::vector<BoundaryName>>("outlet_boundaries");
  params.suppressParameter<MultiMooseEnum>("momentum_outlet_types");
  params.suppressParameter<std::vector<MooseFunctorName>>("pressure_functors");
  params.suppressParameter<std::vector<BoundaryName>>("wall_boundaries");
  params.suppressParameter<MultiMooseEnum>("momentum_wall_types");
  // params.suppressParameter<std::vector<MooseFunctorName>>("momentum_wall_functors");

  // Suppress misc parameters we do not expect we will need for now
  params.suppressParameter<bool>("add_flow_equations");
  params.suppressParameter<PhysicsName>("coupled_turbulence_physics");

  return params;
}

THMWCNSFVFlowPhysics::THMWCNSFVFlowPhysics(const InputParameters & params)
  : PhysicsBase(params), ThermalHydraulicsFlowPhysics(params), WCNSFVFlowPhysics(params)
{
}

void
THMWCNSFVFlowPhysics::initializePhysicsAdditional()
{
  ThermalHydraulicsFlowPhysics::initializePhysicsAdditional();
  WCNSFVFlowPhysics::initializePhysicsAdditional();

  // Move block information from flow_channels to _blocks as WCNSFV routines rely on blocks
  for (const auto flow_channel : _flow_channels)
    addBlocks(flow_channel->getSubdomainNames());
  // TODO: consider other Physics-components
}

void
THMWCNSFVFlowPhysics::actOnAdditionalTasks()
{
  // The THMProblem adds ICs on THM:add_variables, which happens before add_ic
  if (_current_task == "THMPhysics:add_ic")
    addTHMInitialConditions();
}

void
THMWCNSFVFlowPhysics::addNonlinearVariables()
{
  ThermalHydraulicsFlowPhysics::addCommonVariables();

  // TODO: can we use this? it does not use the THM problem does it?
  WCNSFVFlowPhysics::addNonlinearVariables();
}

void
THMWCNSFVFlowPhysics::addAuxiliaryVariables()
{
}

void
THMWCNSFVFlowPhysics::addTHMInitialConditions()
{
  // We are going to assume these are not restarted properly
  ThermalHydraulicsFlowPhysics::addCommonInitialConditions();
}

void
THMWCNSFVFlowPhysics::addInitialConditions()
{
  // Restarting, avoid ICs
  if (_app.isRestarting() || getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  // Use Physics initial conditions only on components on which the initial conditions were not
  // specified
  const auto copy_blocks = blocks();
  for (const auto flow_channel : _flow_channels)
  {
    if (flow_channel->isParamValid("initial_vel") && flow_channel->isParamValid("initial_p"))
    {
      const auto & blocks = flow_channel->getSubdomainNames();

      // Velocity initial condition
      InputParameters params = getFactory().getValidParams("FunctionIC");
      assignBlocks(params, blocks);
      auto vvalue = flow_channel->getParam<FunctionName>("initial_vel");
      params.set<VariableName>("variable") = getVelocityNames()[0];
      params.set<FunctionName>("function") = vvalue;
      getProblem().addInitialCondition("FunctionIC",
                                       prefix() + getVelocityNames()[0] + "_" +
                                           Moose::stringify(blocks) + "_ic",
                                       params);

      // Pressure initial condition
      params.set<VariableName>("variable") = getPressureName();
      params.set<FunctionName>("function") = flow_channel->getParam<FunctionName>("initial_p");
      getProblem().addInitialCondition("FunctionIC",
                                       prefix() + getPressureName() + "_" +
                                           Moose::stringify(blocks) + "_ic",
                                       params);

      // NOTE: this could be a little slow for very many channels. If we specified initial
      // conditions on every single channel, we could skip this
      removeBlocks(flow_channel->getSubdomainNames());
    }
    else if (flow_channel->isParamValid("initial_vel") || flow_channel->isParamValid("initial_p"))
      mooseError(
          "Both or none of 'initial_vel' and 'initial_p' should be specified on flow channel '",
          flow_channel->name() + "'");
  }

  // Add WCNSFV initial conditions on the remaining blocks
  if (_blocks.size())
    WCNSFVFlowPhysics::addInitialConditions();
  // Restore initial block restriction
  _blocks = copy_blocks;
}

void
THMWCNSFVFlowPhysics::addMaterials()
{
  ThermalHydraulicsFlowPhysics::addCommonMaterials();
  WCNSFVFlowPhysics::addMaterials();
}

void
THMWCNSFVFlowPhysics::addFVKernels()
{
  // TODO: process friction factor, gravity

  WCNSFVFlowPhysics::addFVKernels();
}

void
THMWCNSFVFlowPhysics::addAuxiliaryKernels()
{
  // TODO: add aux-variables used by the closures
}

void
THMWCNSFVFlowPhysics::addFVBCs()
{
  // NOTE: This routine will likely move to the derived class if we implement finite volume
  addInletBoundaries();
  addOutletBoundaries();

  WCNSFVFlowPhysics::addFVBCs();
}

void
THMWCNSFVFlowPhysics::addInletBoundaries()
{
  // Fill in the data structures used by WCNSFVFlowPhysics to represent the boundary conditions
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _inlet_types[i];

    if (boundary_type == InletTypeEnum::MdotTemperature)
    {
      MooseEnum flux_mass(NSFVBase::getValidMomentumInletTypes(), "flux-mass");
      MooseFunctorName mdot = std::to_string(comp.getParam<Real>("m_dot"));
      for (const auto & boundary_name : comp.getBoundaryNames())
      {
        addInletBoundary(boundary_name, flux_mass, mdot);
      }
    }
  }
}

void
THMWCNSFVFlowPhysics::addOutletBoundaries()
{
  // Fill in the data structures used by WCNSFVFlowPhysics to represent the boundary conditions
  for (const auto i : index_range(_outlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _outlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _outlet_types[i];
    MooseEnum fixed_pressure(NSFVBase::getValidMomentumOutletTypes(), "fixed-pressure");

    if (boundary_type == OutletTypeEnum::FixedPressure)
    {
      for (const auto & boundary_name : comp.getBoundaryNames())
        addOutletBoundary(boundary_name, fixed_pressure, std::to_string(comp.getParam<Real>("p")));
    }
  }
}

void
THMWCNSFVFlowPhysics::addPostprocessors()
{
  const std::string pp_type = "AverageNodalVariableValue";
  InputParameters params = getFactory().getValidParams(pp_type);
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.set<std::vector<VariableName>>("variable") = {AREA_LINEAR};
  if (!getParam<bool>("output_inlet_areas"))
    params.set<std::vector<OutputName>>("outputs") = {"none"};

  for (const auto i : index_range(_inlet_components))
  {
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    const auto & boundary_type = _inlet_types[i];

    for (const auto & bdy_name : comp.getBoundaryNames())
    {
      params.set<std::vector<BoundaryName>>("boundary") = {bdy_name};

      const auto name_pp = "area_pp_" + bdy_name;
      if (!getProblem().hasUserObject(name_pp))
        getProblem().addPostprocessor(pp_type, name_pp, params);
    }
  }
}
