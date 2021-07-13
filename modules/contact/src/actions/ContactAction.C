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

// Counter for naming auxiliary kernels
static unsigned int contact_auxkernel_counter = 0;

// Counter for naming nodal area user objects
static unsigned int contact_userobject_counter = 0;

registerMooseAction("ContactApp", ContactAction, "add_aux_kernel");
registerMooseAction("ContactApp", ContactAction, "add_aux_variable");
registerMooseAction("ContactApp", ContactAction, "add_constraint"); // for mortar constraint
registerMooseAction("ContactApp", ContactAction, "add_dirac_kernel");
registerMooseAction("ContactApp", ContactAction, "add_mesh_generator"); // for mortar subdomains
registerMooseAction("ContactApp",
                    ContactAction,
                    "add_mortar_variable"); // for mortar lagrange multiplier
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
  params.addParam<MeshGeneratorName>("mesh", "", "The mesh generator for mortar method");
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
  params.addParam<Real>(
      "capture_tolerance", 0.0, "Normal distance from surface within which nodes are captured");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");

  params.addDeprecatedParam<MooseEnum>(
      "system",
      ContactAction::getSystemEnum(),
      "System to use for constraint enforcement",
      "The only available system in the contact action is constraint");
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
      "Parameter for balancing the size of the gap and contact pressure. This purely numerical "
      "parameter affects convergence behavior and, in general, should be larger for stiffer "
      "materials. It is recommended that the user tries out various orders of magnitude for this "
      "parameter if the default value generates poor contact convergence.");
  params.addParam<Real>(
      "c_tangential", 1, "Numerical parameter for nonlinear frictional constraints");
  params.addParam<bool>("ping_pong_protection",
                        false,
                        "Whether to protect against ping-ponging, e.g. the oscillation of the "
                        "secondary node between two "
                        "different primary faces, by tying the secondary node to the "
                        "edge between the involved primary faces");
  params.addParam<Real>(
      "normal_lm_scaling", 1., "Scaling factor to apply to the normal LM variable");
  params.addParam<Real>(
      "tangential_lm_scaling", 1., "Scaling factor to apply to the tangential LM variable");
  params.addParam<bool>(
      "interpolate_normals",
      true,
      "Whether to interpolate the nodal normals (e.g. classic idea of evaluating field at "
      "quadrature points). If this is set to false, then non-interpolated nodal normals will be "
      "used, and then the _normals member should be indexed with _i instead of _qp. This input "
      "parameter is intended for developers.");
  params.addParam<MooseEnum>("mortar_approach",
                             ContactAction::getMortarApproach(),
                             "Whether to choose a variationally consistent mortar approach "
                             "'weighted' or a mixed approach 'legacy' ");
  params.addClassDescription("Sets up all objects needed for mechanical contact enforcement");
  params.addParam<bool>(
      "use_dual",
      true,
      "Whether to use the dual mortar approach. It is defaulted to true for "
      "weighted quantity approach, and to false for the legacy approach. To avoid instabilities "
      "in the solution and obtain the full benefits of a variational enforcement,"
      "use of dual mortar with weighted constraints is strongly recommended. This "
      "input is only intended for advanced users.");

  return params;
}

