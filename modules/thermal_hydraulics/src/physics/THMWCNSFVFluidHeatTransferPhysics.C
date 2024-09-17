//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMWCNSFVFluidHeatTransferPhysics.h"
#include "NSFVBase.h"
#include "THMProblem.h"

// To implement component-specific behavior
#include "PhysicsFlowChannel.h"
#include "PhysicsFlowBoundary.h"
#include "PhysicsHeatTransferBase.h"

registerTHMFlowModelPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics);
registerNavierStokesPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics);
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "THMPhysics:add_ic");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_ic");

// From WCNSFVFluidHeatTransferPhysics
// TODO: make sure list is minimal
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_fv_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_fv_bc");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_material");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_corrector");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_postprocessor");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_aux_variable");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_aux_kernel");
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFluidHeatTransferPhysics, "add_user_object");

InputParameters
THMWCNSFVFluidHeatTransferPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();
  params += WCNSFVFluidHeatTransferPhysics::validParams();

  // Suppress direct setting of boundary parameters from the physics, since these will be set by
  // flow boundary components
  params.suppressParameter<MultiMooseEnum>("energy_inlet_types");
  params.suppressParameter<std::vector<MooseFunctorName>>("energy_inlet_functors");
  params.suppressParameter<MultiMooseEnum>("energy_wall_types");
  params.suppressParameter<std::vector<MooseFunctorName>>("energy_wall_functors");

  // Suppress misc parameters we do not expect we will need for now
  params.suppressParameter<bool>("add_energy_equation");

  return params;
}

THMWCNSFVFluidHeatTransferPhysics::THMWCNSFVFluidHeatTransferPhysics(const InputParameters & params)
  : PhysicsBase(params),
    ThermalHydraulicsFlowPhysics(params),
    WCNSFVFluidHeatTransferPhysics(params)
{
}

