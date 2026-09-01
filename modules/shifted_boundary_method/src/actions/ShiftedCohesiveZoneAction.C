//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShiftedCohesiveZoneAction.h"
#include "ActionWarehouse.h"
#include "AddMaterialAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "NEML2Action.h"
#include "QuasiStaticSolidMechanicsPhysicsBase.h"

#include <algorithm>
#include <set>

namespace
{
bool
getInheritedVolumetricLockingCorrection(ActionWarehouse & awh)
{
  const auto quasi_static_physics = awh.getActions<QuasiStaticSolidMechanicsPhysicsBase>();
  if (quasi_static_physics.empty())
    return false;

  for (const auto * const physics : quasi_static_physics)
  {
    const bool has_locking_param = physics->isParamSetByUser("volumetric_locking_correction");
    if (!has_locking_param)
      continue;

    if (physics->getParam<bool>("volumetric_locking_correction"))
      return true;
  }

  return false;
}

bool
hasOverlap(const std::vector<BoundaryName> & lhs, const std::vector<BoundaryName> & rhs)
{
  for (const auto & left : lhs)
    if (std::find(rhs.begin(), rhs.end(), left) != rhs.end())
      return true;

  return false;
}

bool
producesProperty(const NEML2Action & action, const MaterialPropertyName & property_name)
{
  const auto & outputs = action.getParam<std::vector<std::string>>("moose_outputs");
  if (std::find(outputs.begin(), outputs.end(), property_name) != outputs.end())
    return true;

  const auto & derivatives = action.getParam<std::vector<std::string>>("moose_derivatives");
  return std::find(derivatives.begin(), derivatives.end(), property_name) != derivatives.end();
}

MaterialPropertyName
resolveTransferredProperty(ActionWarehouse & awh, const MaterialPropertyName & property_name)
{
  for (const auto * const material_action : awh.getActions<AddMaterialAction>())
  {
    if (material_action->getMooseObjectType() != "ComputeLagrangianCauchyCustomStress")
      continue;

    const auto & params = material_action->getObjectParams();
    if (property_name == "pk1_stress" || property_name == "stress")
      return params.get<MaterialPropertyName>("custom_cauchy_stress");

    if (property_name == "pk1_jacobian" || property_name == "Jacobian_mult")
      return params.get<MaterialPropertyName>("custom_cauchy_jacobian");
  }

  return property_name;
}
}

// Register the action for various tasks in the MOOSE execution flow
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_interface_kernel");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_material");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_function");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_user_object");
registerMooseAction("ShiftedBoundaryMethodApp",
                    ShiftedCohesiveZoneAction,
                    "add_master_action_material");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_aux_variable");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_aux_kernel");
registerMooseAction("ShiftedBoundaryMethodApp", ShiftedCohesiveZoneAction, "add_kernel");
registerMooseAction("ShiftedBoundaryMethodApp",
                    ShiftedCohesiveZoneAction,
                    "validate_coordinate_systems");

