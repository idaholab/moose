//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"
#include "MortarConstraintBase.h"
#include "NonlinearSystemBase.h"
#include "Parser.h"
#include "AugmentedLagrangianContactProblem.h"

#include "ContactPairLowerDBlockGenerator.h"

#include <set>
#include <algorithm>
#include <unordered_map>
#include <limits>

#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/string_to_enum.h"

// Counter for naming mortar auxiliary kernels
static unsigned int contact_mortar_auxkernel_counter = 0;

// Counter for naming auxiliary kernels
static unsigned int contact_auxkernel_counter = 0;

// Counter for naming nodal area user objects
static unsigned int contact_userobject_counter = 0;

// Counter for distinct contact action objects
static unsigned int contact_action_counter = 0;

// For mortar subdomains
registerMooseAction("ContactApp", ContactAction, "append_mesh_generator");
registerMooseAction("ContactApp", ContactAction, "add_aux_variable");
// For mortar Lagrange multiplier
registerMooseAction("ContactApp", ContactAction, "add_contact_aux_variable");
registerMooseAction("ContactApp", ContactAction, "add_mortar_variable");
registerMooseAction("ContactApp", ContactAction, "add_aux_kernel");
// For mortar constraint
registerMooseAction("ContactApp", ContactAction, "add_constraint");
registerMooseAction("ContactApp", ContactAction, "output_penetration_info_vars");
registerMooseAction("ContactApp", ContactAction, "add_user_object");
// For automatic generation of contact pairs
registerMooseAction("ContactApp", ContactAction, "post_mesh_prepared");

InputParameters
ContactAction::validParams()
{
  InputParameters params = Action::validParams();
  params += ContactAction::commonParameters();

  params.addParam<std::vector<BoundaryName>>(
      "primary", "The list of boundary IDs referring to primary sidesets");
  params.addParam<std::vector<BoundaryName>>(
      "secondary", "The list of boundary IDs referring to secondary sidesets");
  params.addParam<std::vector<BoundaryName>>(
      "automatic_pairing_boundaries",
      {},
      "List of boundary IDs for sidesets that are automatically paired with any other boundary in "
      "this list having a centroid-to-centroid distance less than the value specified in the "
      "'automatic_pairing_distance' parameter. ");
  params.addRangeCheckedParam<Real>(
      "automatic_pairing_distance",
      "automatic_pairing_distance>=0",
      "The maximum distance the centroids of the boundaries provided in the "
      "'automatic_pairing_boundaries' parameter can be to generate a contact pair automatically. "
      "Due to numerical error in the determination of the centroids, it is encouraged that "
      "the user adds a tolerance to this distance (e.g. extra 10%) to make sure no suitable "
      "contact pair is missed. If the 'automatic_pairing_method = NODE' option is chosen instead, "
      "this distance is recommended to be set to at least twice the minimum distance between "
      "nodes of boundaries to be paired.");
  params.addDeprecatedParam<MeshGeneratorName>(
      "mesh",
      "The mesh generator for mortar method",
      "This parameter is not used anymore and can simply be removed");
  params.addParam<VariableName>("secondary_gap_offset",
                                "Offset to gap distance from secondary side");
  params.addParam<VariableName>("mapped_primary_gap_offset",
                                "Offset to gap distance mapped from primary side");
  params.addParam<std::vector<VariableName>>(
      "displacements",
      {},
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>(
      "penalty_friction",
      1e8,
      "The penalty factor to apply in mortar penalty frictional constraints.  It is applied to the "
      "tangential accumulated slip to build the frictional force");
  params.addRangeCheckedParam<Real>(
      "penalty_multiplier",
      1.0,
      "penalty_multiplier > 0",
      "The growth factor for the penalty applied at the end of each augmented "
      "Lagrange update iteration (a value larger than one, e.g., 10, tends to speed up "
      "convergence.)");
  params.addRangeCheckedParam<Real>(
      "penalty_multiplier_friction",
      1.0,
      "penalty_multiplier_friction > 0",
      "The penalty growth factor between augmented Lagrange "
      "iterations for penalizing relative slip distance if the node is under stick conditions.(a "
      "value larger than one, e.g., 10, tends to speed up convergence.)");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tension_release",
                        0.0,
                        "Tension release threshold.  A node in contact "
                        "will not be released if its tensile load is below "
                        "this value.  No tension release if negative.");
  params.addParam<MooseEnum>("model", ContactAction::getModelEnum(), "The contact model to use");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("capture_tolerance",
                        0.0,
                        "Normal distance from surface within which nodes are captured. This "
                        "parameter is used for node-face and mortar formulations.");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");

  params.addParam<bool>("normalize_penalty",
                        false,
                        "Whether to normalize the penalty parameter with the nodal area.");
  params.addParam<bool>(
      "primary_secondary_jacobian",
      true,
      "Whether to include Jacobian entries coupling primary and secondary nodes.");
  params.addParam<Real>("al_penetration_tolerance",
                        "The tolerance of the penetration for augmented Lagrangian method.");
  params.addParam<Real>("al_incremental_slip_tolerance",
                        "The tolerance of the incremental slip for augmented Lagrangian method.");
  params.addRangeCheckedParam<Real>(
      "max_penalty_multiplier",
      1.0e3,
      "max_penalty_multiplier >= 1.0",
      "Maximum multiplier applied to penalty factors when adaptivity is used in an augmented "
      "Lagrange setting. The penalty factor supplied by the user is used as a reference to "
      "determine its maximum. If this multiplier is too large, the condition number of the system "
      "to be solved may be negatively impacted.");
  MooseEnum adaptivity_penalty_normal("SIMPLE BUSSETTA", "SIMPLE");
  adaptivity_penalty_normal.addDocumentation(
      "SIMPLE", "Keep multiplying by the penalty multiplier between AL iterations");
  adaptivity_penalty_normal.addDocumentation(
      "BUSSETTA",
      "Modify the penalty using an algorithm from Bussetta et al, 2012, Comput Mech 49:259-275 "
      "between AL iterations.");
  params.addParam<MooseEnum>(
      "adaptivity_penalty_normal",
      adaptivity_penalty_normal,
      "The augmented Lagrange update strategy used on the normal penalty coefficient.");
  MooseEnum adaptivity_penalty_friction("SIMPLE FRICTION_LIMIT", "FRICTION_LIMIT");
  adaptivity_penalty_friction.addDocumentation(
      "SIMPLE", "Keep multiplying by the frictional penalty multiplier between AL iterations");
  adaptivity_penalty_friction.addDocumentation(
      "FRICTION_LIMIT",
      "This strategy will be guided by the Coulomb limit and be less reliant on the initial "
      "penalty factor provided by the user.");
  params.addParam<MooseEnum>(
      "adaptivity_penalty_friction",
      adaptivity_penalty_friction,
      "The augmented Lagrange update strategy used on the frictional penalty coefficient.");
  params.addParam<Real>("al_frictional_force_tolerance",
                        "The tolerance of the frictional force for augmented Lagrangian method.");
  params.addParam<Real>(
      "c_normal",
      1e6,
      "Parameter for balancing the size of the gap and contact pressure for a mortar formulation. "
      "This purely numerical "
      "parameter affects convergence behavior and, in general, should be larger for stiffer "
      "materials. It is recommended that the user tries out various orders of magnitude for this "
      "parameter if the default value generates poor contact convergence.");
  params.addParam<Real>(
      "c_tangential", 1, "Numerical parameter for nonlinear mortar frictional constraints");
  params.addParam<bool>("ping_pong_protection",
                        false,
                        "Whether to protect against ping-ponging, e.g. the oscillation of the "
                        "secondary node between two "
                        "different primary faces, by tying the secondary node to the "
                        "edge between the involved primary faces");
  params.addParam<Real>(
      "normal_lm_scaling",
      1.,
      "Scaling factor to apply to the normal LM variable for a mortar formulation");
  params.addParam<Real>(
      "tangential_lm_scaling",
      1.,
      "Scaling factor to apply to the tangential LM variable for a mortar formulation");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c by weighting function norm for mortar contact. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.addClassDescription("Sets up all objects needed for mechanical contact enforcement");
  params.addParam<bool>(
      "use_dual",
      "Whether to use the dual mortar approach within a mortar formulation. It is defaulted to "
      "true for "
      "weighted quantity approach, and to false for the legacy approach. To avoid instabilities "
      "in the solution and obtain the full benefits of a variational enforcement,"
      "use of dual mortar with weighted constraints is strongly recommended. This "
      "input is only intended for advanced users.");
  params.addParam<bool>(
      "correct_edge_dropping",
      false,
      "Whether to enable correct edge dropping treatment for mortar constraints. When disabled "
      "any Lagrange Multiplier degree of freedom on a secondary element without full primary "
      "contributions will be set (strongly) to 0.");
  params.addParam<bool>(
      "generate_mortar_mesh",
      true,
      "Whether to generate the mortar mesh from the action. Typically this will be the case, but "
      "one may also want to reuse an existing lower-dimensional mesh prior to a restart.");
  params.addParam<MooseEnum>("automatic_pairing_method",
                             ContactAction::getProximityMethod(),
                             "The proximity method used for automatic pairing of boundaries.");
  params.addParam<bool>(
      "mortar_dynamics",
      false,
      "Whether to use constraints that account for the persistency condition, giving rise to "
      "smoother normal contact pressure evolution. This flag should only be set to yes for dynamic "
      "simulations using the Newmark-beta numerical integrator");
  params.addParam<Real>(
      "newmark_beta",
      0.25,
      "Newmark-beta beta parameter for its inclusion in the weighted gap update formula");
  params.addParam<Real>(
      "newmark_gamma",
      0.5,
      "Newmark-beta gamma parameter for its inclusion in the weighted gap update formula");
  params.addCoupledVar("wear_depth",
                       "The name of the mortar auxiliary variable that is used to modify the "
                       "weighted gap definition");
  params.addParam<std::vector<TagName>>(
      "extra_vector_tags",
      "The tag names for extra vectors that residual data should be saved into");
  params.addParam<std::vector<TagName>>(
      "absolute_value_vector_tags",
      "The tags for the vectors this residual object should fill with the "
      "absolute value of the residual contribution");
  params.addParam<bool>(
      "use_petrov_galerkin",
      false,
      "Whether to use the Petrov-Galerkin approach for the mortar-based constraints. If set to "
      "true, we use the standard basis as the test function and dual basis as "
      "the shape function for the interpolation of the Lagrange multiplier variable.");
  params.addParam<bool>(
      "debug_mesh",
      false,
      "Whether we are going to enable mortar segment mesh debug information. An exodus"
      "file will be generated if the user sets this flag to true");
  params.transferParam<MooseEnum>(MortarConstraintBase::validParams(), "segment_quadrature");
  return params;
}