void
THMWCNSFVFluidHeatTransferPhysics::initializePhysicsAdditional()
{
  ThermalHydraulicsFlowPhysics::initializePhysicsAdditional();
  WCNSFVFluidHeatTransferPhysics::initializePhysicsAdditional();

  // Move block information from flow_channels to _blocks as WCNSFV routines rely on blocks
  for (const auto flow_channel : _flow_channels)
    addBlocks(flow_channel->getSubdomainNames());
  // TODO: consider other Physics-components

  // Delete ANY_BLOCK_ID from the Physics block restriction
  // TODO: never add it in the first place?
  _blocks.erase(std::remove(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID"), _blocks.end());
}

void
THMWCNSFVFluidHeatTransferPhysics::actOnAdditionalTasks()
{
}

void
THMWCNSFVFluidHeatTransferPhysics::addNonlinearVariables()
{
  // Use this for pressure only. Since we are using functors for the velocity variables
  WCNSFVFluidHeatTransferPhysics::addNonlinearVariables();
}

void
THMWCNSFVFluidHeatTransferPhysics::addAuxiliaryVariables()
{
}

void
THMWCNSFVFluidHeatTransferPhysics::addInitialConditions()
{
  // Restarting, avoid ICs
  if (_app.isRestarting() || getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  // Use Physics initial conditions only on components on which the initial conditions were not
  // specified
  const auto copy_blocks = blocks();
  for (const auto flow_channel : _flow_channels)
  {
    if (flow_channel->isParamValid("initial_T"))
    {
      const auto & blocks = flow_channel->getSubdomainNames();

      // Temperature initial condition
      InputParameters params = getFactory().getValidParams("FunctionIC");
      assignBlocks(params, blocks);
      params.set<VariableName>("variable") = getFluidTemperatureName();
      params.set<FunctionName>("function") = flow_channel->getParam<FunctionName>("initial_T");
      getProblem().addInitialCondition("FunctionIC",
                                       prefix() + getFluidTemperatureName() + "_" +
                                           Moose::stringify(blocks) + "_ic",
                                       params);

      // NOTE: this could be a little slow for very many channels. If we specified initial
      // conditions on every single channel, we could skip this
      removeBlocks(flow_channel->getSubdomainNames());
    }
  }

  // Add WCNSFV initial conditions on the remaining blocks
  if (_blocks.size() && std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") == _blocks.end())
    WCNSFVFluidHeatTransferPhysics::addInitialConditions();

  // Restore initial block restriction
  _blocks = copy_blocks;
}

void
THMWCNSFVFluidHeatTransferPhysics::addMaterials()
{
  WCNSFVFluidHeatTransferPhysics::addMaterials();
  addJunctionFunctorMaterials();
  addHeatTransferFunctorMaterials();
}

void
THMWCNSFVFluidHeatTransferPhysics::addJunctionFunctorMaterials()
{
  if (_verbose)
    _console << "Adding junction functor materials for junctions: "
             << Moose::stringify(_junction_components) << std::endl;
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];
    const auto & boundary_names = comp.getBoundaryNames();

    // Add functor materials to compute the boundary values
    if (junction_type == JunctionTypeEnum::OneToOne)
    {
      std::string class_name = "ADFixedNodeValueFunctorMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

      // Get temperature at outlet of the first connected component
      params.set<MooseFunctorName>("functor_in") = getFluidTemperatureName();
      params.set<MooseFunctorName>("functor_name") = "face_value_T_" + boundary_names[0];
      params.set<BoundaryName>("nodeset") = boundary_names[0];
      params.set<SubdomainName>("subdomain_for_node") = comp.getConnectedComponentNames()[0];
      _sim->addFunctorMaterial(class_name, "compute_face_value_T_" + boundary_names[0], params);
    }
  }
}

void
THMWCNSFVFluidHeatTransferPhysics::addHeatTransferFunctorMaterials()
{
  for (const auto & [comp_name, heat_transfer_type] : _heat_transfer_types)
  {
    const auto & comp = _sim->getComponentByName<PhysicsHeatTransferBase>(comp_name);
    // Factor to convert a surface term to a volumetric term
    {
      const std::string class_name = "VolumeToAreaFunctorMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<MooseFunctorName>("functor_name") = "vol_surf_factor_" + comp_name;
      params.set<MooseFunctorName>("area") = "A";
      _sim->addFunctorMaterial(class_name, genName(comp.name(), "conversion_area_volume"), params);
    }

    // Convert wall temperature to a heat flux
    if (heat_transfer_type == FixedWallTemperature)
    {
      const std::string class_name = "ADParsedFunctorMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<std::string>("property_name") = comp_name + "_q_wall";
      // How to set T_wall is decided by the HeatTransfer object, Hw is set by a correlation
      params.set<std::string>("expression") = "(T_wall - T_fluid) * Hw";
      params.set<std::vector<std::string>>("functor_names") = {"T_wall", "Hw", "T_fluid"};

      _sim->addFunctorMaterial(class_name, genName(comp.name(), "q_wall"), params);
    }
  }
}

void
THMWCNSFVFluidHeatTransferPhysics::addFVKernels()
{
  addHeatTransferKernels();
  WCNSFVFluidHeatTransferPhysics::addFVKernels();
}

void
THMWCNSFVFluidHeatTransferPhysics::addAuxiliaryKernels()
{
  // TODO: add aux-variables used by the closures
}

void
THMWCNSFVFluidHeatTransferPhysics::addFVBCs()
{
  // NOTE: This routine will likely move to the derived class if we implement finite volume
  addInletBoundaries();
  addOutletBoundaries();
  addFlowJunctions();

  WCNSFVFluidHeatTransferPhysics::addFVBCs();
}

void
THMWCNSFVFluidHeatTransferPhysics::addInletBoundaries()
{
  if (_verbose)
    _console << "Adding boundary conditions for inlets: " << Moose::stringify(_inlet_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVFluidHeatTransferPhysics to represent the boundary
  // conditions
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _inlet_types[i];

    if (boundary_type == InletTypeEnum::MdotTemperature)
    {
      MooseEnum inlet_type(NSFVBase::getValidEnergyInletTypes(), "flux-mass");
      MooseFunctorName inlet_T = std::to_string(comp.getParam<Real>("T"));
      for (const auto & boundary_name : comp.getBoundaryNames())
        addInletBoundary(boundary_name, inlet_type, inlet_T);
    }
    else
      mooseError("Unsupported inlet boundary type ", boundary_type);
  }
}

void
THMWCNSFVFluidHeatTransferPhysics::addOutletBoundaries()
{
  // Nothing to do for temperature
}

void
THMWCNSFVFluidHeatTransferPhysics::addFlowJunctions()
{
  if (_verbose)
    _console << "Adding junction objects for junctions: " << Moose::stringify(_junction_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVFluidHeatTransferPhysics to represent the boundary
  // conditions
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];
    const auto & boundary_names = comp.getBoundaryNames();

    if (junction_type == JunctionTypeEnum::OneToOne)
    {
      // The ideal junction would be to merge the nodes
      // However, the two pipes might not be at the same location.
      // We make the temperature functors match using dirichlet BCs
      for (const auto bi : index_range(boundary_names))
      {
        // The first boundary is skipped
        if (bi == 0)
          continue;

        MooseEnum fixed_temperature(NSFVBase::getValidEnergyInletTypes(), "fixed-temperature");
        addInletBoundary(boundary_names[1], fixed_temperature, "face_value_T_" + boundary_names[0]);
      }
    }
    else
      mooseError("Unsupported junction type ", junction_type);
  }
}

void
THMWCNSFVFluidHeatTransferPhysics::addWallHeatFlux(const std::string & heat_transfer_component,
                                                   const HeatFluxWallEnum & heat_flux_type)
{
  _heat_transfer_types[heat_transfer_component] = heat_flux_type;
}

void
THMWCNSFVFluidHeatTransferPhysics::addHeatTransferKernels()
{
  for (const auto & [heat_transfer_component, heat_flux_type] : _heat_transfer_types)
  {
    mooseAssert(_sim, "Should have a problem");
    const auto & component = _sim->getComponentByName<HeatTransferBase>(heat_transfer_component);
    const auto volume_surface_adjustment = "vol_surf_factor_" + heat_transfer_component;

    if (heat_flux_type == ThermalHydraulicsFlowPhysics::FixedWallTemperature)
    {
      // Wall temperature has to be converted to a heat flux functor
      for (const auto & block : component.getFlowChannelSubdomains())
        addExternalHeatSource(
            block, heat_transfer_component + "_q_wall", volume_surface_adjustment);
    }
    else if (heat_flux_type == THMWCNSFVFluidHeatTransferPhysics::FixedHeatFlux)
    {
      for (const auto & block : component.getFlowChannelSubdomains())
        addExternalHeatSource(block, component.getWallHeatFluxName(), volume_surface_adjustment);
    }
    else
      mooseAssert(false, "Heat flux type not implemented");
  }
}

void
THMWCNSFVFluidHeatTransferPhysics::addPostprocessors()
{
}