InputParameters
ShiftedCohesiveZoneAction::validParams()
{
  InputParameters params = CohesiveZoneAction::validParams();
  params.addClassDescription("Action to create Shifted Cohesive Zone Method (SCZM) objects "
                             "for non-interface-fitted meshes.");

  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");
  params.addParam<bool>(
      "generate_sbm_distance",
      false,
      "Automatically create surface mesh builders, distance functions, and the boundary distance "
      "user object from the supplied boundaries.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "surface_meshes",
      "Names of saved surface meshes corresponding one-to-one with boundary. If omitted, the "
      "boundary names are used.");
  params.addParam<bool>("check_surface_watertightness",
                        false,
                        "Whether generated surface mesh builders check mesh watertightness.");

  params.addParam<bool>("no_shifted", false, "Disable shifted terms.");
  params.addParam<bool>("directional_correction", true, "Add the directional correction terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property for small strain SCZM.");
  params.addParam<MaterialPropertyName>("tangent",
                                        "Jacobian_mult",
                                        "Name of the material Jacobian tensor property for small "
                                        "strain SCZM.");
  params.addParam<bool>(
      "tangent_wrt_deformation_gradient",
      false,
      "Set true if the provided tangent is already dP/dF (e.g. pk1_jacobian). "
      "If false, tangent is assumed to be strain-based d(stress)/d(strain) (e.g. Jacobian_mult). "
      "Passing a custom dP/dF through ComputeLagrangianCauchyCustomStress still stays on this "
      "strain-based path when the exposed property name here is Jacobian_mult, unless "
      "tangent_wrt_deformation_gradient is set explicitly.");

  params.addParam<bool>("debug_output",
                        false,
                        "Print out stress and neighbor stress for debugging in small strain SCZM.");
  params.addParam<bool>("normal_stress_component",
                        false,
                        "Whether to use the normal component of stress instead of the full vector "
                        "for the directional correction term in small strain SCZM.");
  params.addParam<std::vector<Point>>(
      "junction_pts",
      std::vector<Point>(),
      "Points where projected quadrature points within 'radius' use position-dependent directional "
      "correction scaling.");
  params.addRangeCheckedParam<Real>("radius",
                                    0.0,
                                    "radius >= 0.0",
                                    "Distance threshold for position-dependent directional "
                                    "correction scaling around each junction point.");
  params.addParam<bool>(
      "volumetric_locking_correction",
      false,
      "Whether to apply volume locking to the directional correction term in SCZM.");

  params.addParam<bool>(
      "pk1_to_stress",
      false,
      "Whether to convert PK1 stress to Cauchy stress for the directional correction term "
      "in small strain SCZM");

  params.addParam<bool>(
      "complex_dir_form", false, "Whether to use the complex form of the directional correction");

  params.addParam<Real>("lambda",
                        0.5,
                        "Lambda parameter for the directional correction in small strain SCZM. "
                        "Only used if complex_dir_form=true.");

  params.addParam<Real>("start_directional_correction_time",
                        0.0,
                        "Start time for applying the directional correction term");
  params.addRangeCheckedParam<Real>(
      "directional_correction_ramp_duration",
      0.0,
      "directional_correction_ramp_duration >= 0.0",
      "Duration over which the directional correction term ramps "
      "from 0 to full strength after start_directional_correction_time.");

  return params;
}

ShiftedCohesiveZoneAction::ShiftedCohesiveZoneAction(const InputParameters & params)
  : CohesiveZoneAction(params)
{
  if (getParam<bool>("generate_sbm_distance"))
  {
    if (isParamSetByUser("sbm_distance_uo"))
      paramError("generate_sbm_distance",
                 "Cannot automatically generate the SBM distance objects when 'sbm_distance_uo' "
                 "is also specified.");

    const auto surface_meshes = surfaceMeshNames();
    if (surface_meshes.size() != _boundary.size())
      paramError("surface_meshes",
                 "The number of surface meshes must match the number of boundaries.");

    if (std::set<std::string>(surface_meshes.begin(), surface_meshes.end()).size() !=
        surface_meshes.size())
      paramError("surface_meshes", "Each saved surface mesh may only be used once.");
  }

  switch (_strain)
  {
    case Strain::Small:
    {
      _czm_kernel_name =
          _use_AD ? "ADSCZMInterfaceKernelSmallStrain" : "SCZMInterfaceKernelSmallStrain";
      // For SCZM, we just need to evaulate the displacement jump correctly on the true interface.
      // After that, the local and global traction calculations remain the same as standard CZM.
      _disp_jump_provider_name = _use_AD ? "ADSCZMComputeDisplacementJumpSmallStrain"
                                         : "SCZMComputeDisplacementJumpSmallStrain";
      _equilibrium_traction_calculator_name =
          _use_AD ? "ADCZMComputeGlobalTractionSmallStrain" : "CZMComputeGlobalTractionSmallStrain";
      break;
    }
    case Strain::Finite:
    {
      _czm_kernel_name =
          _use_AD ? "ADSCZMInterfaceKernelTotalLagrangian" : "SCZMInterfaceKernelTotalLagrangian";
      _disp_jump_provider_name = _use_AD ? "ADSCZMComputeDisplacementJumpTotalLagrangian"
                                         : "SCZMComputeDisplacementJumpTotalLagrangian";
      _equilibrium_traction_calculator_name = _use_AD ? "ADCZMComputeGlobalTractionTotalLagrangian"
                                                      : "CZMComputeGlobalTractionTotalLagrangian";
      break;
    }
    default:
      mooseError(
          "ShiftedCohesiveZoneAction Error: Invalid kinematic parameter. Allowed values are: "
          "SmallStrain or TotalLagrangian");
  }
}

