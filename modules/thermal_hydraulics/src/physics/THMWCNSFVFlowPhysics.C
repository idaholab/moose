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
#include "THMMesh.h"

#include "Function.h"

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
registerMooseAction("ThermalHydraulicsApp", THMWCNSFVFlowPhysics, "THMPhysics:change_1D_mesh_info");

InputParameters
THMWCNSFVFlowPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();
  params += WCNSFVFlowPhysics::validParams();

  params.addParam<bool>("output_inlet_areas",
                        false,
                        "Whether to output the postprocessors measuring the inlet areas");

  // Prepare to forward velocity components to the main velocity variable
  // params.set<std::vector<std::string>>("velocity_variable") = {
  //     "vel_x_channel", "vel_y_channel", "vel_z_channel"};

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

  // Delete ANY_BLOCK_ID from the Physics block restriction
  // TODO: never add it in the first place?
  _blocks.erase(std::remove(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID"), _blocks.end());
}

void
THMWCNSFVFlowPhysics::actOnAdditionalTasks()
{
  // The THMProblem adds ICs on THM:add_variables, which happens before add_ic
  if (_current_task == "THMPhysics:add_ic")
    addTHMInitialConditions();
  else if (_current_task == "THMPhysics:change_1D_mesh_info")
    changeMeshFaceAndElemInfo();
}

void
THMWCNSFVFlowPhysics::addNonlinearVariables()
{
  ThermalHydraulicsFlowPhysics::addCommonVariables();

  // Add the channel velocity variable
  // {
  //   const auto variable_type = "INSFVVelocityVariable";
  //   auto params = getFactory().getValidParams(variable_type);
  //   assignBlocks(params, _blocks);
  //   params.set<std::vector<Real>>("scaling") = {getParam<Real>("momentum_scaling")};
  //   params.set<MooseEnum>("face_interp_method") =
  //       getParam<MooseEnum>("momentum_face_interpolation");
  //   params.set<bool>("two_term_boundary_expansion") =
  //       getParam<bool>("momentum_two_term_bc_expansion");
  //   getProblem().addVariable(variable_type, "vel_1d", params);
  //   saveNonlinearVariableName("vel_1d");
  // }

  // Add functors that forward to the nonlinear variables
  // {
  //   std::vector<std::string> velocity_name = {"vel_x_channel", "vel_y_channel", "vel_z_channel"};
  //   std::vector<std::string> channel_direction = {"direction_x", "direction_y", "direction_z"};

  //   std::string class_name = "ADParsedFunctorMaterial";
  //   InputParameters params = _factory.getValidParams(class_name);
  //   assignBlocks(params, _blocks);
  //   params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  //   for (const auto d : make_range(dimension()))
  //   {
  //     params.set<std::string>("property_name") = velocity_name[d];
  //     params.set<std::vector<std::string>>("functor_names") = {"vel_1d ", channel_direction[d]};
  //     params.set<std::string>("expression") = "vel_1d * " + channel_direction[d];
  //     _sim->addFunctorMaterial(class_name, "compute_" + velocity_name[d], params);
  //   }
  // }

  // Use this for pressure only. Since we are using functors for the velocity variables
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
  if (_blocks.size() && std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") == _blocks.end())
    WCNSFVFlowPhysics::addInitialConditions();

  // Restore initial block restriction
  _blocks = copy_blocks;
}

void
THMWCNSFVFlowPhysics::addMaterials()
{
  ThermalHydraulicsFlowPhysics::addCommonMaterials();
  addChannelFrictionRegions();
  WCNSFVFlowPhysics::addMaterials();

  addDirectionFunctorMaterial();
  addJunctionFunctorMaterials();
}

void
THMWCNSFVFlowPhysics::addDirectionFunctorMaterial()
{
  std::string class_name = "GenericVectorFunctorMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  assignBlocks(params, _blocks);
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Get velocity at outlet of the first connected component
  params.set<std::vector<std::string>>("prop_names") = {"direction_vec"};
  params.set<std::vector<MooseFunctorName>>("prop_values") = {
      "direction_x", "direction_y", "direction_z"};
  _sim->addFunctorMaterial(class_name, prefix() + "channel_direction_vector", params);
}

void
THMWCNSFVFlowPhysics::addJunctionFunctorMaterials()
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

      // Get velocity at outlet of the first connected component
      params.set<MooseFunctorName>("functor_in") = getVelocityNames()[0];
      params.set<MooseFunctorName>("functor_name") = "face_value_v_" + boundary_names[0];
      params.set<BoundaryName>("nodeset") = boundary_names[0];
      params.set<SubdomainName>("subdomain_for_node") = comp.getConnectedComponentNames()[0];
      _sim->addFunctorMaterial(class_name, "compute_face_value_v_" + boundary_names[0], params);

      // Get pressure on inlet of second connected component
      params.set<MooseFunctorName>("functor_in") = getPressureName();
      params.set<BoundaryName>("nodeset") = boundary_names[1];
      params.set<SubdomainName>("subdomain_for_node") = comp.getConnectedComponentNames()[1];
      params.set<MooseFunctorName>("functor_name") = "face_value_p_" + boundary_names[1];
      _sim->addFunctorMaterial(class_name, "compute_face_value_p_" + boundary_names[1], params);
    }
  }
}