ContactAction::ContactAction(const InputParameters & params)
  : Action(params),
    _boundary_pairs(getParam<BoundaryName, BoundaryName>("primary", "secondary")),
    _model(getParam<MooseEnum>("model").getEnum<ContactModel>()),
    _formulation(getParam<MooseEnum>("formulation").getEnum<ContactFormulation>()),
    _generate_mortar_mesh(getParam<bool>("generate_mortar_mesh")),
    _mortar_dynamics(getParam<bool>("mortar_dynamics"))
{
  // Check for automatic selection of contact pairs.
  if (getParam<std::vector<BoundaryName>>("automatic_pairing_boundaries").size() > 1)
    _automatic_pairing_boundaries =
        getParam<std::vector<BoundaryName>>("automatic_pairing_boundaries");

  if (_automatic_pairing_boundaries.size() > 0 && !isParamValid("automatic_pairing_distance"))
    paramError("automatic_pairing_distance",
               "For automatic selection of contact pairs (for particular geometries) in contact "
               "action, 'automatic_pairing_distance' needs to be provided.");

  if (_automatic_pairing_boundaries.size() > 0 && !isParamValid("automatic_pairing_method"))
    paramError("automatic_pairing_distance",
               "For automatic selection of contact pairs (for particular geometries) in contact "
               "action, 'automatic_pairing_method' needs to be provided.");

  if (_automatic_pairing_boundaries.size() > 0 && _boundary_pairs.size() != 0)
    paramError("automatic_pairing_boundaries",
               "If a boundary list is provided, primary and secondary surfaces will be identified "
               "automatically. Therefore, one cannot provide an automatic pairing boundary list "
               "and primary/secondary lists.");
  else if (_automatic_pairing_boundaries.size() == 0 && _boundary_pairs.size() == 0)
    paramError("primary",
               "'primary' and 'secondary' surfaces or a list of boundaries for automatic pair "
               "generation need to be provided.");

  // End of checks for automatic selection of contact pairs.

  if (_formulation == ContactFormulation::TANGENTIAL_PENALTY && _model != ContactModel::COULOMB)
    paramError("formulation",
               "The 'tangential_penalty' formulation can only be used with the 'coulomb' model");

  if (_formulation == ContactFormulation::MORTAR_PENALTY)
  {
    // Use dual basis functions for contact traction interpolation
    if (isParamValid("use_dual"))
      _use_dual = getParam<bool>("use_dual");
    else
      _use_dual = true;

    if (_model == ContactModel::GLUED)
      paramError("model", "The 'mortar_penalty' formulation does not support glued contact");

    if (getParam<bool>("mortar_dynamics"))
      paramError("mortar_dynamics",
                 "The 'mortar_penalty' formulation does not support implicit dynamic simulations");

    if (getParam<bool>("use_petrov_galerkin"))
      paramError("use_petrov_galerkin",
                 "The 'mortar_penalty' formulation does not support usage of the Petrov-Galerkin "
                 "flag. The default (use_dual = true) behavior is such that contact tractions are "
                 "interpolated with dual bases whereas mortar or weighted contact quantities are "
                 "interpolated with Lagrange shape functions.");
  }

  if (_formulation == ContactFormulation::MORTAR)
  {
    if (_model == ContactModel::GLUED)
      paramError("model", "The 'mortar' formulation does not support glued contact (yet)");

    // use dual basis function for Lagrange multipliers?
    if (isParamValid("use_dual"))
      _use_dual = getParam<bool>("use_dual");
    else
      _use_dual = true;

    if (!getParam<bool>("mortar_dynamics"))
    {
      if (params.isParamSetByUser("newmark_beta"))
        paramError("newmark_beta", "newmark_beta can only be used with the mortar_dynamics option");

      if (params.isParamSetByUser("newmark_gamma"))
        paramError("newmark_gamma",
                   "newmark_gamma can only be used with the mortar_dynamics option");
    }

    if (isParamSetByUser("penalty"))
      paramError("penalty",
                 "The 'penalty' parameter is not used for the 'mortar' formulation which instead "
                 "uses Lagrange multipliers");
  }
  else
  {
    if (params.isParamSetByUser("correct_edge_dropping"))
      paramError(
          "correct_edge_dropping",
          "The 'correct_edge_dropping' option can only be used with the 'mortar' formulation "
          "(weighted)");
    else if (params.isParamSetByUser("use_dual") &&
             _formulation != ContactFormulation::MORTAR_PENALTY)
      paramError("use_dual",
                 "The 'use_dual' option can only be used with the 'mortar' formulation");
    else if (params.isParamSetByUser("c_normal"))
      paramError("c_normal",
                 "The 'c_normal' option can only be used with the 'mortar' formulation");
    else if (params.isParamSetByUser("c_tangential"))
      paramError("c_tangential",
                 "The 'c_tangential' option can only be used with the 'mortar' formulation");
    else if (params.isParamSetByUser("mortar_dynamics"))
      paramError("mortar_dynamics",
                 "The 'mortar_dynamics' constraint option can only be used with the 'mortar' "
                 "formulation and in dynamic simulations using Newmark-beta");
    else if (params.isParamSetByUser("segment_quadrature"))
      paramError("segment_quadrature",
                 "The 'segment_quadrature' option can only be used with the "
                 "'mortar' formulation.");
  }

  if (_formulation == ContactFormulation::RANFS)
  {
    if (isParamValid("secondary_gap_offset"))
      paramError("secondary_gap_offset",
                 "The 'secondary_gap_offset' option can only be used with the "
                 "'MechanicalContactConstraint'");
    if (isParamValid("mapped_primary_gap_offset"))
      paramError("mapped_primary_gap_offset",
                 "The 'mapped_primary_gap_offset' option can only be used with the "
                 "'MechanicalContactConstraint'");
  }
  else if (getParam<bool>("ping_pong_protection"))
    paramError("ping_pong_protection",
               "The 'ping_pong_protection' option can only be used with the 'ranfs' formulation");

  // Remove repeated pairs from input file.
  removeRepeatedPairs();
}

