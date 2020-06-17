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

  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary surface");

  params.addParam<MeshGeneratorName>("mesh", "", "The mesh generator for mortar method");

  params.addParam<VariableName>("disp_x", "The x displacement");
  params.addParam<VariableName>("disp_y", "The y displacement");
  params.addParam<VariableName>("disp_z", "The z displacement");

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
  params.addParam<bool>("master_secondary_jacobian",
                        true,
                        "Whether to include Jacobian entries coupling master and secondary nodes.");
  params.addParam<Real>("al_penetration_tolerance",
                        "The tolerance of the penetration for augmented Lagrangian method.");
  params.addParam<Real>("al_incremental_slip_tolerance",
                        "The tolerance of the incremental slip for augmented Lagrangian method.");

  params.addParam<Real>("al_frictional_force_tolerance",
                        "The tolerance of the frictional force for augmented Lagrangian method.");
  params.addParam<Real>(
      "c_normal", 1, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<Real>(
      "c_tangential", 1, "Parameter for balancing the contact pressure and velocity");
  params.addParam<bool>(
      "ping_pong_protection",
      false,
      "Whether to protect against ping-ponging, e.g. the oscillation of the secondary node between two "
      "different master faces, by tying the secondary node to the "
      "edge between the involved master faces");
  params.addParam<Real>(
      "normal_lm_scaling", 1., "Scaling factor to apply to the normal LM variable");
  params.addParam<Real>(
      "tangential_lm_scaling", 1., "Scaling factor to apply to the tangential LM variable");
  params.addParam<bool>("use_dual", false, "Whether to use the dual mortar approach");

  params.addClassDescription("Sets up all objects needed for mechanical contact enforcement");

  return params;
}

ContactAction::ContactAction(const InputParameters & params)
  : Action(params),
    _master(getParam<BoundaryName>("master")),
    _secondary(getParam<BoundaryName>("secondary")),
    _model(getParam<MooseEnum>("model")),
    _formulation(getParam<MooseEnum>("formulation")),
    _system(getParam<MooseEnum>("system")),
    _mesh_gen_name(getParam<MeshGeneratorName>("mesh")),
    _use_dual(getParam<bool>("use_dual"))
{

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
  }
  else if (_use_dual)
    paramError("use_dual", "The 'use_dual' option can only be used with the 'mortar' formulation");

  if (_formulation != "ranfs")
    if (getParam<bool>("ping_pong_protection"))
      paramError("ping_pong_protection",
                 "The 'ping_pong_protection' option can only be used with the 'ranfs' formulation");
}