void
THMWCNSFVFlowPhysics::addChannelFrictionRegions()
{
  // Process friction factor input
  for (const auto flow_channel : _flow_channels)
  {
    const auto & blocks = flow_channel->getSubdomainNames();
    if (flow_channel->isParamValid("f"))
    {
      const std::string f_name = flow_channel->getParam<FunctionName>("f");
      // we ll only support isotropic friction in 1D for now
      addFrictionRegion(blocks, {"Darcy"}, {f_name});
      // TODO Remove this code and use the closure instead
    }
  }
}

void
THMWCNSFVFlowPhysics::addFVKernels()
{
  // Process channel orientation and gravity vector input
  for (const auto flow_channel : _flow_channels)
  {
    const auto & blocks = flow_channel->getSubdomainNames();
    const auto & physics_gravity = getParam<RealVectorValue>("gravity");
    if (flow_channel->isParamValid("gravity_vector"))
    {
      const auto & local_gravity = flow_channel->getParam<RealVectorValue>("gravity_vector");
      for (const auto & block : blocks)
        _gravity_vector_map[block] = local_gravity;
    }
    else
      for (const auto & block : blocks)
        _gravity_vector_map[block] = physics_gravity;

    if (flow_channel->isParamValid("orientation"))
    {
      const auto & local_orientation = flow_channel->getParam<RealVectorValue>("orientation");
      for (const auto & block : blocks)
        _flow_channel_orientation_map[block] = local_orientation;
    }
  }

  WCNSFVFlowPhysics::addFVKernels();
}

RealVectorValue
THMWCNSFVFlowPhysics::getLocalGravityVector(const SubdomainName & block) const
{
  // Gravity kernel will access the first component of gravity vector, since we use vel_x for the
  // variable
  return RealVectorValue({libmesh_map_find(_flow_channel_orientation_map, block) *
                              libmesh_map_find(_gravity_vector_map, block),
                          0,
                          0});
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
  addFlowJunctions();

  WCNSFVFlowPhysics::addFVBCs();
}

void
THMWCNSFVFlowPhysics::addInletBoundaries()
{
  if (_verbose)
    _console << "Adding boundary conditions for inlets: " << Moose::stringify(_inlet_components)
             << std::endl;
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
    else
      mooseError("Unsupported inlet boundary type ", boundary_type);
  }
}