void
ContactAction::removeRepeatedPairs()
{
  if (_boundary_pairs.size() == 0 && _automatic_pairing_boundaries.size() == 0)
    paramError(
        "primary",
        "Number of contact pairs in the contact action is zero. Please revise your input file.");

  // Remove repeated interactions
  std::vector<std::pair<BoundaryName, BoundaryName>> lean_boundary_pairs;

  for (const auto & [primary, secondary] : _boundary_pairs)
  {
    // Structured bindings are not capturable (primary_copy, secondary_copy)
    auto it = std::find_if(lean_boundary_pairs.begin(),
                           lean_boundary_pairs.end(),
                           [&, primary_copy = primary, secondary_copy = secondary](
                               const std::pair<BoundaryName, BoundaryName> & lean_pair)
                           {
                             const bool match_one = lean_pair.second == secondary_copy &&
                                                    lean_pair.first == primary_copy;
                             const bool match_two = lean_pair.second == primary_copy &&
                                                    lean_pair.first == secondary_copy;
                             const bool exist = match_one || match_two;
                             return exist;
                           });

    if (it == lean_boundary_pairs.end())
      lean_boundary_pairs.emplace_back(primary, secondary);
    else
      mooseInfo("Contact pair ",
                primary,
                "--",
                secondary,
                " has been removed from the contact interaction list due to "
                "duplicates in the input file.");
  }

  _boundary_pairs = lean_boundary_pairs;
}

void
ContactAction::act()
{
  // proform problem checks/corrections once during the first feasible task
  if (_current_task == "add_contact_aux_variable")
  {
    if (!_problem->getDisplacedProblem())
      mooseError(
          "Contact requires updated coordinates.  Use the 'displacements = ...' parameter in the "
          "Mesh block.");

    // It is risky to apply this optimization to contact problems
    // since the problem configuration may be changed during Jacobian
    // evaluation. We therefore turn it off for all contact problems so that
    // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3.
    if (!_problem->isSNESMFReuseBaseSetbyUser())
      _problem->setSNESMFReuseBase(false, false);
  }

  if (_current_task == "post_mesh_prepared" && _automatic_pairing_boundaries.size() > 0)
  {
    if (getParam<MooseEnum>("automatic_pairing_method").getEnum<ProximityMethod>() ==
        ProximityMethod::NODE)
      createSidesetsFromNodeProximity();
    else if (getParam<MooseEnum>("automatic_pairing_method").getEnum<ProximityMethod>() ==
             ProximityMethod::CENTROID)
      createSidesetPairsFromGeometry();
  }

  if (_formulation == ContactFormulation::MORTAR ||
      _formulation == ContactFormulation::MORTAR_PENALTY)
    addMortarContact();
  else
    addNodeFaceContact();

  if (_current_task == "add_aux_kernel")
  {
    if (!_problem->getDisplacedProblem())
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");

    // Create auxiliary kernels for each contact pairs
    for (const auto & contact_pair : _boundary_pairs)
    {
      const auto & [primary_name, secondary_name] = contact_pair;
      if ((_formulation != ContactFormulation::MORTAR) &&
          (_formulation != ContactFormulation::MORTAR_PENALTY))
      {
        InputParameters params = _factory.getValidParams("PenetrationAux");
        params.applyParameters(parameters(),
                               {"secondary_gap_offset", "mapped_primary_gap_offset", "order"});

        std::vector<VariableName> displacements =
            getParam<std::vector<VariableName>>("displacements");
        const auto order = _problem->systemBaseNonlinear(/*nl_sys_num=*/0)
                               .system()
                               .variable_type(displacements[0])
                               .order.get_order();

        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
        params.set<std::vector<BoundaryName>>("boundary") = {secondary_name};
        params.set<BoundaryName>("paired_boundary") = primary_name;
        params.set<AuxVariableName>("variable") = "penetration";
        if (isParamValid("secondary_gap_offset"))
          params.set<std::vector<VariableName>>("secondary_gap_offset") = {
              getParam<VariableName>("secondary_gap_offset")};
        if (isParamValid("mapped_primary_gap_offset"))
          params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
              getParam<VariableName>("mapped_primary_gap_offset")};
        params.set<bool>("use_displaced_mesh") = true;
        std::string name = _name + "_contact_" + Moose::stringify(contact_auxkernel_counter++);

        _problem->addAuxKernel("PenetrationAux", name, params);
      }
      else
      {
        const auto type = "MortarUserObjectAux";
        InputParameters params = _factory.getValidParams(type);
        params.set<std::vector<BoundaryName>>("boundary") = {secondary_name};
        params.set<AuxVariableName>("variable") = "gap";
        params.set<bool>("use_displaced_mesh") = true; // Unnecessary as this object only operates
                                                       // on nodes, but we'll do it for consistency
        params.set<MooseEnum>("contact_quantity") = "normal_gap";
        const auto & [primary_id, secondary_id, uo_name] =
            libmesh_map_find(_bnd_pair_to_mortar_info, contact_pair);
        params.set<UserObjectName>("user_object") = uo_name;
        std::string name = _name + "_contact_gap_" + std::to_string(primary_id) + "_" +
                           std::to_string(secondary_id);

        _problem->addAuxKernel(type, name, params);
      }
    }

    addContactPressureAuxKernel();

    const unsigned int ndisp = getParam<std::vector<VariableName>>("displacements").size();

    // Add MortarFrictionalPressureVectorAux
    if (_formulation == ContactFormulation::MORTAR && _model == ContactModel::COULOMB && ndisp > 2)
    {
      std::string action_name = MooseUtils::shortName(name());
      for (const auto & contact_pair : _boundary_pairs)
      {
        const std::string suffix = pairSuffix(contact_pair);
        InputParameters params = _factory.getValidParams("MortarFrictionalPressureVectorAux");

        params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
        params.set<ExecFlagEnum>("execute_on", true) = {EXEC_NONLINEAR};
        params.set<std::vector<VariableName>>("tangent_one") = {action_name + "_tangential_lm" +
                                                                suffix};
        params.set<std::vector<VariableName>>("tangent_two") = {action_name + "_tangential_3d_lm" +
                                                                suffix};

        std::vector<std::string> disp_components({"x", "y", "z"});
        unsigned component_index = 0;

        // Loop over three displacements
        for (const auto & disp_component : disp_components)
        {
          params.set<AuxVariableName>("variable") = _name + "_tangent_" + disp_component;
          params.set<unsigned int>("component") = component_index;

          std::string name = _name + "_mortar_frictional_pressure_" + disp_component + "_" +
                             Moose::stringify(contact_mortar_auxkernel_counter++);

          _problem->addAuxKernel("MortarFrictionalPressureVectorAux", name, params);
          component_index++;
        }
      }
    }
  }

  if (_current_task == "add_contact_aux_variable")
  {
    std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
    const auto order = _problem->systemBaseNonlinear(/*nl_sys_num=*/0)
                           .system()
                           .variable_type(displacements[0])
                           .order.get_order();
    std::unique_ptr<InputParameters> current_params;
    const auto create_aux_var_params = [this, order, &current_params]() -> InputParameters &
    {
      current_params = std::make_unique<InputParameters>(_factory.getValidParams("MooseVariable"));
      current_params->set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      current_params->set<MooseEnum>("family") = "LAGRANGE";
      return *current_params;
    };

    if ((_formulation != ContactFormulation::MORTAR) &&
        (_formulation != ContactFormulation::MORTAR_PENALTY))
    {
      // Add penetration aux variable
      _problem->addAuxVariable("MooseVariable", "penetration", create_aux_var_params());
      // Add nodal area aux variable
      _problem->addAuxVariable("MooseVariable", "nodal_area", create_aux_var_params());
    }
    else
      _problem->addAuxVariable("MooseVariable", "gap", create_aux_var_params());

    // Add contact pressure aux variable
    _problem->addAuxVariable("MooseVariable", "contact_pressure", create_aux_var_params());

    const unsigned int ndisp = getParam<std::vector<VariableName>>("displacements").size();

    // Add MortarFrictionalPressureVectorAux variables
    if (_formulation == ContactFormulation::MORTAR && _model == ContactModel::COULOMB && ndisp > 2)
    {
      {
        std::vector<std::string> disp_components({"x", "y", "z"});
        // Loop over three displacements
        for (const auto & disp_component : disp_components)
        {
          auto var_params = _factory.getValidParams("MooseVariable");
          var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
          var_params.set<MooseEnum>("family") = "LAGRANGE";

          _problem->addAuxVariable(
              "MooseVariable", _name + "_tangent_" + disp_component, var_params);
        }
      }
    }
  }

  if (_current_task == "add_user_object" && (_formulation != ContactFormulation::MORTAR) &&
      (_formulation != ContactFormulation::MORTAR_PENALTY))
  {
    auto var_params = _factory.getValidParams("NodalArea");

    // Get secondary_boundary_vector from possibly updated set from the
    // ContactAction constructor cleanup
    const auto actions = _awh.getActions<ContactAction>();

    std::vector<BoundaryName> secondary_boundary_vector;
    for (const auto * const action : actions)
      for (const auto j : index_range(action->_boundary_pairs))
        secondary_boundary_vector.push_back(action->_boundary_pairs[j].second);

    var_params.set<std::vector<BoundaryName>>("boundary") = secondary_boundary_vector;
    var_params.set<std::vector<VariableName>>("variable") = {"nodal_area"};

    mooseAssert(_problem, "Problem pointer is NULL");
    var_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    var_params.set<bool>("use_displaced_mesh") = true;

    _problem->addUserObject("NodalArea",
                            "nodal_area_object_" + Moose::stringify(contact_userobject_counter++),
                            var_params);
  }
}

