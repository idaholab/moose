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
#include "NonlinearSystemBase.h"
#include "libmesh/petsc_nonlinear_solver.h"

registerMooseAction("ContactApp", ContactAction, "add_aux_kernel");

registerMooseAction("ContactApp", ContactAction, "add_aux_variable");

registerMooseAction("ContactApp", ContactAction, "add_dirac_kernel");

registerMooseAction("ContactApp", ContactAction, "output_penetration_info_vars");

template <>
InputParameters
validParams<ContactAction>()
{
  InputParameters params = validParams<Action>();
  params += ContactAction::commonParameters();

  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");

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

  params.addParam<MooseEnum>(
      "system", ContactAction::getSystemEnum(), "System to use for constraint enforcement");
  params.addParam<bool>("normalize_penalty",
                        false,
                        "Whether to normalize the penalty parameter with the nodal area.");
  params.addParam<bool>("master_slave_jacobian",
                        true,
                        "Whether to include Jacobian entries coupling master and slave nodes.");
  params.addParam<Real>("al_penetration_tolerance",
                        "The tolerance of the penetration for augmented Lagrangian method.");
  params.addParam<Real>("al_incremental_slip_tolerance",
                        "The tolerance of the incremental slip for augmented Lagrangian method.");

  params.addParam<Real>("al_frictional_force_tolerance",
                        "The tolerance of the frictional force for augmented Lagrangian method.");
  return params;
}

ContactAction::ContactAction(const InputParameters & params)
  : Action(params),
    _master(getParam<BoundaryName>("master")),
    _slave(getParam<BoundaryName>("slave")),
    _model(getParam<MooseEnum>("model")),
    _formulation(getParam<MooseEnum>("formulation")),
    _system(getParam<MooseEnum>("system"))
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
}

void
ContactAction::act()
{
  if (!_problem->getDisplacedProblem())
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");

  // It is risky to apply this optimization to contact problems
  // since the problem configuration may be changed during Jacobian
  // evaluation. We therefore turn it off for all contact problems so that
  // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3.
  if (!_problem->isSNESMFReuseBaseSetbyUser())
    _problem->setSNESMFReuseBase(false, false);

  std::string action_name = MooseUtils::shortName(name());

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

  // Determine number of dimensions
  const unsigned int ndisp = displacements.size();

  // convert vector of VariableName to vector of VariableName
  std::vector<VariableName> coupled_displacements(ndisp);
  for (unsigned int i = 0; i < ndisp; ++i)
    coupled_displacements[i] = displacements[i];

  if (_current_task == "add_dirac_kernel")
  {
    // MechanicalContactConstraint has to be added after the init_problem task, so it cannot be
    // added for the add_constraint task.
    if (_system == "Constraint")
    {
      InputParameters params = _factory.getValidParams("MechanicalContactConstraint");
      params.applyParameters(parameters(), {"displacements"});
      params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + name()};
      params.set<std::vector<VariableName>>("displacements") = coupled_displacements;
      params.set<BoundaryName>("boundary") = _master;
      params.set<bool>("use_displaced_mesh") = true;

      for (unsigned int i = 0; i < ndisp; ++i)
      {
        std::string name = action_name + "_constraint_" + Moose::stringify(i);

        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = displacements[i];
        params.set<std::vector<VariableName>>("master_variable") = {coupled_displacements[i]};
        _problem->addConstraint("MechanicalContactConstraint", name, params);
      }
    }

    if (_system == "DiracKernel")
    {
      {
        InputParameters params = _factory.getValidParams("ContactMaster");
        params.applyParameters(parameters(), {"displacements"});
        params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + name()};
        params.set<std::vector<VariableName>>("displacements") = coupled_displacements;
        params.set<BoundaryName>("boundary") = _master;
        params.set<bool>("use_displaced_mesh") = true;

        for (unsigned int i = 0; i < ndisp; ++i)
        {
          std::string name = action_name + "_master_" + Moose::stringify(i);
          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = displacements[i];
          _problem->addDiracKernel("ContactMaster", name, params);
        }
      }

      {
        InputParameters params = _factory.getValidParams("SlaveConstraint");
        params.applyParameters(parameters(), {"displacements"});
        params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + name()};
        params.set<std::vector<VariableName>>("displacements") = coupled_displacements;
        params.set<BoundaryName>("boundary") = _slave;
        params.set<bool>("use_displaced_mesh") = true;

        for (unsigned int i = 0; i < ndisp; ++i)
        {
          std::string name = action_name + "_slave_" + Moose::stringify(i);
          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = displacements[i];
          _problem->addDiracKernel("SlaveConstraint", name, params);
        }
      }
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
  return MooseEnum("kinematic penalty augmented_lagrange tangential_penalty mortar", "kinematic");
}

MooseEnum
ContactAction::getSystemEnum()
{
  return MooseEnum("DiracKernel Constraint", "DiracKernel");
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
