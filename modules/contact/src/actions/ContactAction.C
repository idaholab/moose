//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

// for mortar subdomains
registerMooseAction("ContactApp", ContactAction, "append_mesh_generator");
registerMooseAction("ContactApp", ContactAction, "add_aux_variable");
// for mortar Lagrange multiplier
registerMooseAction("ContactApp", ContactAction, "add_contact_aux_variable");
registerMooseAction("ContactApp", ContactAction, "add_mortar_variable");
registerMooseAction("ContactApp", ContactAction, "add_aux_kernel");
// for mortar constraint
registerMooseAction("ContactApp", ContactAction, "add_constraint");
registerMooseAction("ContactApp", ContactAction, "output_penetration_info_vars");
registerMooseAction("ContactApp", ContactAction, "add_user_object");

InputParameters
ContactAction::validParams()
{
  InputParameters params = Action::validParams();
  params += ContactAction::commonParameters();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "primary", "The list of boundary IDs referring to primary sidesets");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "secondary", "The list of boundary IDs referring to secondary sidesets");
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
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
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

  if (_boundary_pairs.size() != 1 && _formulation == ContactFormulation::MORTAR)
    paramError("formulation", "When using mortar, a vector of contact pairs cannot be used");

  if (_formulation == ContactFormulation::TANGENTIAL_PENALTY && _model != ContactModel::COULOMB)
    paramError("formulation",
               "The 'tangential_penalty' formulation can only be used with the 'coulomb' model");

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
  }
  else
  {
    if (params.isParamSetByUser("correct_edge_dropping"))
      paramError(
          "correct_edge_dropping",
          "The 'correct_edge_dropping' option can only be used with the 'mortar' formulation "
          "(weighted)");
    else if (params.isParamSetByUser("use_dual"))
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

  if (_formulation == ContactFormulation::MORTAR)
    addMortarContact();
  else
    addNodeFaceContact();

  if (_current_task == "add_aux_kernel")
  { // Add ContactPenetrationAuxAction.
    if (!_problem->getDisplacedProblem())
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");
    // Create auxiliary kernels for each contact pairs
    for (const auto & contact_pair : _boundary_pairs)
    {
      {
        InputParameters params = _factory.getValidParams("PenetrationAux");
        params.applyParameters(parameters(),
                               {"secondary_gap_offset", "mapped_primary_gap_offset", "order"});

        std::vector<VariableName> displacements =
            getParam<std::vector<VariableName>>("displacements");
        const auto order = _problem->systemBaseNonlinear()
                               .system()
                               .variable_type(displacements[0])
                               .order.get_order();

        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
        params.set<BoundaryName>("paired_boundary") = contact_pair.first;
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
    }

    addContactPressureAuxKernel();

    const unsigned int ndisp = getParam<std::vector<VariableName>>("displacements").size();

    // Add MortarFrictionalPressureVectorAux
    if (_formulation == ContactFormulation::MORTAR && _model == ContactModel::COULOMB && ndisp > 2)
    {
      {
        InputParameters params = _factory.getValidParams("MortarFrictionalPressureVectorAux");

        params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
        params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
        params.set<std::vector<BoundaryName>>("boundary") = {_boundary_pairs[0].second};
        params.set<ExecFlagEnum>("execute_on", true) = {EXEC_NONLINEAR};

        std::string action_name = MooseUtils::shortName(name());
        const std::string tangential_lagrange_multiplier_name = action_name + "_tangential_lm";
        const std::string tangential_lagrange_multiplier_3d_name =
            action_name + "_tangential_3d_lm";

        params.set<std::vector<VariableName>>("tangent_one") = {
            tangential_lagrange_multiplier_name};
        params.set<std::vector<VariableName>>("tangent_two") = {
            tangential_lagrange_multiplier_3d_name};

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
    const auto order =
        _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();
    // Add ContactPenetrationVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "penetration", var_params);
    }
    // Add ContactPressureVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "contact_pressure", var_params);
    }
    // Add nodal area contact variable
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "nodal_area", var_params);
    }

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

  if (_current_task == "add_user_object")
  {
    auto var_params = _factory.getValidParams("NodalArea");
    var_params.set<std::vector<BoundaryName>>("boundary") =
        getParam<std::vector<BoundaryName>>("secondary");
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
  // Add ContactPressureAux: Only one object for all contact pairs
  // if (_formulation != ContactFormulation::MORTAR)
  auto actions = _awh.getActions<ContactAction>();

  // Increment counter for contact action objects
  contact_action_counter++;

  // Add auxiliary kernel if we are the last contact action object.
  if (contact_action_counter == actions.size())
  {
    size_t all_action_pairs_size = 0;
    for (const auto & action : actions)
      all_action_pairs_size += action->_boundary_pairs.size();

    std::vector<BoundaryName> boundary_vector(all_action_pairs_size);
    std::vector<BoundaryName> pair_boundary_vector(all_action_pairs_size);

    size_t i = 0;
    for (const auto & action : actions)
    {
      for (const auto j : make_range(action->_boundary_pairs.size()))
      {
        pair_boundary_vector[i] = action->_boundary_pairs[j].first;
        boundary_vector[i] = action->_boundary_pairs[j].second;
        i++;
      }
    }

    InputParameters params = _factory.getValidParams("ContactPressureAux");
    params.applyParameters(parameters(), {"order"});

    std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
    const auto order =
        _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();

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

void
ContactAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  if (_formulation == ContactFormulation::MORTAR)
  {
    auto params = MortarConstraintBase::validParams();
    params.set<bool>("use_displaced_mesh") = true;
    std::string action_name = MooseUtils::shortName(name());
    const std::string primary_subdomain_name = action_name + "_primary_subdomain";
    const std::string secondary_subdomain_name = action_name + "_secondary_subdomain";
    params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
    params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
    params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
    params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
    addRelationshipManagers(input_rm_type, params);
  }
}

void
ContactAction::addMortarContact()
{
  std::string action_name = MooseUtils::shortName(name());

  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  const unsigned int ndisp = displacements.size();

  // Definitions for mortar contact.
  const std::string primary_subdomain_name = action_name + "_primary_subdomain";
  const std::string secondary_subdomain_name = action_name + "_secondary_subdomain";
  const std::string normal_lagrange_multiplier_name = action_name + "_normal_lm";
  const std::string tangential_lagrange_multiplier_name = action_name + "_tangential_lm";
  const std::string tangential_lagrange_multiplier_3d_name = action_name + "_tangential_3d_lm";

  if (_current_task == "append_mesh_generator")
  {
    // Don't do mesh generators when recovering or when the user has requested for us not to
    // (presumably because the lower-dimensional blocks are already in the mesh due to manual
    // addition or because we are restarting)
    if (!(_app.isRecovering() && _app.isUltimateMaster()) && !_app.masterMesh() &&
        _generate_mortar_mesh)
    {
      const MeshGeneratorName primary_name = primary_subdomain_name + "_generator";
      const MeshGeneratorName secondary_name = secondary_subdomain_name + "_generator";

      auto primary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
      auto secondary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");

      primary_params.set<SubdomainName>("new_block_name") = primary_subdomain_name;
      secondary_params.set<SubdomainName>("new_block_name") = secondary_subdomain_name;

      primary_params.set<std::vector<BoundaryName>>("sidesets") = {_boundary_pairs[0].first};
      secondary_params.set<std::vector<BoundaryName>>("sidesets") = {_boundary_pairs[0].second};

      _app.appendMeshGenerator("LowerDBlockFromSidesetGenerator", primary_name, primary_params);
      _app.appendMeshGenerator("LowerDBlockFromSidesetGenerator", secondary_name, secondary_params);
    }
  }

  if (_current_task == "add_mortar_variable")
  {
    // Add the lagrange multiplier on the secondary subdomain.
    const auto addLagrangeMultiplier =
        [this, &secondary_subdomain_name, &displacements](const std::string & variable_name,
                                                          const Real scaling_factor) //
    {
      InputParameters params = _factory.getValidParams("MooseVariableBase");

      // Allow the user to select "weighted" constraints and standard bases (use_dual = false) or
      // "legacy" constraints and dual bases (use_dual = true). Unless it's for testing purposes,
      // this combination isn't recommended
      params.set<bool>("use_dual") = _use_dual;

      mooseAssert(_problem->systemBaseNonlinear().hasVariable(displacements[0]),
                  "Displacement variable is missing");
      const auto primal_type =
          _problem->systemBaseNonlinear().system().variable_type(displacements[0]);

      const int lm_order = primal_type.order.get_order();

      if (primal_type.family == LAGRANGE)
      {
        params.set<MooseEnum>("family") = Utility::enum_to_string<FEFamily>(primal_type.family);
        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{lm_order});
      }
      else
        mooseError("Invalid bases for mortar contact.");

      params.set<std::vector<SubdomainName>>("block") = {secondary_subdomain_name};
      params.set<std::vector<Real>>("scaling") = {scaling_factor};
      auto fe_type = AddVariableAction::feType(params);
      auto var_type = AddVariableAction::variableType(fe_type);
      _problem->addVariable(var_type, variable_name, params);
    };

    addLagrangeMultiplier(normal_lagrange_multiplier_name, getParam<Real>("normal_lm_scaling"));

    if (_model == ContactModel::COULOMB)
    {
      addLagrangeMultiplier(tangential_lagrange_multiplier_name,
                            getParam<Real>("tangential_lm_scaling"));
      if (ndisp > 2)
        addLagrangeMultiplier(tangential_lagrange_multiplier_3d_name,
                              getParam<Real>("tangential_lm_scaling"));
    }
  }

  if (_current_task == "add_user_object")
  {

    if (_model != ContactModel::COULOMB)
    {
      auto var_params = _factory.getValidParams("LMWeightedGapUserObject");

      var_params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
      var_params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
      var_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      var_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      var_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
      var_params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
      var_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
      if (ndisp > 2)
        var_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};
      var_params.set<bool>("use_displaced_mesh") = true;
      var_params.set<std::vector<VariableName>>("lm_variable") = {normal_lagrange_multiplier_name};

      _problem->addUserObject("LMWeightedGapUserObject",
                              "lm_weightedgap_object_" +
                                  Moose::stringify(contact_userobject_counter),
                              var_params);
    }
    else if (_model == ContactModel::COULOMB)
    {
      auto var_params = _factory.getValidParams("LMWeightedVelocitiesUserObject");
      var_params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
      var_params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
      var_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      var_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      var_params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
      var_params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
      var_params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
      if (ndisp > 2)
        var_params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

      var_params.set<VariableName>("secondary_variable") = displacements[0];
      var_params.set<bool>("use_displaced_mesh") = true;
      var_params.set<std::vector<VariableName>>("lm_variable_normal") = {
          normal_lagrange_multiplier_name};
      var_params.set<std::vector<VariableName>>("lm_variable_tangential_one") = {
          tangential_lagrange_multiplier_name};
      if (ndisp > 2)
        var_params.set<std::vector<VariableName>>("lm_variable_tangential_two") = {
            tangential_lagrange_multiplier_3d_name};

      _problem->addUserObject("LMWeightedVelocitiesUserObject",
                              "lm_weightedvelocities_object_" +
                                  Moose::stringify(contact_userobject_counter),
                              var_params);
    }
  }

  if (_current_task == "add_constraint")
  {
    // Add the normal Lagrange multiplier constraint on the secondary boundary.

    // If no friction, only weighted gap class
    if (_model != ContactModel::COULOMB)
    {
      std::string mortar_constraint_name;

      if (!_mortar_dynamics)
        mortar_constraint_name = "ComputeWeightedGapLMMechanicalContact";
      else
        mortar_constraint_name = "ComputeDynamicWeightedGapLMMechanicalContact";

      InputParameters params = _factory.getValidParams(mortar_constraint_name);
      if (_mortar_dynamics)
      {
        params.set<Real>("newmark_beta") = getParam<Real>("newmark_beta");
        params.set<Real>("newmark_gamma") = getParam<Real>("newmark_gamma");
        params.set<Real>("capture_tolerance") = getParam<Real>("capture_tolerance");
        if (isParamValid("wear_depth"))
          params.set<CoupledName>("wear_depth") = getParam<CoupledName>("wear_depth");
      }
      else // We need user objects for quasistatic constraints
        params.set<UserObjectName>("weighted_gap_uo") =
            "lm_weightedgap_object_" +
            Moose::stringify(contact_userobject_counter - 1); // Adjust for order

      params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
      params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
      params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
      params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
      params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
      params.set<bool>("normalize_c") = getParam<bool>("normalize_c");
      params.set<Real>("c") = getParam<Real>("c_normal");

      if (ndisp > 1)
        params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
      if (ndisp > 2)
        params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

      params.set<bool>("use_displaced_mesh") = true;
      if (isParamValid("extra_vector_tags"))
        params.set<std::vector<TagName>>("extra_vector_tags") =
            getParam<std::vector<TagName>>("extra_vector_tags");

      _problem->addConstraint(
          mortar_constraint_name, action_name + "_normal_lm_weighted_gap", params);
      _problem->haveADObjects(true);
    }
    // Add the tangential and normal Lagrange's multiplier constraints on the secondary boundary.
    else if (_model == ContactModel::COULOMB)
    {
      std::string mortar_constraint_name;

      if (!_mortar_dynamics)
        mortar_constraint_name = "ComputeFrictionalForceLMMechanicalContact";
      else
        mortar_constraint_name = "ComputeDynamicFrictionalForceLMMechanicalContact";

      InputParameters params = _factory.getValidParams(mortar_constraint_name);
      if (_mortar_dynamics)
      {
        params.set<Real>("newmark_beta") = getParam<Real>("newmark_beta");
        params.set<Real>("newmark_gamma") = getParam<Real>("newmark_gamma");
        params.set<Real>("capture_tolerance") = getParam<Real>("capture_tolerance");
        if (isParamValid("wear_depth"))
          params.set<CoupledName>("wear_depth") = getParam<CoupledName>("wear_depth");
      }
      else
      { // We need user objects for quasistatic constraints
        params.set<UserObjectName>("weighted_gap_uo") =
            "lm_weightedvelocities_object_" +
            Moose::stringify(contact_userobject_counter - 1); // Adjust for order
        params.set<UserObjectName>("weighted_velocities_uo") =
            "lm_weightedvelocities_object_" +
            Moose::stringify(contact_userobject_counter - 1); // Adjust for order
      }

      params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
      params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
      params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
      params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      params.set<bool>("use_displaced_mesh") = true;
      params.set<Real>("c_t") = getParam<Real>("c_tangential");
      params.set<Real>("c") = getParam<Real>("c_normal");
      params.set<bool>("normalize_c") = getParam<bool>("normalize_c");
      params.set<bool>("compute_primal_residuals") = false;

      params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};

      if (ndisp > 1)
        params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
      if (ndisp > 2)
        params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

      params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
      params.set<std::vector<VariableName>>("friction_lm") = {tangential_lagrange_multiplier_name};

      if (ndisp > 2)
        params.set<std::vector<VariableName>>("friction_lm_dir") = {
            tangential_lagrange_multiplier_3d_name};

      params.set<Real>("mu") = getParam<Real>("friction_coefficient");
      if (isParamValid("extra_vector_tags"))
        params.set<std::vector<TagName>>("extra_vector_tags") =
            getParam<std::vector<TagName>>("extra_vector_tags");

      _problem->addConstraint(mortar_constraint_name, action_name + "_tangential_lm", params);
      _problem->haveADObjects(true);
    }
    const auto addMechanicalContactConstraints =
        [this, &primary_subdomain_name, &secondary_subdomain_name, &displacements](
            const std::string & variable_name,
            const std::string & constraint_prefix,
            const std::string & constraint_type,
            const bool is_additional_frictional_constraint,
            const bool is_normal_constraint)
    {
      InputParameters params = _factory.getValidParams(constraint_type);

      params.set<bool>("correct_edge_dropping") = getParam<bool>("correct_edge_dropping");
      params.set<BoundaryName>("primary_boundary") = _boundary_pairs[0].first;
      params.set<BoundaryName>("secondary_boundary") = _boundary_pairs[0].second;
      params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      params.set<NonlinearVariableName>("variable") = variable_name;
      params.set<bool>("use_displaced_mesh") = true;
      params.set<bool>("compute_lm_residuals") = false;

      // Additional displacement residual for frictional problem
      // The second frictional LM acts on a perpendicular direction.
      if (is_additional_frictional_constraint)
        params.set<MooseEnum>("direction") = "direction_2";
      if (isParamValid("extra_vector_tags"))
        params.set<std::vector<TagName>>("extra_vector_tags") =
            getParam<std::vector<TagName>>("extra_vector_tags");

      for (unsigned int i = 0; i < displacements.size(); ++i)
      {
        std::string constraint_name = constraint_prefix + Moose::stringify(i);

        params.set<VariableName>("secondary_variable") = displacements[i];
        params.set<MooseEnum>("component") = i;

        if (is_normal_constraint && _model != ContactModel::COULOMB)
          params.set<UserObjectName>("weighted_gap_uo") =
              "lm_weightedgap_object_" +
              Moose::stringify(contact_userobject_counter - 1); // Adjust for order
        else if (is_normal_constraint && _model == ContactModel::COULOMB)
          params.set<UserObjectName>("weighted_gap_uo") =
              "lm_weightedvelocities_object_" +
              Moose::stringify(contact_userobject_counter - 1); // Adjust for order
        else
          params.set<UserObjectName>("weighted_velocities_uo") =
              "lm_weightedvelocities_object_" +
              Moose::stringify(contact_userobject_counter - 1); // Adjust for order

        _problem->addConstraint(constraint_type, constraint_name, params);
      }
      _problem->haveADObjects(true);
    };

    // Add mortar mechanical contact constraint objects for primal variables
    addMechanicalContactConstraints(normal_lagrange_multiplier_name,
                                    action_name + "_normal_constraint_",
                                    "NormalMortarMechanicalContact",
                                    false,
                                    true);

    if (_model == ContactModel::COULOMB)
    {
      addMechanicalContactConstraints(tangential_lagrange_multiplier_name,
                                      action_name + "_tangential_constraint_",
                                      "TangentialMortarMechanicalContact",
                                      false,
                                      false);
      if (ndisp > 2)
        addMechanicalContactConstraints(tangential_lagrange_multiplier_3d_name,
                                        action_name + "_tangential_constraint_3d_",
                                        "TangentialMortarMechanicalContact",
                                        true,
                                        false);
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

  const auto order =
      _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();

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
      if (isParamValid("extra_vector_tags"))
        params.set<std::vector<TagName>>("extra_vector_tags") =
            getParam<std::vector<TagName>>("extra_vector_tags");
      _problem->addConstraint(constraint_type, name, params);
    }
  }
}

MooseEnum
ContactAction::getModelEnum()
{
  return MooseEnum("frictionless glued coulomb", "frictionless");
}

MooseEnum
ContactAction::getFormulationEnum()
{
  return MooseEnum("ranfs kinematic penalty augmented_lagrange tangential_penalty mortar",
                   "kinematic");
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