void
ContactAction::addContactPressureAuxKernel()
{
  // Increment counter for contact action objects
  contact_action_counter++;

  if ((_formulation != ContactFormulation::MORTAR) &&
      (_formulation != ContactFormulation::MORTAR_PENALTY))
  {
    // Add ContactPressureAux: Only one object for all contact pairs
    const auto actions = _awh.getActions<ContactAction>();

    // Add auxiliary kernel if we are the last contact action object.
    if (contact_action_counter == actions.size())
    {
      std::vector<BoundaryName> boundary_vector;
      std::vector<BoundaryName> pair_boundary_vector;

      for (const auto * const action : actions)
        for (const auto j : index_range(action->_boundary_pairs))
        {
          boundary_vector.push_back(action->_boundary_pairs[j].second);
          pair_boundary_vector.push_back(action->_boundary_pairs[j].first);
        }

      InputParameters params = _factory.getValidParams("ContactPressureAux");
      params.applyParameters(parameters(), {"order"});

      std::vector<VariableName> displacements =
          getParam<std::vector<VariableName>>("displacements");
      const auto order = _problem->systemBaseNonlinear(/*nl_sys_num=*/0)
                             .system()
                             .variable_type(displacements[0])
                             .order.get_order();

      params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      params.set<std::vector<BoundaryName>>("boundary") = boundary_vector;
      params.set<std::vector<BoundaryName>>("paired_boundary") = pair_boundary_vector;
      params.set<AuxVariableName>("variable") = "contact_pressure";
      params.addRequiredCoupledVar("nodal_area", "The nodal area");
      params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area"};
      params.set<bool>("use_displaced_mesh") = true;

      std::string name = _name + "_contact_pressure";
      params.set<ExecFlagEnum>("execute_on",
                               true) = {EXEC_NONLINEAR, EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN};
      _problem->addAuxKernel("ContactPressureAux", name, params);
    }
  }
  else
    for (const auto & contact_pair : _boundary_pairs)
    {
      const auto & [_, secondary_name] = contact_pair;
      const auto type = "MortarUserObjectAux";
      InputParameters params = _factory.getValidParams(type);
      params.set<std::vector<BoundaryName>>("boundary") = {secondary_name};
      params.set<AuxVariableName>("variable") = "contact_pressure";
      params.set<bool>("use_displaced_mesh") = true; // Unecessary as this object only operates on
                                                     // nodes, but we'll do it for consistency
      params.set<MooseEnum>("contact_quantity") = "normal_pressure";
      const auto & [primary_id, secondary_id, uo_name] =
          libmesh_map_find(_bnd_pair_to_mortar_info, contact_pair);
      params.set<UserObjectName>("user_object") = uo_name;
      const std::string name = _name + "_contact_pressure" + std::to_string(primary_id) + "_" +
                               std::to_string(secondary_id);

      _problem->addAuxKernel(type, name, params);
    }
}

void
ContactAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  if (_formulation == ContactFormulation::MORTAR ||
      _formulation == ContactFormulation::MORTAR_PENALTY)
  {
    std::string action_name = MooseUtils::shortName(name());
    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string suffix = pairSuffix(contact_pair);
      auto params = MortarConstraintBase::validParams();
      params.set<bool>("use_displaced_mesh") = true;
      params.set<BoundaryName>("primary_boundary") = contact_pair.first;
      params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
      params.set<SubdomainName>("primary_subdomain") = action_name + "_primary_subdomain" + suffix;
      params.set<SubdomainName>("secondary_subdomain") =
          action_name + "_secondary_subdomain" + suffix;
      params.set<bool>("use_petrov_galerkin") = getParam<bool>("use_petrov_galerkin");
      addRelationshipManagers(input_rm_type, params);
    }
  }
}