ContactAction::ContactAction(const InputParameters & params)
  : Action(params),
    _primary(getParam<std::vector<BoundaryName>>("primary")),
    _secondary(getParam<std::vector<BoundaryName>>("secondary")),
    _model(getParam<MooseEnum>("model")),
    _formulation(getParam<MooseEnum>("formulation")),
    _system(getParam<MooseEnum>("system")),
    _mesh_gen_name(getParam<MeshGeneratorName>("mesh")),
    _mortar_approach(getParam<MooseEnum>("mortar_approach").getEnum<MortarApproach>()),
    _use_dual(getParam<bool>("use_dual")),
    _number_pairs(_primary.size())
{

  if (_primary.size() != _secondary.size())
    paramError("primary",
               "Sizes of 'primary' and 'secondary' arrays in contact action's input must match");

  if (_primary.size() != 1 && _formulation == "mortar")
    paramError("formulation", "When using mortar, a vector of contact pairs cannot be used");

  if (_formulation == "tangential_penalty")
  {
    if (_system != "Constraint")
      paramError(
          "formulation",
          "The 'tangential_penalty' formulation can only be used with the 'Constraint' system");
    if (_model != "coulomb")
      paramError("formulation",
                 "The 'tangential_penalty' formulation can only be used with the 'coulomb' model");
  }

  if (_formulation == "mortar")
  {
    if (_system != "constraint")
      paramError("formulation",
                 "The 'mortar' formulation can only be used with the 'Constraint' system");
    if (_mesh_gen_name.empty())
      paramError("mesh", "The 'mortar' formulation requires 'mesh' to be supplied");
    if (_model == "glued")
      paramError("model", "The 'mortar' formulation does not support glued contact (yet)");

    if (_mortar_approach == MortarApproach::Legacy)
    {
      mooseDeprecated(
          "Use of legacy mortar contact approach is deprecated and will be removed by December "
          "2021. Instead, select the default option based on weighted quantities and dual bases");

      // If user does not specify whether to use dual bases, standard bases will be used with the
      // `legacy` option.
      if (!params.isParamSetByUser("use_dual"))
        _use_dual = false;
    }
  }
  else if (params.isParamSetByUser("mortar_approach"))
    paramError("mortar_approach",
               "The 'mortar_approach' option can only be used with the 'mortar' formulation");
  else if (params.isParamSetByUser("use_dual"))
    paramError("use_dual", "The 'use_dual' option can only be used with the 'mortar' formulation");

  if (_formulation != "ranfs")
    if (getParam<bool>("ping_pong_protection"))
      paramError("ping_pong_protection",
                 "The 'ping_pong_protection' option can only be used with the 'ranfs' formulation");

  if (_formulation == "ranfs")
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
}

void
ContactAction::act()
{
  if (_formulation == "mortar")
    // This method executes multiple tasks
    addMortarContact();

  if (_current_task == "add_dirac_kernel")
  {
    // It is risky to apply this optimization to contact problems
    // since the problem configuration may be changed during Jacobian
    // evaluation. We therefore turn it off for all contact problems so that
    // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3.
    if (!_problem->isSNESMFReuseBaseSetbyUser())
      _problem->setSNESMFReuseBase(false, false);

    if (!_problem->getDisplacedProblem())
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");

    if (_formulation != "mortar")
      addNodeFaceContact();
  }

  if (_current_task == "add_aux_kernel")
  { // Add ContactPenetrationAuxAction.
    if (!_problem->getDisplacedProblem())
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");
    // Create auxiliary kernels for each contact pairs
    for (unsigned int contact_pair = 0; contact_pair < _number_pairs; contact_pair++)
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
        params.set<std::vector<BoundaryName>>("boundary") = {_secondary[contact_pair]};
        params.set<BoundaryName>("paired_boundary") = _primary[contact_pair];
        params.set<AuxVariableName>("variable") = "penetration";
        if (isParamValid("secondary_gap_offset"))
          params.set<std::vector<VariableName>>("secondary_gap_offset") = {
              getParam<VariableName>("secondary_gap_offset")};
        if (isParamValid("mapped_primary_gap_offset"))
          params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
              getParam<VariableName>("mapped_primary_gap_offset")};
        params.set<bool>("use_displaced_mesh") = true;
        std::string name = _name + "_contact_" + Moose::stringify(contact_auxkernel_counter);

        _problem->addAuxKernel("PenetrationAux", name, params);
      }
      // Add ContactPressureAuxAction
      {
        InputParameters params = _factory.getValidParams("ContactPressureAux");
        params.applyParameters(parameters(), {"order"});

        std::vector<VariableName> displacements =
            getParam<std::vector<VariableName>>("displacements");
        const auto order = _problem->systemBaseNonlinear()
                               .system()
                               .variable_type(displacements[0])
                               .order.get_order();

        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
        params.set<std::vector<BoundaryName>>("boundary") = {_secondary[contact_pair]};
        params.set<BoundaryName>("paired_boundary") = _primary[contact_pair];
        params.set<AuxVariableName>("variable") = "contact_pressure";
        params.addRequiredCoupledVar("nodal_area", "The nodal area");
        params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + _name};
        params.set<bool>("use_displaced_mesh") = true;

        std::string name =
            _name + "_contact_pressure_" + Moose::stringify(contact_auxkernel_counter++);

        params.set<ExecFlagEnum>("execute_on",
                                 true) = {EXEC_NONLINEAR, EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN};
        _problem->addAuxKernel("ContactPressureAux", name, params);
      }
    }
  }

  if (_current_task == "add_aux_variable")
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

      _problem->addAuxVariable("MooseVariable", "nodal_area_" + _name, var_params);
    }
  }

  if (_current_task == "add_user_object")
  {
    auto var_params = _factory.getValidParams("NodalArea");
    var_params.set<std::vector<BoundaryName>>("boundary") = _secondary;
    var_params.set<std::vector<VariableName>>("variable") = {"nodal_area_" + _name};

    mooseAssert(_problem, "Problem pointer is NULL");
    var_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    var_params.set<bool>("use_displaced_mesh") = true;

    _problem->addUserObject("NodalArea",
                            "nodal_area_object_" + Moose::stringify(contact_userobject_counter++),
                            var_params);
  }
}