void
ShiftedCohesiveZoneAction::act()
{
  // Consistency check: ndisp must match mesh dimension
  if (_ndisp != _mesh->dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_current_task == "add_interface_kernel" && getParam<bool>("directional_correction"))
  {
    std::vector<MaterialPropertyName> stress_names;
    std::vector<MaterialPropertyName> tangent_names;

    if (isParamSetByUser("stress"))
      stress_names.push_back(getParam<MaterialPropertyName>("stress"));
    else if (_strain == Strain::Finite)
      stress_names.push_back("pk1_stress");
    else
    {
      // Mirror SCZMInterfaceKernelSmallStrain, which prefers stress/Jacobian_mult but falls back
      // to pk1_stress/pk1_jacobian for Lagrangian-compatible small-strain materials.
      stress_names.push_back("stress");
      stress_names.push_back("pk1_stress");
    }

    if (isParamSetByUser("tangent"))
      tangent_names.push_back(getParam<MaterialPropertyName>("tangent"));
    else if (_strain == Strain::Finite)
      tangent_names.push_back("pk1_jacobian");
    else
    {
      tangent_names.push_back("Jacobian_mult");
      tangent_names.push_back("pk1_jacobian");
    }

    std::vector<MaterialPropertyName> validated_neml2_properties;

    const auto validate_neml2_interface =
        [this, &validated_neml2_properties](const MaterialPropertyName & property_name)
    {
      const auto neml2_property_name = resolveTransferredProperty(_awh, property_name);

      if (std::find(validated_neml2_properties.begin(),
                    validated_neml2_properties.end(),
                    neml2_property_name) != validated_neml2_properties.end())
        return;

      validated_neml2_properties.push_back(neml2_property_name);

      for (const auto * const neml2_action : _awh.getActions<NEML2Action>())
      {
        if (!producesProperty(*neml2_action, neml2_property_name))
          continue;

        const bool interface_set = neml2_action->isParamSetByUser("interface");
        const auto & interfaces = neml2_action->getParam<std::vector<BoundaryName>>("interface");

        if (!interface_set || interfaces.empty())
          neml2_action->paramError(
              "interface",
              "This NEML2 block provides the material property '",
              neml2_property_name,
              "' used by [Physics/SolidMechanics/ShiftedCohesiveZone] with "
              "directional_correction = true. Set 'interface' to the SCZM boundary (for example, "
              "interface = ${boundary}) or set "
              "'directional_correction = false' in the ShiftedCohesiveZone block.");

        if (!hasOverlap(_boundary, interfaces))
          neml2_action->paramError(
              "interface",
              "This NEML2 block provides the material property '",
              neml2_property_name,
              "' used by "
              "[Physics/SolidMechanics/ShiftedCohesiveZone] with directional_correction = true, "
              "but "
              "its 'interface' setting does not overlap the SCZM boundary. Make the NEML2 "
              "'interface' include the SCZM boundary or set 'directional_correction = false'.");

        return;
      }
    };

    for (const auto & stress_name : stress_names)
      validate_neml2_interface(stress_name);

    for (const auto & tangent_name : tangent_names)
      validate_neml2_interface(tangent_name);
  }

  // Call the base utility for parameter validation across multiple blocks
  chekMultipleActionParameters();

  // Task Routing
  if (_current_task == "add_function" && getParam<bool>("generate_sbm_distance"))
    addSBMDistanceFunctions();
  else if (_current_task == "add_user_object" && getParam<bool>("generate_sbm_distance"))
    addSBMDistanceUserObjects();
  else if (_current_task == "add_interface_kernel")
    addRequiredADSCZMInterfaceKernels();
  else if (_current_task == "add_master_action_material")
    addRequiredSCZMInterfaceMaterials();

  // optional, add required outputs
  actOutputGeneration();
}

std::vector<MeshGeneratorName>
ShiftedCohesiveZoneAction::surfaceMeshNames() const
{
  if (isParamValid("surface_meshes"))
    return getParam<std::vector<MeshGeneratorName>>("surface_meshes");

  std::vector<MeshGeneratorName> surface_meshes;
  surface_meshes.reserve(_boundary.size());
  for (const auto & boundary : _boundary)
    surface_meshes.push_back(boundary);
  return surface_meshes;
}

UserObjectName
ShiftedCohesiveZoneAction::surfaceMeshBuilderName(const BoundaryName & boundary) const
{
  return name() + "_" + boundary + "_builder";
}

FunctionName
ShiftedCohesiveZoneAction::surfaceDistanceFunctionName(const BoundaryName & boundary) const
{
  return name() + "_dist_" + boundary;
}

UserObjectName
ShiftedCohesiveZoneAction::sbmDistanceUserObjectName() const
{
  return name() + "_sbm_distance";
}