std::string
ContactAction::pairSuffix(const std::pair<BoundaryName, BoundaryName> & pair) const
{
  return _boundary_pairs.size() > 1 ? "_" + Moose::stringify(pair, "_") : "";
}

void
ContactAction::addMortarContact()
{
  std::string action_name = MooseUtils::shortName(name());

  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  const unsigned int ndisp = displacements.size();

  // Add the lagrange multiplier on the secondary subdomain.
  const auto addLagrangeMultiplier =
      [this, &displacements](const std::string & secondary_subdomain,
                             const std::string & variable_name,
                             const Real scaling_factor,
                             const bool add_aux_lm,
                             const bool penalty_traction) //
  {
    InputParameters params = _factory.getValidParams("MooseVariableBase");

    // Allow the user to select "weighted" constraints and standard bases (use_dual = false) or
    // "legacy" constraints and dual bases (use_dual = true). Unless it's for testing purposes,
    // this combination isn't recommended
    if (!add_aux_lm || penalty_traction)
      params.set<bool>("use_dual") = _use_dual;

    mooseAssert(_problem->systemBaseNonlinear(/*nl_sys_num=*/0).hasVariable(displacements[0]),
                "Displacement variable is missing");
    const auto primal_type =
        _problem->systemBaseNonlinear(/*nl_sys_num=*/0).system().variable_type(displacements[0]);

    const int lm_order = primal_type.order.get_order();

    if (primal_type.family == LAGRANGE)
    {
      params.set<MooseEnum>("family") = Utility::enum_to_string<FEFamily>(primal_type.family);
      params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{lm_order});
    }
    else
      mooseError("Invalid bases for mortar contact.");

    params.set<std::vector<SubdomainName>>("block") = {secondary_subdomain};
    if (!(add_aux_lm || penalty_traction))
      params.set<std::vector<Real>>("scaling") = {scaling_factor};

    auto fe_type = AddVariableAction::feType(params);
    auto var_type = AddVariableAction::variableType(fe_type);
    if (add_aux_lm || penalty_traction)
      _problem->addAuxVariable(var_type, variable_name, params);
    else
      _problem->addVariable(var_type, variable_name, params);
  };

  if (_current_task == "append_mesh_generator")
  {
    // Don't do mesh generators when recovering or when the user has requested for us not to
    // (presumably because the lower-dimensional blocks are already in the mesh due to manual
    // addition or because we are restarting)
    if (!(_app.isRecovering() && _app.isUltimateMaster()) && !_app.useMasterMesh() &&
        _generate_mortar_mesh)
    {
      if (_automatic_pairing_boundaries.size() > 0)
      {
        auto params = _factory.getValidParams("ContactPairLowerDBlockGenerator");
        params.set<std::vector<BoundaryName>>("automatic_pairing_boundaries") =
            _automatic_pairing_boundaries;
        params.set<Real>("automatic_pairing_distance") =
            getParam<Real>("automatic_pairing_distance");
        params.set<MooseEnum>("automatic_pairing_method") =
            getParam<MooseEnum>("automatic_pairing_method");
        params.set<std::string>("prefix") = action_name;
        _app.appendMeshGenerator(
            "ContactPairLowerDBlockGenerator", action_name + "_auto_mortar_generator", params);
      }
      else
      {
        for (const auto & contact_pair : _boundary_pairs)
        {
          const std::string suffix = pairSuffix(contact_pair);
          const std::string primary_subdomain_name = action_name + "_primary_subdomain" + suffix;
          const std::string secondary_subdomain_name =
              action_name + "_secondary_subdomain" + suffix;

          const MeshGeneratorName primary_name = primary_subdomain_name + "_generator";
          const MeshGeneratorName secondary_name = secondary_subdomain_name + "_generator";

          auto primary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
          auto secondary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");

          primary_params.set<SubdomainName>("new_block_name") = primary_subdomain_name;
          secondary_params.set<SubdomainName>("new_block_name") = secondary_subdomain_name;

          primary_params.set<std::vector<BoundaryName>>("sidesets") = {contact_pair.first};
          secondary_params.set<std::vector<BoundaryName>>("sidesets") = {contact_pair.second};

          _app.appendMeshGenerator(
              "LowerDBlockFromSidesetGenerator", primary_name, primary_params);
          _app.appendMeshGenerator(
              "LowerDBlockFromSidesetGenerator", secondary_name, secondary_params);
        }
      }
    }
  }

  if (_current_task == "add_mortar_variable" && _formulation == ContactFormulation::MORTAR)
  {
    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string suffix = pairSuffix(contact_pair);
      const std::string secondary_subdomain_name = action_name + "_secondary_subdomain" + suffix;
      const std::string normal_lagrange_multiplier_name = action_name + "_normal_lm" + suffix;
      const std::string tangential_lagrange_multiplier_name =
          action_name + "_tangential_lm" + suffix;
      const std::string tangential_lagrange_multiplier_3d_name =
          action_name + "_tangential_3d_lm" + suffix;
      const std::string auxiliary_lagrange_multiplier_name = action_name + "_aux_lm" + suffix;

      addLagrangeMultiplier(
          secondary_subdomain_name, normal_lagrange_multiplier_name, getParam<Real>("normal_lm_scaling"), false, false);

      if (_model == ContactModel::COULOMB)
      {
        addLagrangeMultiplier(secondary_subdomain_name,
                              tangential_lagrange_multiplier_name,
                              getParam<Real>("tangential_lm_scaling"),
                              false,
                              false);
        if (ndisp > 2)
          addLagrangeMultiplier(secondary_subdomain_name,
                                tangential_lagrange_multiplier_3d_name,
                                getParam<Real>("tangential_lm_scaling"),
                                false,
                                false);
      }

      if (getParam<bool>("use_petrov_galerkin"))
        addLagrangeMultiplier(
            secondary_subdomain_name, auxiliary_lagrange_multiplier_name, 1.0, true, false);
    }
  }
  else if (_current_task == "add_mortar_variable" &&
           _formulation == ContactFormulation::MORTAR_PENALTY)
  {
    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string suffix = pairSuffix(contact_pair);
      const std::string secondary_subdomain_name = action_name + "_secondary_subdomain" + suffix;
      const std::string auxiliary_lagrange_multiplier_name = action_name + "_aux_lm" + suffix;

      if (_use_dual)
        addLagrangeMultiplier(
            secondary_subdomain_name, auxiliary_lagrange_multiplier_name, 1.0, false, true);
    }
  }

  if (_current_task == "add_user_object")
  {
    // check if the correct problem class is selected if AL parameters are provided
    if (_formulation == ContactFormulation::MORTAR_PENALTY &&
        !dynamic_cast<AugmentedLagrangianContactProblemInterface *>(_problem.get()))
    {
      const std::vector<std::string> params = {"penalty_multiplier",
                                               "penalty_multiplier_friction",
                                               "al_penetration_tolerance",
                                               "al_incremental_slip_tolerance",
                                               "al_frictional_force_tolerance"};
      for (const auto & param : params)
        if (parameters().isParamSetByUser(param))
          paramError(param,
                     "Augmented Lagrange parameter was specified, but the selected problem type "
                     "does not support Augmented Lagrange iterations.");
    }

    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string suffix = pairSuffix(contact_pair);
      const std::string primary_subdomain_name = action_name + "_primary_subdomain" + suffix;
      const std::string secondary_subdomain_name = action_name + "_secondary_subdomain" + suffix;
      const std::string normal_lagrange_multiplier_name = action_name + "_normal_lm" + suffix;
      const std::string tangential_lagrange_multiplier_name =
          action_name + "_tangential_lm" + suffix;
      const std::string tangential_lagrange_multiplier_3d_name =
          action_name + "_tangential_3d_lm" + suffix;
      const std::string auxiliary_lagrange_multiplier_name = action_name + "_aux_lm" + suffix;

      const auto register_mortar_uo_name = [this, &suffix](const auto & bnd_pair,
                                                           const auto & uo_prefix)
      {
        const auto & [primary_name, secondary_name] = bnd_pair;
        const auto primary_id = _mesh->getBoundaryID(primary_name);
        const auto secondary_id = _mesh->getBoundaryID(secondary_name);
        const auto uo_name = uo_prefix + name() + suffix;
        _bnd_pair_to_mortar_info.emplace(bnd_pair, MortarInfo{primary_id, secondary_id, uo_name});
        return uo_name;
      };

      if (_model != ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR)
      {
        auto uo_params = _factory.getValidParams("LMWeightedGapUserObject");

        uo_params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        uo_params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        uo_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        uo_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        uo_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        uo_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          uo_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};
        uo_params.set<bool>("use_displaced_mesh") = true;
        uo_params.set<std::vector<VariableName>>("lm_variable") = {normal_lagrange_multiplier_name};
        uo_params.applySpecificParameters(
            parameters(), {"correct_edge_dropping", "use_petrov_galerkin", "debug_mesh"});
        if (getParam<bool>("use_petrov_galerkin"))
          uo_params.set<std::vector<VariableName>>("aux_lm") = {
              auxiliary_lagrange_multiplier_name};

        _problem->addUserObject(
            "LMWeightedGapUserObject",
            register_mortar_uo_name(contact_pair, "lm_weightedgap_object_"),
            uo_params);
      }
      else if (_model == ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR)
      {
        auto uo_params = _factory.getValidParams("LMWeightedVelocitiesUserObject");
        uo_params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        uo_params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        uo_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        uo_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        uo_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        uo_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          uo_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        uo_params.set<VariableName>("secondary_variable") = displacements[0];
        uo_params.set<bool>("use_displaced_mesh") = true;
        uo_params.set<std::vector<VariableName>>("lm_variable_normal") = {
            normal_lagrange_multiplier_name};
        uo_params.set<std::vector<VariableName>>("lm_variable_tangential_one") = {
            tangential_lagrange_multiplier_name};
        if (ndisp > 2)
          uo_params.set<std::vector<VariableName>>("lm_variable_tangential_two") = {
              tangential_lagrange_multiplier_3d_name};
        uo_params.applySpecificParameters(
            parameters(), {"correct_edge_dropping", "use_petrov_galerkin", "debug_mesh"});
        if (getParam<bool>("use_petrov_galerkin"))
          uo_params.set<std::vector<VariableName>>("aux_lm") = {
              auxiliary_lagrange_multiplier_name};

        _problem->addUserObject(
            "LMWeightedVelocitiesUserObject",
            register_mortar_uo_name(contact_pair, "lm_weightedvelocities_object_"),
            uo_params);
      }

      if (_model != ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR_PENALTY)
      {
        auto uo_params = _factory.getValidParams("PenaltyWeightedGapUserObject");

        uo_params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        uo_params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        uo_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        uo_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        uo_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        uo_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};

        // AL parameters
        uo_params.applySpecificParameters(parameters(),
                                          {"correct_edge_dropping",
                                           "penalty",
                                           "debug_mesh",
                                           "max_penalty_multiplier",
                                           "adaptivity_penalty_normal"});

        if (isParamValid("al_penetration_tolerance"))
          uo_params.set<Real>("penetration_tolerance") =
              getParam<Real>("al_penetration_tolerance");
        if (isParamValid("penalty_multiplier"))
          uo_params.set<Real>("penalty_multiplier") = getParam<Real>("penalty_multiplier");
        // In the contact action, we force the physical value of the normal gap, which also
        // normalizes the penalty factor with the "area" around the node
        uo_params.set<bool>("use_physical_gap") = true;

        if (_use_dual)
          uo_params.set<std::vector<VariableName>>("aux_lm") = {
              auxiliary_lagrange_multiplier_name};

        if (ndisp > 2)
          uo_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};
        uo_params.set<bool>("use_displaced_mesh") = true;

        _problem->addUserObject(
            "PenaltyWeightedGapUserObject",
            register_mortar_uo_name(contact_pair, "penalty_weightedgap_object_"),
            uo_params);
        _problem->haveADObjects(true);
      }
      else if (_model == ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR_PENALTY)
      {
        auto uo_params = _factory.getValidParams("PenaltyFrictionUserObject");
        uo_params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        uo_params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        uo_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        uo_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        uo_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        uo_params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
        uo_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          uo_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        uo_params.set<VariableName>("secondary_variable") = displacements[0];
        uo_params.set<bool>("use_displaced_mesh") = true;
        uo_params.set<Real>("friction_coefficient") = getParam<Real>("friction_coefficient");
        uo_params.set<Real>("penalty") = getParam<Real>("penalty");
        uo_params.set<Real>("penalty_friction") = getParam<Real>("penalty_friction");

        // AL parameters
        uo_params.set<Real>("max_penalty_multiplier") = getParam<Real>("max_penalty_multiplier");
        uo_params.set<MooseEnum>("adaptivity_penalty_normal") =
            getParam<MooseEnum>("adaptivity_penalty_normal");
        uo_params.set<MooseEnum>("adaptivity_penalty_friction") =
            getParam<MooseEnum>("adaptivity_penalty_friction");
        if (isParamValid("al_penetration_tolerance"))
          uo_params.set<Real>("penetration_tolerance") =
              getParam<Real>("al_penetration_tolerance");
        if (isParamValid("penalty_multiplier"))
          uo_params.set<Real>("penalty_multiplier") = getParam<Real>("penalty_multiplier");
        if (isParamValid("penalty_multiplier_friction"))
          uo_params.set<Real>("penalty_multiplier_friction") =
              getParam<Real>("penalty_multiplier_friction");

        if (isParamValid("al_incremental_slip_tolerance"))
          uo_params.set<Real>("slip_tolerance") = getParam<Real>("al_incremental_slip_tolerance");
        // In the contact action, we force the physical value of the normal gap, which also
        // normalizes the penalty factor with the "area" around the node
        uo_params.set<bool>("use_physical_gap") = true;

        if (_use_dual)
          uo_params.set<std::vector<VariableName>>("aux_lm") = {
              auxiliary_lagrange_multiplier_name};

        uo_params.applySpecificParameters(parameters(),
                                          {"friction_coefficient", "penalty", "penalty_friction"});

        _problem->addUserObject(
            "PenaltyFrictionUserObject",
            register_mortar_uo_name(contact_pair, "penalty_friction_object_"),
            uo_params);
        _problem->haveADObjects(true);
      }
    }
  }

  if (_current_task == "add_constraint")
  {
    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string suffix = pairSuffix(contact_pair);
      const std::string primary_subdomain_name = action_name + "_primary_subdomain" + suffix;
      const std::string secondary_subdomain_name = action_name + "_secondary_subdomain" + suffix;
      const std::string normal_lagrange_multiplier_name = action_name + "_normal_lm" + suffix;
      const std::string tangential_lagrange_multiplier_name =
          action_name + "_tangential_lm" + suffix;
      const std::string tangential_lagrange_multiplier_3d_name =
          action_name + "_tangential_3d_lm" + suffix;

      // Prepare problem for enforcement with Lagrange multipliers
      if (_model != ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR)
      {
        std::string mortar_constraint_name;

        if (!_mortar_dynamics)
          mortar_constraint_name = "ComputeWeightedGapLMMechanicalContact";
        else
          mortar_constraint_name = "ComputeDynamicWeightedGapLMMechanicalContact";

        InputParameters params = _factory.getValidParams(mortar_constraint_name);
        if (_mortar_dynamics)
          params.applySpecificParameters(
              parameters(), {"newmark_beta", "newmark_gamma", "capture_tolerance", "wear_depth"});

        else // We need user objects for quasistatic constraints
          params.set<UserObjectName>("weighted_gap_uo") =
              "lm_weightedgap_object_" + name() + suffix;

        params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        params.set<Real>("c") = getParam<Real>("c_normal");

        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        params.set<bool>("use_displaced_mesh") = true;

        params.applySpecificParameters(parameters(),
                                       {"correct_edge_dropping",
                                        "normalize_c",
                                        "extra_vector_tags",
                                        "absolute_value_vector_tags",
                                        "debug_mesh"});

        _problem->addConstraint(
            mortar_constraint_name, action_name + "_normal_lm_weighted_gap" + suffix, params);
        _problem->haveADObjects(true);
      }
      // Add the tangential and normal Lagrange's multiplier constraints on the secondary boundary.
      else if (_model == ContactModel::COULOMB && _formulation == ContactFormulation::MORTAR)
      {
        std::string mortar_constraint_name;

        if (!_mortar_dynamics)
          mortar_constraint_name = "ComputeFrictionalForceLMMechanicalContact";
        else
          mortar_constraint_name = "ComputeDynamicFrictionalForceLMMechanicalContact";

        InputParameters params = _factory.getValidParams(mortar_constraint_name);
        if (_mortar_dynamics)
          params.applySpecificParameters(
              parameters(), {"newmark_beta", "newmark_gamma", "capture_tolerance", "wear_depth"});
        else
        { // We need user objects for quasistatic constraints
          params.set<UserObjectName>("weighted_gap_uo") =
              "lm_weightedvelocities_object_" + name() + suffix;
          params.set<UserObjectName>("weighted_velocities_uo") =
              "lm_weightedvelocities_object_" + name() + suffix;
        }

        params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
        params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        params.set<bool>("use_displaced_mesh") = true;
        params.set<Real>("c_t") = getParam<Real>("c_tangential");
        params.set<Real>("c") = getParam<Real>("c_normal");
        params.set<bool>("normalize_c") = getParam<bool>("normalize_c");
        params.set<bool>("compute_primal_residuals") = false;

        params.set<MooseEnum>("segment_quadrature") = getParam<MooseEnum>("segment_quadrature");

        params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};

        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<std::vector<VariableName>>("friction_lm") = {
            tangential_lagrange_multiplier_name};

        if (ndisp > 2)
          params.set<std::vector<VariableName>>("friction_lm_dir") = {
              tangential_lagrange_multiplier_3d_name};

        params.set<Real>("mu") = getParam<Real>("friction_coefficient");
        params.applySpecificParameters(
            parameters(), {"extra_vector_tags", "absolute_value_vector_tags", "debug_mesh"});

        _problem->addConstraint(
            mortar_constraint_name, action_name + "_tangential_lm" + suffix, params);
        _problem->haveADObjects(true);
      }

      const auto addMechanicalContactConstraints =
          [this, &contact_pair, &suffix, &primary_subdomain_name, &secondary_subdomain_name,
           &displacements](const std::string & variable_name,
                           const std::string & constraint_prefix,
                           const std::string & constraint_type,
                           const bool is_additional_frictional_constraint,
                           const bool is_normal_constraint)
      {
        InputParameters params = _factory.getValidParams(constraint_type);

        params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
        params.set<BoundaryName>("primary_boundary") = contact_pair.first;
        params.set<BoundaryName>("secondary_boundary") = contact_pair.second;
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;

        if (_formulation == ContactFormulation::MORTAR)
          params.set<NonlinearVariableName>("variable") = variable_name;

        params.set<MooseEnum>("segment_quadrature") = getParam<MooseEnum>("segment_quadrature");
        params.set<bool>("use_displaced_mesh") = true;
        params.set<bool>("compute_lm_residuals") = false;

        // Additional displacement residual for frictional problem
        // The second frictional LM acts on a perpendicular direction.
        if (is_additional_frictional_constraint)
          params.set<MooseEnum>("direction") = "direction_2";
        params.applySpecificParameters(
            parameters(), {"extra_vector_tags", "absolute_value_vector_tags", "debug_mesh"});

        for (const auto i : index_range(displacements))
        {
          std::string constraint_name = constraint_prefix + Moose::stringify(i);

          params.set<VariableName>("secondary_variable") = displacements[i];
          params.set<MooseEnum>("component") = i;

          if (is_normal_constraint && _model != ContactModel::COULOMB &&
              _formulation == ContactFormulation::MORTAR)
            params.set<UserObjectName>("weighted_gap_uo") =
                "lm_weightedgap_object_" + name() + suffix;
          else if (is_normal_constraint && _model == ContactModel::COULOMB &&
                   _formulation == ContactFormulation::MORTAR)
            params.set<UserObjectName>("weighted_gap_uo") =
                "lm_weightedvelocities_object_" + name() + suffix;
          else if (_formulation == ContactFormulation::MORTAR)
            params.set<UserObjectName>("weighted_velocities_uo") =
                "lm_weightedvelocities_object_" + name() + suffix;
          else if (is_normal_constraint && _model != ContactModel::COULOMB &&
                   _formulation == ContactFormulation::MORTAR_PENALTY)
            params.set<UserObjectName>("weighted_gap_uo") =
                "penalty_weightedgap_object_" + name() + suffix;
          else if (is_normal_constraint && _model == ContactModel::COULOMB &&
                   _formulation == ContactFormulation::MORTAR_PENALTY)
            params.set<UserObjectName>("weighted_gap_uo") =
                "penalty_friction_object_" + name() + suffix;
          else if (_formulation == ContactFormulation::MORTAR_PENALTY)
            params.set<UserObjectName>("weighted_velocities_uo") =
                "penalty_friction_object_" + name() + suffix;

          _problem->addConstraint(constraint_type, constraint_name, params);
        }
        _problem->haveADObjects(true);
      };

      // Add mortar mechanical contact constraint objects for primal variables
      addMechanicalContactConstraints(normal_lagrange_multiplier_name,
                                      action_name + "_normal_constraint" + suffix + "_",
                                      "NormalMortarMechanicalContact",
                                      /* is_additional_frictional_constraint = */ false,
                                      /* is_normal_constraint = */ true);

      if (_model == ContactModel::COULOMB)
      {
        addMechanicalContactConstraints(tangential_lagrange_multiplier_name,
                                        action_name + "_tangential_constraint" + suffix + "_",
                                        "TangentialMortarMechanicalContact",
                                        /* is_additional_frictional_constraint = */ false,
                                        /* is_normal_constraint = */ false);
        if (ndisp > 2)
          addMechanicalContactConstraints(
              tangential_lagrange_multiplier_3d_name,
              action_name + "_tangential_constraint_3d" + suffix + "_",
              "TangentialMortarMechanicalContact",
              /* is_additional_frictional_constraint = */ true,
              /* is_normal_constraint = */ false);
      }
    }
  }
}