void
ContactAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  if (_formulation == "mortar")
    addRelationshipManagers(input_rm_type, _pars);
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

  if (_current_task == "add_mesh_generator")
  {

    // Don't do mesh generators when recovering.
    if (!(_app.isRecovering() && _app.isUltimateMaster()) && !_app.masterMesh())
    {
      const MeshGeneratorName primary_name = primary_subdomain_name + "_generator";
      const MeshGeneratorName secondary_name = secondary_subdomain_name + "_generator";

      auto primary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
      auto secondary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");

      primary_params.set<MeshGeneratorName>("input") = _mesh_gen_name;
      secondary_params.set<MeshGeneratorName>("input") = primary_name;

      primary_params.set<SubdomainName>("new_block_name") = primary_subdomain_name;
      secondary_params.set<SubdomainName>("new_block_name") = secondary_subdomain_name;

      primary_params.set<std::vector<BoundaryName>>("sidesets") = {_primary[0]};
      secondary_params.set<std::vector<BoundaryName>>("sidesets") = {_secondary[0]};

      _app.addMeshGenerator("LowerDBlockFromSidesetGenerator", primary_name, primary_params);
      _app.addMeshGenerator("LowerDBlockFromSidesetGenerator", secondary_name, secondary_params);
    }
  }

  if (_current_task == "add_mortar_variable")
  {
    // Add the lagrange multiplier on the secondary subdomain.
    const auto addLagrangeMultiplier =
        [this, &secondary_subdomain_name, &displacements](const std::string & variable_name,
                                                          const int min_lm_order,
                                                          const bool frictional,
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
      const int codimension = frictional && !(_mortar_approach == MortarApproach::Weighted);
      // Order of LM is independent of whether it enforces normal contact or frictional contact.
      const int lm_order = std::max(primal_type.order.get_order() - codimension, min_lm_order);

      if (primal_type.family == LAGRANGE && lm_order < 1)
      {
        params.set<MooseEnum>("family") = "MONOMIAL";
        params.set<MooseEnum>("order") = "CONSTANT";
      }
      else if (primal_type.family == LAGRANGE)
      {
        params.set<MooseEnum>("family") = Utility::enum_to_string<FEFamily>(primal_type.family);
        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{lm_order});
      }
      else
        mooseError("Invalid bases for mortar contact.");

      params.set<std::vector<SubdomainName>>("block") = {secondary_subdomain_name};
      params.set<std::vector<Real>>("scaling") = {scaling_factor};
      auto fe_type = AddVariableAction::feType(params);
      auto var_type = AddVariableAction::determineType(fe_type, 1);
      _problem->addVariable(var_type, variable_name, params);
    };

    // Normal contact:
    //   Lagrange family with order one less than primal, but by necessity with a min
    //   order of 1 (we don't have zeroth order Lagrange). We must use a Lagrange basis because we
    //   need dofs at nodes in order to enforce the zero-penetration constraint
    //
    addLagrangeMultiplier(
        normal_lagrange_multiplier_name, 1, false, getParam<Real>("normal_lm_scaling"));

    // Tangential contact:
    //    For standard Mortar: one order lower than primal, Lagrange family unless zeroth order,
    //    than MONOMIAL. For dual Mortar: same family, equal order as primal, Lagrange family.
    if (_model == "coulomb" && _mortar_approach == MortarApproach::Weighted)
      addLagrangeMultiplier(
          tangential_lagrange_multiplier_name, 1, true, getParam<Real>("tangential_lm_scaling"));
    else if (_model == "coulomb" && _mortar_approach == MortarApproach::Legacy)
      addLagrangeMultiplier(
          tangential_lagrange_multiplier_name, 0, true, getParam<Real>("tangential_lm_scaling"));
  }

  if (_current_task == "add_constraint")
  {
    // Add the normal Lagrange multiplier constraint on the secondary boundary.

    // If no friction, only weighted gap class
    if (_model != "coulomb")
    {
      if (_mortar_approach == MortarApproach::Weighted)
      {
        InputParameters params = _factory.getValidParams("ComputeWeightedGapLMMechanicalContact");

        params.set<BoundaryName>("primary_boundary") = _primary[0];
        params.set<BoundaryName>("secondary_boundary") = _secondary[0];
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        params.set<bool>("interpolate_normals") = getParam<bool>("interpolate_normals");

        params.set<Real>("c") = getParam<Real>("c_normal");

        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        params.set<bool>("use_displaced_mesh") = true;

        _problem->addConstraint("ComputeWeightedGapLMMechanicalContact",
                                action_name + "_normal_lm_weighted_gap",
                                params);
        _problem->haveADObjects(true);
      }
      else if (_mortar_approach == MortarApproach::Legacy)
      {
        InputParameters params = _factory.getValidParams("NormalNodalLMMechanicalContact");

        params.set<BoundaryName>("primary") = _primary[0];
        params.set<BoundaryName>("secondary") = _secondary[0];
        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<bool>("use_displaced_mesh") = true;
        params.set<MooseEnum>("ncp_function_type") = "min";
        params.set<Real>("c") = getParam<Real>("c_normal");
        if (_pars.isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = _pars.get<Real>("tangential_tolerance");

        params.set<std::vector<VariableName>>("primary_variable") = {displacements[0]};
        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        _problem->addConstraint(
            "NormalNodalLMMechanicalContact", action_name + "_normal_lm", params);
      }
    }
    // Add the tangential and normal Lagrange's multiplier constraints on the secondary boundary.
    else if (_model == "coulomb")
    {
      if (_mortar_approach == MortarApproach::Weighted)
      {
        InputParameters params =
            _factory.getValidParams("ComputeFrictionalForceLMMechanicalContact");

        params.set<BoundaryName>("primary_boundary") = _primary[0];
        params.set<BoundaryName>("secondary_boundary") = _secondary[0];
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        params.set<bool>("use_displaced_mesh") = true;
        params.set<Real>("c_t") = getParam<Real>("c_tangential");
        params.set<Real>("c") = getParam<Real>("c_normal");
        params.set<bool>("compute_primal_residuals") = false;

        params.set<std::vector<VariableName>>("disp_x") = {displacements[0]};
        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};

        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<std::vector<VariableName>>("friction_lm") = {
            tangential_lagrange_multiplier_name};
        params.set<Real>("mu") = getParam<Real>("friction_coefficient");
        // secondary_disp_z is not implemented for tangential (yet).

        _problem->addConstraint(
            "ComputeFrictionalForceLMMechanicalContact", action_name + "_tangential_lm", params);
        _problem->haveADObjects(true);
      }
      else if (_mortar_approach == MortarApproach::Legacy)
      {
        InputParameters params = _factory.getValidParams("NormalNodalLMMechanicalContact");

        params.set<BoundaryName>("primary") = _primary[0];
        params.set<BoundaryName>("secondary") = _secondary[0];
        params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
        params.set<bool>("use_displaced_mesh") = true;
        params.set<MooseEnum>("ncp_function_type") = "min";
        params.set<Real>("c") = getParam<Real>("c_normal");
        if (_pars.isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = _pars.get<Real>("tangential_tolerance");

        params.set<std::vector<VariableName>>("primary_variable") = {displacements[0]};
        if (ndisp > 1)
          params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
        if (ndisp > 2)
          params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

        _problem->addConstraint(
            "NormalNodalLMMechanicalContact", action_name + "_normal_lm", params);

        params = _factory.getValidParams("TangentialMortarLMMechanicalContact");

        params.set<BoundaryName>("primary_boundary") = _primary[0];
        params.set<BoundaryName>("secondary_boundary") = _secondary[0];
        params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
        params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
        params.set<NonlinearVariableName>("variable") = tangential_lagrange_multiplier_name;
        params.set<bool>("use_displaced_mesh") = true;
        params.set<MooseEnum>("ncp_function_type") = "fb";
        params.set<Real>("c") = getParam<Real>("c_tangential");
        params.set<bool>("compute_primal_residuals") = false;
        params.set<NonlinearVariableName>("contact_pressure") = normal_lagrange_multiplier_name;
        params.set<Real>("friction_coefficient") = getParam<Real>("friction_coefficient");

        params.set<VariableName>("secondary_variable") = displacements[0];
        if (ndisp > 1)
          params.set<NonlinearVariableName>("secondary_disp_y") = displacements[1];
        // secondary_disp_z is not implemented for tangential (yet).

        _problem->addConstraint(
            "TangentialMortarLMMechanicalContact", action_name + "_tangential_lm", params);
      }
    }
    const auto addMechanicalContactConstraints =
        [this, &primary_subdomain_name, &secondary_subdomain_name, &displacements](
            const std::string & variable_name,
            const std::string & constraint_prefix,
            const std::string & constraint_type) //
    {
      InputParameters params = _factory.getValidParams(constraint_type);

      params.set<BoundaryName>("primary_boundary") = _primary[0];
      params.set<BoundaryName>("secondary_boundary") = _secondary[0];
      params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
      params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;
      params.set<NonlinearVariableName>("variable") = variable_name;
      params.set<bool>("use_displaced_mesh") = true;
      params.set<bool>("compute_lm_residuals") = false;

      for (unsigned int i = 0; i < displacements.size(); ++i)
      {
        std::string constraint_name = constraint_prefix + Moose::stringify(i);

        params.set<VariableName>("secondary_variable") = displacements[i];
        params.set<MooseEnum>("component") = i;

        _problem->addConstraint(constraint_type, constraint_name, params);
      }
      _problem->haveADObjects(true);
    };

    // Add mortar mechanical contact constraint objects for primal variables
    addMechanicalContactConstraints(normal_lagrange_multiplier_name,
                                    action_name + "_normal_constraint_",
                                    "NormalMortarMechanicalContact");
    if (_model == "coulomb")
      addMechanicalContactConstraints(tangential_lagrange_multiplier_name,
                                      action_name + "_tangential_constraint_",
                                      "TangentialMortarMechanicalContact");
  }
}

