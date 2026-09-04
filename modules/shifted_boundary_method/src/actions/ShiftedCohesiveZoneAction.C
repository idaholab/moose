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
  params.addParam<MeshGeneratorName>(
      "complete_interface_mesh",
      "Saved surface mesh containing the complete boundary of each subdomain. Setting this "
      "parameter automatically creates one interface manager and distance user object.");
  params.addParam<std::vector<std::vector<SubdomainID>>>(
      "interface_subdomain_pairs",
      {},
      "Ordered subdomain pair for each boundary when it cannot be inferred from boundary names.");
  params.addParam<Real>("interface_matching_tolerance",
                        libMesh::TOLERANCE,
                        "Relative face-matching tolerance for the single-mesh interface manager.");

  params.addParam<bool>("no_shifted", false, "Disable shifted terms.");
  params.addParam<bool>("directional_correction", true, "Add the directional correction terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property for small strain SCZM.");
  params.addParam<MaterialPropertyName>("tangent",
                                        "Jacobian_mult",
                                        "Name of the material Jacobian tensor property for small "
                                        "strain SCZM.");
  params.addParam<MooseEnum>(
      "tangent_definition",
      MooseEnum("auto stress_wrt_strain pk1_wrt_deformation_gradient", "auto"),
      "Mathematical definition of the tangent material property. 'auto' recognizes the standard "
      "Jacobian_mult property as d(stress)/d(strain) and pk1_jacobian as d(PK1)/d(F). Select "
      "'stress_wrt_strain' or 'pk1_wrt_deformation_gradient' explicitly for a custom tangent "
      "property; use the latter only when the property is known to be d(PK1)/d(F).");
  params.addParam<bool>(
      "volumetric_locking_correction",
      false,
      "Whether to apply volume locking to the directional correction term in SCZM.");

  return params;
}

