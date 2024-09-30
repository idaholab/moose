//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMWCNSFVScalarTransportPhysics.h"
#include "NSFVBase.h"
#include "THMProblem.h"

// To implement component-specific behavior
#include "PhysicsFlowChannel.h"
#include "PhysicsFlowBoundary.h"
#include "PhysicsHeatTransferBase.h"
#include "ScalarTransferBase.h"

registerTHMFlowModelPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVScalarTransportPhysics);
registerNavierStokesPhysicsBaseTasks("ThermalHydraulicsApp", THMWCNSFVScalarTransportPhysics);
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVScalarTransportPhysics, "THMPhysics:add_ic");
registerWCNSFVScalarTransportBaseTasks("ThermalHydraulicsApp", THMWCNSFVScalarTransportPhysics);

InputParameters
THMWCNSFVScalarTransportPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();
  params += WCNSFVScalarTransportPhysics::validParams();

  // Suppress direct setting of boundary parameters from the physics, since these will be set by
  // flow boundary components
  params.suppressParameter<MultiMooseEnum>("passive_scalar_inlet_types");
  params.suppressParameter<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_inlet_functors");

  // Suppress misc parameters we do not expect we will need for now
  params.suppressParameter<bool>("add_scalar_equation");

  return params;
}

THMWCNSFVScalarTransportPhysics::THMWCNSFVScalarTransportPhysics(const InputParameters & params)
  : PhysicsBase(params), ThermalHydraulicsFlowPhysics(params), WCNSFVScalarTransportPhysics(params)
{
}

void
THMWCNSFVScalarTransportPhysics::initializePhysicsAdditional()
{
  ThermalHydraulicsFlowPhysics::initializePhysicsAdditional();
  WCNSFVScalarTransportPhysics::initializePhysicsAdditional();

  // Move block information from flow_channels to _blocks as WCNSFV routines rely on blocks
  for (const auto flow_channel : _flow_channels)
    addBlocks(flow_channel->getSubdomainNames());
  // TODO: consider other Physics-components

  // Delete ANY_BLOCK_ID from the Physics block restriction
  // TODO: never add it in the first place?
  _blocks.erase(std::remove(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID"), _blocks.end());
}

void
THMWCNSFVScalarTransportPhysics::actOnAdditionalTasks()
{
}

void
THMWCNSFVScalarTransportPhysics::addNonlinearVariables()
{
  // Use this for pressure only. Since we are using functors for the velocity variables
  WCNSFVScalarTransportPhysics::addNonlinearVariables();
}

void
THMWCNSFVScalarTransportPhysics::addInitialConditions()
{
  // Restarting, avoid ICs
  if (_app.isRestarting() || getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  // Save initial block restriction
  const auto copy_blocks = blocks();

  for (const auto i : index_range(_passive_scalar_names))
  {
    const auto scalar_name = _passive_scalar_names[i];

    // Use Physics initial conditions only on components on which the initial conditions were not
    // specified
    for (const auto flow_channel : _flow_channels)
    {
      const auto & scalar_ics =
          flow_channel->getParam<std::vector<FunctionName>>("initial_scalars");
      if (scalar_ics.size())
      {
        const auto & blocks = flow_channel->getSubdomainNames();

        // Temperature initial condition
        InputParameters params = getFactory().getValidParams("FunctionIC");
        assignBlocks(params, blocks);
        params.set<VariableName>("variable") = scalar_name;
        params.set<FunctionName>("function") = scalar_ics[i];
        getProblem().addInitialCondition(
            "FunctionIC", prefix() + scalar_name + "_" + flow_channel->name() + "_ic", params);

        // NOTE: this could be a little slow for very many channels. If we specified initial
        // conditions on every single channel, we could skip this
        if (i == 0)
          removeBlocks(flow_channel->getSubdomainNames());
        else if (hasBlocks(flow_channel->getSubdomainNames()))
          mooseError("Initial conditions should be specified for every scalar. Scalar " +
                     scalar_name + " has no initial condition on channel " + flow_channel->name());
      }
    }
  }

  // Add WCNSFV initial conditions on the remaining blocks
  if (_blocks.size() && std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") == _blocks.end())
    WCNSFVScalarTransportPhysics::addInitialConditions();

  // Restore initial block restriction
  _blocks = copy_blocks;
}

void
THMWCNSFVScalarTransportPhysics::addMaterials()
{
  addJunctionFunctorMaterials();
  addScalarTransferFunctorMaterials();
}

void
THMWCNSFVScalarTransportPhysics::addJunctionFunctorMaterials()
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

      params.set<BoundaryName>("nodeset") = boundary_names[0];
      params.set<SubdomainName>("subdomain_for_node") = comp.getConnectedComponentNames()[0];
      for (const auto & scalar_name : _passive_scalar_names)
      {
        params.set<MooseFunctorName>("functor_in") = scalar_name;
        params.set<MooseFunctorName>("functor_name") =
            "face_value_" + scalar_name + "_" + boundary_names[0];
        _sim->addFunctorMaterial(
            class_name, "compute_face_value_" + scalar_name + "_" + boundary_names[0], params);
      }
    }
  }
}

void
THMWCNSFVScalarTransportPhysics::addScalarTransferFunctorMaterials()
{
  for (const auto & [comp_name, heat_transfer_type] : _scalar_transfer_types)
  {
    const auto & comp = _sim->getComponentByName<ScalarTransferBase>(comp_name);
    // Factor to convert a surface term to a volumetric term
    if (!_sim->hasFunctor("vol_surf_factor_" + comp_name, 0))
    {
      const std::string class_name = "VolumeToAreaFunctorMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<MooseFunctorName>("functor_name") = "vol_surf_factor_" + comp_name;
      params.set<MooseFunctorName>("area") = "A";
      _sim->addFunctorMaterial(class_name, genName(comp.name(), "conversion_area_volume"), params);
      params.set<MooseFunctorName>("functor_name") = "minus_vol_surf_factor_" + comp_name;
      params.set<MooseFunctorName>("coef") = "-1";
      _sim->addFunctorMaterial(class_name, genName(comp.name(), "minus_conversion_area_volume"), params);
    }
  }
}

void
THMWCNSFVScalarTransportPhysics::addFVKernels()
{
  addScalarTransferKernels();
  WCNSFVScalarTransportPhysics::addFVKernels();
}

void
THMWCNSFVScalarTransportPhysics::addFVBCs()
{
  // NOTE: This routine will likely move to the derived class if we implement finite volume
  addInletBoundaries();
  addOutletBoundaries();
  addFlowJunctions();

  WCNSFVScalarTransportPhysics::addFVBCs();
}

void
THMWCNSFVScalarTransportPhysics::addInletBoundaries()
{
  if (_verbose)
    _console << "Adding boundary conditions for inlets: " << Moose::stringify(_inlet_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVScalarTransportPhysics to represent the boundary
  // conditions
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _inlet_types[i];

    if (boundary_type == InletTypeEnum::GeneralBoundary)
    {
      MooseEnum inlet_type(NSFVBase::getValidScalarInletTypes(), "flux-mass");
      for (const auto scalar_i : index_range(_passive_scalar_names))
      {
        const auto & scalar_name = _passive_scalar_names[scalar_i];
        if (comp.isValueBoundary(scalar_name))
        {
          MooseFunctorName inlet_flux = comp.getBoundaryValue(scalar_name);
          for (const auto & boundary_name : comp.getBoundaryNames())
            addInletBoundary(boundary_name, inlet_type, inlet_flux, scalar_i);
        }
        else
          mooseError("Flux boundary not supported for variable", scalar_name);
      }
    }
    else
      mooseError("Unsupported inlet boundary type ", boundary_type);
  }
}

void
THMWCNSFVScalarTransportPhysics::addOutletBoundaries()
{
  // Nothing to do for scalar quantities
  // at least until we implement reversible boundaries
}

void
THMWCNSFVScalarTransportPhysics::addFlowJunctions()
{
  if (_verbose)
    _console << "Adding junction objects for junctions: " << Moose::stringify(_junction_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVScalarTransportPhysics to represent the boundary
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

        MooseEnum fixed_value(NSFVBase::getValidScalarInletTypes(), "fixed-value");
        for (const auto & scalar_i : index_range(_passive_scalar_names))
        {
          const auto & scalar_name = _passive_scalar_names[scalar_i];
          addInletBoundary(boundary_names[1],
                           fixed_value,
                           "face_value_" + scalar_name + "_" + boundary_names[0],
                           scalar_i);
        }
      }
    }
    else
      mooseError("Unsupported junction type ", junction_type);
  }
}