void
ContactAction::addNodeFaceContact()
{
  if (_current_task != "add_constraint")
    return;

  std::string action_name = MooseUtils::shortName(name());
  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  const unsigned int ndisp = displacements.size();

  std::string constraint_type;

  if (_formulation == ContactFormulation::RANFS)
    constraint_type = "RANFSNormalMechanicalContact";
  else
    constraint_type = "MechanicalContactConstraint";

  InputParameters params = _factory.getValidParams(constraint_type);

  params.applyParameters(parameters(),
                         {"displacements",
                          "secondary_gap_offset",
                          "mapped_primary_gap_offset",
                          "primary",
                          "secondary"});

  const auto order = _problem->systemBaseNonlinear(/*nl_sys_num=*/0)
                         .system()
                         .variable_type(displacements[0])
                         .order.get_order();

  params.set<std::vector<VariableName>>("displacements") = displacements;
  params.set<bool>("use_displaced_mesh") = true;
  params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});

  for (const auto & contact_pair : _boundary_pairs)
  {
    if (_formulation != ContactFormulation::RANFS)
    {
      params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area"};
      params.set<BoundaryName>("boundary") = contact_pair.first;
      if (isParamValid("secondary_gap_offset"))
        params.set<std::vector<VariableName>>("secondary_gap_offset") = {
            getParam<VariableName>("secondary_gap_offset")};
      if (isParamValid("mapped_primary_gap_offset"))
        params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
            getParam<VariableName>("mapped_primary_gap_offset")};
    }

    for (unsigned int i = 0; i < ndisp; ++i)
    {
      std::string name = action_name + "_constraint_" + Moose::stringify(contact_pair, "_") + "_" +
                         Moose::stringify(i);

      if (_formulation == ContactFormulation::RANFS)
        params.set<MooseEnum>("component") = i;
      else
        params.set<unsigned int>("component") = i;

      params.set<BoundaryName>("primary") = contact_pair.first;
      params.set<BoundaryName>("secondary") = contact_pair.second;
      params.set<NonlinearVariableName>("variable") = displacements[i];
      params.set<std::vector<VariableName>>("primary_variable") = {displacements[i]};
      params.applySpecificParameters(parameters(),
                                     {"extra_vector_tags", "absolute_value_vector_tags"});
      _problem->addConstraint(constraint_type, name, params);
    }
  }
}