void
ShiftedCohesiveZoneAction::addSBMDistanceFunctions()
{
  for (const auto & boundary : _boundary)
  {
    auto params = _factory.getValidParams("UnsignedDistanceToSurfaceMesh");
    params.set<UserObjectName>("builder") = surfaceMeshBuilderName(boundary);
    _problem->addFunction(
        "UnsignedDistanceToSurfaceMesh", surfaceDistanceFunctionName(boundary), params);
  }
}

void
ShiftedCohesiveZoneAction::addSBMDistanceUserObjects()
{
  const auto surface_meshes = surfaceMeshNames();
  std::vector<FunctionName> distance_functions;
  distance_functions.reserve(_boundary.size());

  for (const auto i : index_range(_boundary))
  {
    auto params = _factory.getValidParams("SBMSurfaceMeshBuilder");
    params.set<MeshGeneratorName>("interface_mesh") = surface_meshes[i];
    params.set<bool>("check_watertightness") =
        getParam<bool>("check_surface_watertightness");
    _problem->addUserObject("SBMSurfaceMeshBuilder", surfaceMeshBuilderName(_boundary[i]), params);
    distance_functions.push_back(surfaceDistanceFunctionName(_boundary[i]));
  }

  auto params = _factory.getValidParams("BoundaryShortestDistanceToSurface");
  params.set<std::vector<FunctionName>>("surfaces") = distance_functions;
  params.set<std::vector<BoundaryName>>("boundary") = _boundary;
  params.set<int>("execution_order_group") = 0;
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  _problem->addUserObject(
      "BoundaryShortestDistanceToSurface", sbmDistanceUserObjectName(), params);
}

void
ShiftedCohesiveZoneAction::addRequiredADSCZMInterfaceKernels()
{
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    // Create unique kernel name for each displacement component
    std::string unique_kernel_name = _czm_kernel_name + "_" + _name + "_" + Moose::stringify(i);

    InputParameters paramsk = _factory.getValidParams(_czm_kernel_name);

    paramsk.set<unsigned int>("component") = i;
    paramsk.set<NonlinearVariableName>("variable") = _displacements[i];
    paramsk.set<std::vector<VariableName>>("neighbor_var") = {_displacements[i]};
    paramsk.set<std::vector<VariableName>>("displacements") = _displacements;
    paramsk.set<std::vector<BoundaryName>>("boundary") = _boundary;
    paramsk.set<std::string>("base_name") = _base_name;

    // Set SCZM-specific parameters
    if (getParam<bool>("generate_sbm_distance"))
      paramsk.set<UserObjectName>("sbm_distance_uo") = sbmDistanceUserObjectName();
    else if (isParamSetByUser("sbm_distance_uo"))
      paramsk.set<UserObjectName>("sbm_distance_uo") = getParam<UserObjectName>("sbm_distance_uo");
    if (isParamSetByUser("no_shifted"))
      paramsk.set<bool>("no_shifted") = getParam<bool>("no_shifted");
    if (isParamSetByUser("directional_correction"))
      paramsk.set<bool>("directional_correction") = getParam<bool>("directional_correction");

    if (isParamSetByUser("stress"))
      paramsk.set<MaterialPropertyName>("stress") = getParam<MaterialPropertyName>("stress");
    if (isParamSetByUser("tangent"))
      paramsk.set<MaterialPropertyName>("tangent") = getParam<MaterialPropertyName>("tangent");
    if (isParamSetByUser("tangent_wrt_deformation_gradient"))
      paramsk.set<bool>("tangent_wrt_deformation_gradient") =
          getParam<bool>("tangent_wrt_deformation_gradient");
    if (isParamSetByUser("debug_output"))
      paramsk.set<bool>("debug_output") = getParam<bool>("debug_output");
    const bool user_set_volumetric_locking_correction =
        isParamSetByUser("volumetric_locking_correction");
    const bool inherited_volumetric_locking_correction =
        getInheritedVolumetricLockingCorrection(_awh);
    if (paramsk.isParamValid("volumetric_locking_correction"))
    {
      if (user_set_volumetric_locking_correction)
      {
        paramsk.set<bool>("volumetric_locking_correction") =
            getParam<bool>("volumetric_locking_correction");
        if (getParam<bool>("volumetric_locking_correction") !=
            inherited_volumetric_locking_correction)
          mooseWarning(
              "ShiftedCohesiveZoneAction: 'volumetric_locking_correction' value in "
              "ShiftedCohesiveZone block "
              "is different from that in the physics block(s). Make sure this is intended.");
      }
      else
        paramsk.set<bool>("volumetric_locking_correction") =
            inherited_volumetric_locking_correction;
    }
    if (isParamSetByUser("normal_stress_component") &&
        paramsk.isParamValid("normal_stress_component"))
      paramsk.set<bool>("normal_stress_component") = getParam<bool>("normal_stress_component");
    if (isParamSetByUser("junction_pts") && paramsk.isParamValid("junction_pts"))
      paramsk.set<std::vector<Point>>("junction_pts") =
          getParam<std::vector<Point>>("junction_pts");
    if (isParamSetByUser("radius") && paramsk.isParamValid("radius"))
      paramsk.set<Real>("radius") = getParam<Real>("radius");

    if (isParamSetByUser("pk1_to_stress") && paramsk.isParamValid("pk1_to_stress"))
      paramsk.set<bool>("pk1_to_stress") = getParam<bool>("pk1_to_stress");

    if (isParamSetByUser("complex_dir_form") && paramsk.isParamValid("complex_dir_form"))
      paramsk.set<bool>("complex_dir_form") = getParam<bool>("complex_dir_form");
    if (isParamSetByUser("lambda") && paramsk.isParamValid("lambda"))
      paramsk.set<Real>("lambda") = getParam<Real>("lambda");

    if (isParamSetByUser("start_directional_correction_time") &&
        paramsk.isParamValid("start_directional_correction_time"))
      paramsk.set<Real>("start_directional_correction_time") =
          getParam<Real>("start_directional_correction_time");
    if (isParamSetByUser("directional_correction_ramp_duration") &&
        paramsk.isParamValid("directional_correction_ramp_duration"))
      paramsk.set<Real>("directional_correction_ramp_duration") =
          getParam<Real>("directional_correction_ramp_duration");

    std::string save_in_side;
    std::vector<AuxVariableName> save_in_var_names;
    if (_save_in_primary.size() == _ndisp || _save_in_secondary.size() == _ndisp)
    {
      prepareSaveInInputs(save_in_var_names, save_in_side, _save_in_primary, _save_in_secondary, i);
      paramsk.set<std::vector<AuxVariableName>>("save_in") = save_in_var_names;
      paramsk.set<MultiMooseEnum>("save_in_var_side") = save_in_side;
    }
    if (_diag_save_in_primary.size() == _ndisp || _diag_save_in_secondary.size() == _ndisp)
    {
      prepareSaveInInputs(
          save_in_var_names, save_in_side, _diag_save_in_primary, _diag_save_in_secondary, i);
      paramsk.set<std::vector<AuxVariableName>>("diag_save_in") = save_in_var_names;
      paramsk.set<MultiMooseEnum>("diag_save_in_var_side") = save_in_side;
    }
    _problem->addInterfaceKernel(_czm_kernel_name, unique_kernel_name, paramsk);
  }
}