ShiftedCohesiveZoneAction::ShiftedCohesiveZoneAction(const InputParameters & params)
  : CohesiveZoneAction(params)
{
  const bool generate_distance = getParam<bool>("generate_sbm_distance");
  const bool has_distance_uo = isParamSetByUser("sbm_distance_uo");
  const bool has_complete_interface_mesh = isParamSetByUser("complete_interface_mesh");
  if (static_cast<unsigned int>(generate_distance) + static_cast<unsigned int>(has_distance_uo) +
          static_cast<unsigned int>(has_complete_interface_mesh) >
      1)
    paramError("sbm_distance_uo",
               "Specify only one of 'sbm_distance_uo', 'generate_sbm_distance', and "
               "'complete_interface_mesh'.");

  if (!generate_distance && isParamSetByUser("surface_meshes"))
    paramError("surface_meshes", "This parameter is only valid with 'generate_sbm_distance=true'.");
  if (!generate_distance && isParamSetByUser("check_surface_watertightness"))
    paramError("check_surface_watertightness",
               "This parameter is only valid with 'generate_sbm_distance=true'.");
  if (!has_complete_interface_mesh && isParamSetByUser("interface_matching_tolerance"))
    paramError("interface_matching_tolerance",
               "This parameter is only valid with 'complete_interface_mesh'.");

  if (generate_distance)
  {
    const auto surface_meshes = surfaceMeshNames();
    if (surface_meshes.size() != _boundary.size())
      paramError("surface_meshes", "The number of surface meshes must match the boundaries.");
    if (std::set<MeshGeneratorName>(surface_meshes.begin(), surface_meshes.end()).size() !=
        surface_meshes.size())
      paramError("surface_meshes", "Each saved surface mesh may only be used once.");
  }

  if (!has_complete_interface_mesh && isParamSetByUser("interface_subdomain_pairs"))
    paramError("interface_subdomain_pairs",
               "This parameter is only valid with 'complete_interface_mesh'.");
  if (has_complete_interface_mesh && isParamSetByUser("interface_subdomain_pairs") &&
      getParam<std::vector<std::vector<SubdomainID>>>("interface_subdomain_pairs").size() !=
          _boundary.size())
    paramError("interface_subdomain_pairs", "Provide exactly one subdomain pair per boundary.");

  if (_use_AD && isParamSetByUser("tangent"))
    paramError("tangent", "This parameter applies only to the non-AD SCZM interface kernel.");
  if (_use_AD && isParamSetByUser("tangent_definition"))
    paramError("tangent_definition",
               "This parameter applies only to the non-AD SCZM interface kernel. The AD kernel "
               "differentiates the residual automatically.");

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
      paramError("strain", "Shifted cohesive zone models currently support only small strain.");
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
  else if (_current_task == "add_user_object" &&
           (getParam<bool>("generate_sbm_distance") || isParamSetByUser("complete_interface_mesh")))
    addSBMDistanceUserObjects();
  else if (_current_task == "add_interface_kernel")
    addRequiredCZMInterfaceKernels();
  else if (_current_task == "add_master_action_material")
    addRequiredCZMInterfaceMaterials();

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

UserObjectName
ShiftedCohesiveZoneAction::interfaceManagerName() const
{
  return name() + "_interface_manager";
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
  auto distance_params = _factory.getValidParams("BoundaryShortestDistanceToSurface");
  distance_params.set<std::vector<BoundaryName>>("boundary") = _boundary;
  distance_params.set<int>("execution_order_group") = 0;
  distance_params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  if (isParamSetByUser("complete_interface_mesh"))
  {
    auto manager_params = _factory.getValidParams("SBMInterfaceManager");
    manager_params.set<MeshGeneratorName>("complete_interface_mesh") =
        getParam<MeshGeneratorName>("complete_interface_mesh");
    manager_params.set<Real>("tolerance") = getParam<Real>("interface_matching_tolerance");
    _problem->addUserObject("SBMInterfaceManager", interfaceManagerName(), manager_params);
    distance_params.set<UserObjectName>("manager") = interfaceManagerName();
    if (isParamSetByUser("interface_subdomain_pairs"))
      distance_params.set<std::vector<std::vector<SubdomainID>>>("interface_subdomain_pairs") =
          getParam<std::vector<std::vector<SubdomainID>>>("interface_subdomain_pairs");
  }
  else
  {
    const auto surface_meshes = surfaceMeshNames();
    std::vector<FunctionName> distance_functions;
    distance_functions.reserve(_boundary.size());
    for (const auto i : index_range(_boundary))
    {
      auto params = _factory.getValidParams("SBMSurfaceMeshBuilder");
      params.set<MeshGeneratorName>("interface_mesh") = surface_meshes[i];
      params.set<bool>("check_watertightness") = getParam<bool>("check_surface_watertightness");
      _problem->addUserObject(
          "SBMSurfaceMeshBuilder", surfaceMeshBuilderName(_boundary[i]), params);
      distance_functions.push_back(surfaceDistanceFunctionName(_boundary[i]));
    }
    distance_params.set<std::vector<FunctionName>>("surfaces") = distance_functions;
  }

  _problem->addUserObject(
      "BoundaryShortestDistanceToSurface", sbmDistanceUserObjectName(), distance_params);
}

void
ShiftedCohesiveZoneAction::customizeCZMInterfaceKernel(InputParameters & params) const
{
  if (getParam<bool>("generate_sbm_distance") || isParamSetByUser("complete_interface_mesh"))
    params.set<UserObjectName>("sbm_distance_uo") = sbmDistanceUserObjectName();
  else if (isParamSetByUser("sbm_distance_uo"))
    params.set<UserObjectName>("sbm_distance_uo") = getParam<UserObjectName>("sbm_distance_uo");
  if (isParamSetByUser("no_shifted"))
    params.set<bool>("no_shifted") = getParam<bool>("no_shifted");
  if (isParamSetByUser("directional_correction"))
    params.set<bool>("directional_correction") = getParam<bool>("directional_correction");
  if (isParamSetByUser("stress"))
    params.set<MaterialPropertyName>("stress") = getParam<MaterialPropertyName>("stress");
  if (isParamSetByUser("tangent"))
    params.set<MaterialPropertyName>("tangent") = getParam<MaterialPropertyName>("tangent");
  if (isParamSetByUser("tangent_definition") && params.isParamValid("tangent_definition"))
    params.set<MooseEnum>("tangent_definition") = getParam<MooseEnum>("tangent_definition");

  if (params.isParamValid("volumetric_locking_correction"))
  {
    const bool inherited_correction = getInheritedVolumetricLockingCorrection(_awh);
    if (isParamSetByUser("volumetric_locking_correction"))
    {
      params.set<bool>("volumetric_locking_correction") =
          getParam<bool>("volumetric_locking_correction");
      if (getParam<bool>("volumetric_locking_correction") != inherited_correction)
        mooseWarning(
            "ShiftedCohesiveZoneAction: 'volumetric_locking_correction' value in "
            "ShiftedCohesiveZone block is different from that in the physics block(s). Make sure "
            "this is intended.");
    }
    else
      params.set<bool>("volumetric_locking_correction") = inherited_correction;
  }
}

void
ShiftedCohesiveZoneAction::customizeCZMDisplacementJump(InputParameters & params) const
{
  if (getParam<bool>("generate_sbm_distance") || isParamSetByUser("complete_interface_mesh"))
    params.set<UserObjectName>("sbm_distance_uo") = sbmDistanceUserObjectName();
  else if (isParamSetByUser("sbm_distance_uo"))
    params.set<UserObjectName>("sbm_distance_uo") = getParam<UserObjectName>("sbm_distance_uo");
  if (isParamSetByUser("no_shifted"))
    params.set<bool>("no_shifted") = getParam<bool>("no_shifted");
}