void
ContactAction::act()
{
  if (_formulation == "mortar")
    addMortarContact();
  else
  {
    if (_current_task == "add_dirac_kernel")
    {
      // It is risky to apply this optimization to contact problems
      // since the problem configuration may be changed during Jacobian
      // evaluation. We therefore turn it off for all contact problems so that
      // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3.
      if (!_problem->isSNESMFReuseBaseSetbyUser())
        _problem->setSNESMFReuseBase(false, false);

      if (!_problem->getDisplacedProblem())
        mooseError(
            "Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
            "Mesh block.");

      if (_system == "Constraint")
        // MechanicalContactConstraint has to be added after the init_problem task, so it cannot
        // be added for the add_constraint task.
        addNodeFaceContact();
    }
  }

  if (_current_task == "add_aux_kernel")
  { // Add ContactPenetrationAuxAction.
    if (!_problem->getDisplacedProblem())
    {
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");
    }

    {
      InputParameters params = _factory.getValidParams("PenetrationAux");
      params.applyParameters(parameters());
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
      params.set<std::vector<BoundaryName>>("boundary") = {_secondary};
      params.set<BoundaryName>("paired_boundary") = _master;
      params.set<AuxVariableName>("variable") = "penetration";

      params.set<bool>("use_displaced_mesh") = true;
      std::string name = _name + "_contact_" + Moose::stringify(contact_auxkernel_counter);

      _problem->addAuxKernel("PenetrationAux", name, params);
    }
    // Add ContactPressureAuxAction
    {
      InputParameters params = _factory.getValidParams("ContactPressureAux");
      params.applyParameters(parameters());

      params.set<std::vector<BoundaryName>>("boundary") = {_secondary};
      params.set<BoundaryName>("paired_boundary") = _master;
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

  if (_current_task == "add_aux_variable")
  {
    // Add ContactPenetrationVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "penetration", var_params);
    }
    // Add ContactPressureVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "contact_pressure", var_params);
    }
    // Add nodal area contact variable
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "nodal_area_" + _name, var_params);
    }
  }

  if (_current_task == "add_user_object")
  {
    auto var_params = _factory.getValidParams("NodalArea");
    var_params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};
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

  std::vector<VariableName> displacements = getDisplacementVarNames();
  const unsigned int ndisp = displacements.size();

  // Definitions for mortar contact.
  const std::string master_subdomain_name = action_name + "_master_subdomain";
  const std::string secondary_subdomain_name = action_name + "_secondary_subdomain";
  const std::string normal_lagrange_multiplier_name = action_name + "_normal_lm";
  const std::string tangential_lagrange_multiplier_name = action_name + "_tangential_lm";

  if (_current_task == "add_mesh_generator")
  {
    // Don't do mesh generators when recovering.
    if (!(_app.isRecovering() && _app.isUltimateMaster()) && !_app.masterMesh())
    {
      const MeshGeneratorName master_name = master_subdomain_name + "_generator";
      const MeshGeneratorName secondary_name = secondary_subdomain_name + "_generator";

      auto master_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
      auto secondary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");

      master_params.set<MeshGeneratorName>("input") = _mesh_gen_name;
      secondary_params.set<MeshGeneratorName>("input") = master_name;

      master_params.set<SubdomainName>("new_block_name") = master_subdomain_name;
      secondary_params.set<SubdomainName>("new_block_name") = secondary_subdomain_name;

      master_params.set<std::vector<BoundaryName>>("sidesets") = {_master};
      secondary_params.set<std::vector<BoundaryName>>("sidesets") = {_secondary};

      _app.addMeshGenerator("LowerDBlockFromSidesetGenerator", master_name, master_params);
      _app.addMeshGenerator("LowerDBlockFromSidesetGenerator", secondary_name, secondary_params);
    }
  }

  if (_current_task == "add_mortar_variable")
  {
    // Add the lagrange multiplier on the secondary subdomain.
    const auto addLagrangeMultiplier =
        [this, &secondary_subdomain_name, &displacements](const std::string & variable_name,
                                                      const int codimension,
                                                      const Real scaling_factor) //
    {
      InputParameters params = _factory.getValidParams("MooseVariableBase");
      params.set<bool>("use_dual") = _use_dual;

      mooseAssert(_problem->systemBaseNonlinear().hasVariable(displacements[0]),
                  "Displacement variable is missing");
      const auto primal_type =
          _problem->systemBaseNonlinear().system().variable_type(displacements[0]);
      const int lm_order = std::max(primal_type.order.get_order() - codimension, 0);

      if (primal_type.family == MONOMIAL || (primal_type.family == LAGRANGE && lm_order < 1))
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
        mooseError("Primal variable type must be either MONOMIAL or LAGRANGE for mortar contact.");

      params.set<std::vector<SubdomainName>>("block") = {secondary_subdomain_name};
      params.set<std::vector<Real>>("scaling") = {scaling_factor};
      auto fe_type = AddVariableAction::feType(params);
      auto var_type = AddVariableAction::determineType(fe_type, 1);
      _problem->addVariable(var_type, variable_name, params);
    };

    // Normal contact: same family/order as primal.
    // Tangential contact:
    //    For standard Mortar: same family, one order lower.
    //    For dual Mortar: same family, equal order as primal.
    addLagrangeMultiplier(normal_lagrange_multiplier_name, 0, getParam<Real>("normal_lm_scaling"));
    if (_model == "coulomb")
    {
      if (_use_dual)
        addLagrangeMultiplier(
            tangential_lagrange_multiplier_name, 0, getParam<Real>("tangential_lm_scaling"));
      else
        addLagrangeMultiplier(
            tangential_lagrange_multiplier_name, 1, getParam<Real>("tangential_lm_scaling"));
    }
  }

  if (_current_task == "add_constraint")
  {
    // Add the normal Lagrange multiplier constraint on the secondary boundary.
    {
      InputParameters params = _factory.getValidParams("NormalNodalLMMechanicalContact");

      params.set<BoundaryName>("master") = _master;
      params.set<BoundaryName>("secondary") = _secondary;
      params.set<NonlinearVariableName>("variable") = normal_lagrange_multiplier_name;
      params.set<bool>("use_displaced_mesh") = true;
      params.set<MooseEnum>("ncp_function_type") = "min";
      params.set<Real>("c") = getParam<Real>("c_normal");
      if (_pars.isParamValid("tangential_tolerance"))
        params.set<Real>("tangential_tolerance") = _pars.get<Real>("tangential_tolerance");

      params.set<std::vector<VariableName>>("master_variable") = {displacements[0]};
      if (ndisp > 1)
        params.set<std::vector<VariableName>>("disp_y") = {displacements[1]};
      if (ndisp > 2)
        params.set<std::vector<VariableName>>("disp_z") = {displacements[2]};

      _problem->addConstraint("NormalNodalLMMechanicalContact", action_name + "_normal_lm", params);
    }

    // Add the tangential Lagrange multiplier constraint on the secondary boundary.
    if (_model == "coulomb")
    {
      InputParameters params = _factory.getValidParams("TangentialMortarLMMechanicalContact");

      params.set<BoundaryName>("master_boundary") = _master;
      params.set<BoundaryName>("secondary_boundary") = _secondary;
      params.set<SubdomainName>("master_subdomain") = master_subdomain_name;
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

    const auto addMechanicalContactConstraints =
        [this, &master_subdomain_name, &secondary_subdomain_name, &displacements](
            const std::string & variable_name,
            const std::string & constraint_prefix,
            const std::string & constraint_type) //
    {
      InputParameters params = _factory.getValidParams(constraint_type);

      params.set<BoundaryName>("master_boundary") = _master;
      params.set<BoundaryName>("secondary_boundary") = _secondary;
      params.set<SubdomainName>("master_subdomain") = master_subdomain_name;
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

    // Add mortar mechanical contact for each dimension
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

  std::vector<VariableName> displacements = getDisplacementVarNames();
  const unsigned int ndisp = displacements.size();

  std::string constraint_type;

  if (_formulation == "ranfs")
    constraint_type = "RANFSNormalMechanicalContact";
  else
    constraint_type = "MechanicalContactConstraint";

  InputParameters params = _factory.getValidParams(constraint_type);

  params.applyParameters(parameters(), {"displacements"});
  params.set<std::vector<VariableName>>("displacements") = displacements;
  params.set<bool>("use_displaced_mesh") = true;

  if (_formulation != "ranfs")
  {
    params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + name()};
    params.set<BoundaryName>("boundary") = _master;
  }

  for (unsigned int i = 0; i < ndisp; ++i)
  {
    std::string name = action_name + "_constraint_" + Moose::stringify(i);

    if (_formulation == "ranfs")
      params.set<MooseEnum>("component") = i;
    else
      params.set<unsigned int>("component") = i;

    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<VariableName>>("master_variable") = {displacements[i]};
    _problem->addConstraint(constraint_type, name, params);
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

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");

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

std::vector<VariableName>
ContactAction::getDisplacementVarNames()
{
  std::vector<VariableName> displacements;
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<VariableName>>("displacements");
  else
  {
    // Legacy parameter scheme for displacements
    if (!isParamValid("disp_x"))
      mooseError("Specify displacement variables using the `displacements` parameter.");
    displacements.push_back(getParam<VariableName>("disp_x"));

    if (isParamValid("disp_y"))
    {
      displacements.push_back(getParam<VariableName>("disp_y"));
      if (isParamValid("disp_z"))
        displacements.push_back(getParam<VariableName>("disp_z"));
    }

    mooseDeprecated("use the `displacements` parameter rather than the `disp_*` parameters (those "
                    "will go away with the deprecation of the Solid Mechanics module).");
  }

  return displacements;
}