void
ContactAction::createSidesetsFromNodeProximity()
{
  mooseInfo("The contact action is reading the list of boundaries and automatically pairs them "
            "if the distance between nodes is less than a specified distance.");

  if (!_mesh)
    mooseError("Failed to obtain mesh for automatically generating contact pairs.");

  if (!_mesh->getMesh().is_serial())
    paramError(
        "automatic_pairing_boundaries",
        "The generation of automatic contact pairs in the contact action requires a serial mesh.");

  _boundary_pairs = ContactPairLowerDBlockGenerator::findPairsNodeProximity(
      _mesh->getMesh(),
      _automatic_pairing_boundaries,
      getParam<Real>("automatic_pairing_distance"));

  mooseInfo(
      "The following boundary pairs were created by the contact action using nodal proximity: ");
  for (const auto & [primary, secondary] : _boundary_pairs)
    mooseInfoRepeated(
        "Primary boundary ID: ", primary, " and secondary boundary ID: ", secondary, ".");
}

void
ContactAction::createSidesetPairsFromGeometry()
{
  mooseInfo("The contact action is reading the list of boundaries and automatically pairs them "
            "if their centroids fall within a specified distance of each other.");

  if (!_mesh)
    mooseError("Failed to obtain mesh for automatically generating contact pairs.");

  if (!_mesh->getMesh().is_serial())
    paramError(
        "automatic_pairing_boundaries",
        "The generation of automatic contact pairs in the contact action requires a serial mesh.");

  _boundary_pairs = ContactPairLowerDBlockGenerator::findPairsCentroid(
      _mesh->getMesh(),
      _automatic_pairing_boundaries,
      getParam<Real>("automatic_pairing_distance"));
}