void
THMWCNSFVFlowPhysics::addOutletBoundaries()
{
  if (_verbose)
    _console << "Adding boundary conditions for outlets: " << Moose::stringify(_outlet_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVFlowPhysics to represent the boundary conditions
  for (const auto i : index_range(_outlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _outlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    const auto & boundary_type = _outlet_types[i];

    if (boundary_type == OutletTypeEnum::FixedPressure)
    {
      MooseEnum fixed_pressure(NSFVBase::getValidMomentumOutletTypes(), "fixed-pressure");
      for (const auto & boundary_name : comp.getBoundaryNames())
        addOutletBoundary(boundary_name, fixed_pressure, std::to_string(comp.getParam<Real>("p")));
    }
    else if (boundary_type == OutletTypeEnum::FreeBoundary)
    {
      MooseEnum free_boundary(NSFVBase::getValidMomentumOutletTypes(), "zero-gradient");
      for (const auto & boundary_name : comp.getBoundaryNames())
        addOutletBoundary(boundary_name, free_boundary, "");
    }
    else
      mooseError("Unsupported outlet boundary type ", boundary_type);
  }
}

void
THMWCNSFVFlowPhysics::addFlowJunctions()
{
  if (_verbose)
    _console << "Adding junction objects for junctions: " << Moose::stringify(_junction_components)
             << std::endl;
  // Fill in the data structures used by WCNSFVFlowPhysics to represent the boundary conditions
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
      // We make the pressure & velocity functors match using dirichlet BCs
      for (const auto bi : index_range(boundary_names))
      {
        // The first boundary is skipped
        if (bi == 0)
          continue;

        MooseEnum fixed_velocity(NSFVBase::getValidMomentumInletTypes(), "fixed-velocity");
        MooseEnum fixed_pressure(NSFVBase::getValidMomentumOutletTypes(), "fixed-pressure");
        addInletBoundary(boundary_names[1], fixed_velocity, "face_value_v_" + boundary_names[0]);
        addOutletBoundary(boundary_names[0], fixed_pressure, "face_value_p_" + boundary_names[1]);
      }
    }
    else
      mooseError("Unsupported junction type ", junction_type);
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

    for (const auto & bdy_name : comp.getBoundaryNames())
    {
      params.set<std::vector<BoundaryName>>("boundary") = {bdy_name};

      const auto name_pp = "area_pp_" + bdy_name;
      if (!getProblem().hasUserObject(name_pp))
        getProblem().addPostprocessor(pp_type, name_pp, params);
    }
  }
}

void
THMWCNSFVFlowPhysics::changeMeshFaceAndElemInfo()
{
  // NOTE: an alternative to messing with the mesh like this would be to use
  // custom porosity functors. This works so I will keep it around. It would
  // also be helpful when debugging the porosity functors

  // Note: we change here
  // - the flux size on every face for FVFluxBC / Kernels (by changing the area)
  // - the gradient orientations! grad_P is now always aligned with 'x'
  // - the velocity * normal product on outer boundaries for outflow BCs. Always aligned.

  // Need to work on THM mesh (do I?)
  auto & thm_mesh = static_cast<THMMesh &>(_sim->mesh());

  // Loop on flow channels
  for (const auto flow_channel : _flow_channels)
  {
    const auto & blocks = flow_channel->getSubdomainNames();
    const auto & channel_area =
        getProblem().getFunction(flow_channel->getParam<FunctionName>("A"), 0);
    const auto & channel_orientation = flow_channel->getParam<RealVectorValue>("orientation");

    for (const auto & block : blocks)
    {
      const auto block_id = thm_mesh.getSubdomainID(block);

      // Loop on elements in the flow channel
      for (const auto elem : thm_mesh.getMesh().active_local_subdomain_elements_ptr_range(block_id))
      {
        const auto centroid = elem->true_centroid();
        // Fix element info
        ElemInfo & elem_info = const_cast<ElemInfo &>(thm_mesh.elemInfo(elem->id()));
        elem_info.volumeRef() = channel_area.value(0, centroid) * elem->hmax();

        // Fix centroid
        elem_info.centroid() = elem_info.centroid().norm() * RealVectorValue(1, 0, 0);

        // Fix face infos attached to the element
        for (const auto side : elem->side_index_range())
        {
          FaceInfo * fi = const_cast<FaceInfo *>(thm_mesh.faceInfo(elem, side));

          if (!fi)
            continue;

          // Channel area for all faces along the channel
          fi->faceArea() = channel_area.value(0, centroid);
          // Channel is always aligned with vel_x as vel_x follows the channel
          // Distinguish between faces aligned with the orientation and not
          if (fi->normal() * channel_orientation > 0)
            fi->normal() = {1, 0, 0};
          else
            fi->normal() = {-1, 0, 0};

          // Change all other quantities (more hacking)
          fi->dCN() = fi->dCN().norm() * fi->normal();
          fi->eCN() = fi->normal();

          // Note:
          // we did not change any of the centroids, neither element nor face.
          fi->faceCentroid() = fi->faceCentroid().norm() * RealVectorValue(1, 0, 0);
        }
      }
    }
  }
}