void
ShiftedCohesiveZoneAction::addRequiredSCZMInterfaceMaterials()
{
  // Add the Shifted Displacement Jump Material
  std::string unique_material_name = _disp_jump_provider_name + "_" + _name;
  InputParameters params_jump = _factory.getValidParams(_disp_jump_provider_name);
  params_jump.set<std::vector<BoundaryName>>("boundary") = _boundary;
  params_jump.set<std::vector<VariableName>>("displacements") = _displacements;
  params_jump.set<std::string>("base_name") = _base_name;

  // Set SCZM-specific parameters
  if (getParam<bool>("generate_sbm_distance"))
    params_jump.set<UserObjectName>("sbm_distance_uo") = sbmDistanceUserObjectName();
  else if (isParamSetByUser("sbm_distance_uo"))
    params_jump.set<UserObjectName>("sbm_distance_uo") =
        getParam<UserObjectName>("sbm_distance_uo");
  if (isParamSetByUser("no_shifted"))
    params_jump.set<bool>("no_shifted") = getParam<bool>("no_shifted");

  _problem->addInterfaceMaterial(_disp_jump_provider_name, unique_material_name, params_jump);

  // Add the Shifted Traction Material (traction calculation remains the same as standard CZM)
  unique_material_name = _equilibrium_traction_calculator_name + "_" + _name;
  InputParameters params_traction = _factory.getValidParams(_equilibrium_traction_calculator_name);
  params_traction.set<std::vector<BoundaryName>>("boundary") = _boundary;
  params_traction.set<std::string>("base_name") = _base_name;
  _problem->addInterfaceMaterial(
      _equilibrium_traction_calculator_name, unique_material_name, params_traction);
}
