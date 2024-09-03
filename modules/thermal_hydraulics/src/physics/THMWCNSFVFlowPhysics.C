//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMWCNSFVFlowPhysics.h"
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
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_fv_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_fv_bc");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_material");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_corrector");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_postprocessor");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "get_turbulence_physics");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_aux_variable");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_aux_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "add_user_object");

InputParameters
THMWCNSFVFlowPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();
  params += WCNSFVFlowPhysics::validParams();

  // Suppress direct setting of boundary parameters from the physics, since these will be set by
  // flow boundary components
  params.suppressParameter<std::vector<BoundaryName>>("inlet_boundaries");
  params.suppressParameter<std::vector<MooseEnum>>("momentum_inlet_types");
  params.suppressParameter<std::vector<MooseFunctorName>>("momentum_inlet_functors");
  params.suppressParameter<std::vector<BoundaryName>>("outlet_boundaries");
  params.suppressParameter<std::vector<MooseEnum>>("momentum_outlet_types");
  params.suppressParameter<std::vector<MooseFunctorName>>("pressure_functors");
  params.suppressParameter<std::vector<BoundaryName>>("wall_boundaries");
  params.suppressParameter<std::vector<MooseEnum>>("momentum_wall_types");
  // params.suppressParameter<std::vector<MooseFunctorName>>("momentum_wall_functors");

  // Suppress misc parameters we do not expect we will need
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
  ThermalHydraulicsFlowPhysics::addCommonInitialConditions();
  // TODO: check that they are valid
  WCNSFVFlowPhysics::addInitialConditions();
}

void
THMWCNSFVFlowPhysics::addMaterials()
{
  ThermalHydraulicsFlowPhysics::addCommonMaterials();
  WCNSFVFlowPhysics::addMaterials();
}

void
THMWCNSFVFlowPhysics::addAuxiliaryKernels()
{
}

void
THMWCNSFVFlowPhysics::addUserObjects()
{
  WCNSFVFlowPhysics::addUserObjects();
}

void
THMWCNSFVFlowPhysics::addInletBoundaries()
{
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _inlet_types[i];

    // Boundary fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    if (boundary_type == InletTypeEnum::MdotTemperature)
    {
    }
  }
}

void
THMWCNSFVFlowPhysics::addOutletBoundaries()
{
  for (const auto i : index_range(_outlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _outlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _outlet_types[i];

    // Boundary fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    if (boundary_type == OutletTypeEnum::FixedPressure)
    {
    }
  }
}