void
ContactAction::addNodeFaceContact()
{
  std::string action_name = MooseUtils::shortName(name());
  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  const unsigned int ndisp = displacements.size();

  std::string constraint_type;

  if (_formulation == "ranfs")
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

  for (unsigned int contact_pair = 0; contact_pair < _number_pairs; contact_pair++)
  {
    if (_formulation != "ranfs")
    {
      params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + name()};
      params.set<BoundaryName>("boundary") = _primary[contact_pair];
      if (isParamValid("secondary_gap_offset"))
        params.set<std::vector<VariableName>>("secondary_gap_offset") = {
            getParam<VariableName>("secondary_gap_offset")};
      if (isParamValid("mapped_primary_gap_offset"))
        params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
            getParam<VariableName>("mapped_primary_gap_offset")};
    }

    for (unsigned int i = 0; i < ndisp; ++i)
    {
      std::string name =
          action_name + "_constraint_" + Moose::stringify(contact_pair) + "_" + Moose::stringify(i);

      if (_formulation == "ranfs")
        params.set<MooseEnum>("component") = i;
      else
        params.set<unsigned int>("component") = i;

      params.set<BoundaryName>("primary") = _primary[contact_pair];
      params.set<BoundaryName>("secondary") = _secondary[contact_pair];
      params.set<NonlinearVariableName>("variable") = displacements[i];
      params.set<std::vector<VariableName>>("primary_variable") = {displacements[i]};
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
ContactAction::getMortarApproach()
{
  return MooseEnum("weighted legacy", "weighted");
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