void
THMWCNSFVScalarTransportPhysics::addWallScalarFlux(const std::string & scalar_transfer_component,
                                                   const ScalarFluxWallEnum & scalar_flux_type)
{
  _scalar_transfer_types[scalar_transfer_component] = scalar_flux_type;
}

void
THMWCNSFVScalarTransportPhysics::addScalarTransferKernels()
{
  for (const auto & [scalar_transfer_component, scalar_flux_type] : _scalar_transfer_types)
  {
    mooseAssert(_sim, "Should have a problem");

    const auto & component =
        _sim->getComponentByName<ScalarTransferBase>(scalar_transfer_component);
    const auto volume_surface_adjustment = "vol_surf_factor_" + scalar_transfer_component;

    if (scalar_flux_type == THMWCNSFVScalarTransportPhysics::FixedScalarValue)
    {
      addExternalScalarSources(
          component.getFlowChannelSubdomains(),
          component.getWallScalarValuesNames(),
          std::vector<MooseFunctorName>(_passive_scalar_names.size(), volume_surface_adjustment));
      // This is inefficient, but for 1D Physics we should be fine
      std::vector<MooseFunctorName> _passive_scalar_names_functors;
      for (const auto & scalar_name : _passive_scalar_names)
        _passive_scalar_names_functors.push_back(scalar_name);
      addExternalScalarSources(component.getFlowChannelSubdomains(),
                               _passive_scalar_names_functors,
                               std::vector<MooseFunctorName>(_passive_scalar_names.size(),
                                                             "minus_" + volume_surface_adjustment));
    }
    else if (scalar_flux_type == THMWCNSFVScalarTransportPhysics::FixedScalarFlux)
    {
      addExternalScalarSources(
          component.getFlowChannelSubdomains(),
          component.getWallScalarFluxNames(),
          std::vector<MooseFunctorName>(_passive_scalar_names.size(), volume_surface_adjustment));
    }
    else
      mooseAssert(false, "Flux type not implemented");
  }
}