MooseEnum
ContactAction::getModelEnum()
{
  return MooseEnum(getContactModelOptions(), "frictionless");
}

MooseEnum
ContactAction::getProximityMethod()
{
  return MooseEnum(getProximityMethodOptions());
}

MooseEnum
ContactAction::getFormulationEnum()
{
  auto formulations = MooseEnum(getContactFormulationOptions(), "kinematic");

  formulations.addDocumentation(
      "ranfs",
      "Reduced Active Nonlinear Function Set scheme for node-on-face contact. Provides exact "
      "enforcement without Lagrange multipliers or penalty terms.");
  formulations.addDocumentation(
      "kinematic",
      "Kinematic contact constraint enforcement transfers the internal forces at secondary nodes "
      "to the corresponding primary face for node-on-face contact. Provides exact "
      "enforcement without Lagrange multipliers or penalty terms.");
  formulations.addDocumentation(
      "penalty",
      "Node-on-face penalty based contact constraint enforcement. Interpenetration is penalized. "
      "Enforcement depends on the penalty magnitude. High penalties can introduce ill conditioning "
      "of the system.");
  formulations.addDocumentation("augmented_lagrange",
                                "Node-on-face augmented Lagrange penalty based contact constraint "
                                "enforcement. Interpenetration is enforced up to a user specified "
                                "tolerance, ill-conditioning is generally avoided. Requires an "
                                "Augmented Lagrange Problem class to be used in the simulation.");
  formulations.addDocumentation(
      "tangential_penalty",
      "Node-on-face penalty based frictional contact constraint enforcement. Interpenetration and "
      "slip distance for sticking nodes are penalized. Enforcement depends on the penalty "
      "magnitudes. High penalties can introduce ill conditioning of the system.");
  formulations.addDocumentation(
      "mortar",
      "Mortar based contact constraint enforcement using Lagrange multipliers. Provides exact "
      "enforcement and a variationally consistent formulation. Lagrange multipliers introduce a "
      "saddle point character in the system matrix which can have a negative impact on scalability "
      "with iterative solvers");
  formulations.addDocumentation(
      "mortar_penalty",
      "Mortar and penalty based contact constraint enforcement. When using an Augmented Lagrange "
      "Problem class this provides normal (and tangential) contact constratint enforced up to a "
      "user specified tolerances. Without AL the enforcement depends on the penalty magnitudes. "
      "High penalties can introduce ill conditioning of the system.");

  return formulations;
}

MooseEnum
ContactAction::getSystemEnum()
{
  return MooseEnum("Constraint", "Constraint");
}

MooseEnum
ContactAction::getSmoothingEnum()
{
  return MooseEnum("edge_based nodal_normal_based", "");
}

InputParameters
ContactAction::commonParameters()
{
  InputParameters params = emptyInputParameters();

  params.addParam<MooseEnum>("normal_smoothing_method",
                             ContactAction::getSmoothingEnum(),
                             "Method to use to smooth normals");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");

  params.addParam<MooseEnum>(
      "formulation", ContactAction::getFormulationEnum(), "The contact formulation");

  params.addParam<MooseEnum>("model", ContactAction::getModelEnum(), "The contact model to use");

  return params;
}
