/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ContactAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

template <>
InputParameters
validParams<ContactAction>()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  MooseEnum formulation("DEFAULT KINEMATIC PENALTY AUGMENTED_LAGRANGE TANGENTIAL_PENALTY",
                        "DEFAULT");
  MooseEnum system("DiracKernel Constraint", "DiracKernel");

  InputParameters params = validParams<Action>();

  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");

  params.addParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");

  params.addParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("penalty_slip",
                        1e8,
                        "The penalty to apply on sliping direction.  This can "
                        "vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tension_release",
                        0.0,
                        "Tension release threshold.  A node in contact "
                        "will not be released if its tensile load is below "
                        "this value.  No tension release if negative.");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "capture_tolerance", 0, "Normal distance from surface within which nodes are captured");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  params.addParam<MooseEnum>(
      "formulation",
      formulation,
      "The contact formulation: default, penalty, augmented_lagrange, tangential_penalty");
  params.addParam<MooseEnum>("system",
                             system,
                             "System to use for constraint enforcement.  Options are: " +
                                 system.getRawNames());
  params.addParam<bool>("normalize_penalty",
                        false,
                        "Whether to normalize the penalty parameter with the nodal area.");
  params.addParam<bool>("master_slave_jacobian",
                        true,
                        "Whether to include jacobian entries coupling master and slave nodes.");

  params.addParam<Real>("penetration_tolerance", 1e-9, "The tolerance of the distance function");
  params.addParam<Real>("stickking_tolerance", 1e-9, "The tolerance of the sticking");

  params.addParam<Real>("frictionalforce_tolerance", 1e-9, "The tolerance of the frictionalforce");

  return params;
}

ContactAction::ContactAction(const InputParameters & params)
  : Action(params),
    _master(getParam<BoundaryName>("master")),
    _slave(getParam<BoundaryName>("slave")),
    _model(getParam<std::string>("model")),
    _formulation(getParam<MooseEnum>("formulation")),
    _system(getParam<MooseEnum>("system"))
{
  if (_formulation == "tangential_penalty")
  {
    if (_system != "Constraint")
      mooseError(
          "The 'tangential_penalty' formulation can only be used with the 'Constraint' system");
    if (_model != "coulomb")
      mooseError("The 'tangential_penalty' formulation can only be used with the 'coulomb' model");
  }
}

void
ContactAction::act()
{
  if (!_problem->getDisplacedProblem())
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");

  std::string action_name = MooseUtils::shortName(name());

  std::vector<NonlinearVariableName> displacements;
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<NonlinearVariableName>>("displacements");
  else
  {
    // Legacy parameter scheme for displacements
    if (!isParamValid("disp_x"))
      mooseError("Specify displacement variables using the `displacements` parameter.");
    displacements.push_back(getParam<NonlinearVariableName>("disp_x"));

    if (isParamValid("disp_y"))
    {
      displacements.push_back(getParam<NonlinearVariableName>("disp_y"));
      if (isParamValid("disp_z"))
        displacements.push_back(getParam<NonlinearVariableName>("disp_z"));
    }

    mooseDeprecated("use the `displacements` parameter rather than the `disp_*` parameters (those "
                    "will go away with the deprecation of the Solid Mechanics module).");
  }

  // Determine number of dimensions
  const unsigned int ndisp = displacements.size();

  // convert vector of NonlinearVariableName to vector of VariableName
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
      params.applyParameters(parameters(), {"displacements", "formulation"});
      params.set<std::string>("formulation") = _formulation;
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
        params.applyParameters(parameters(), {"displacements", "formulation"});
        params.set<std::string>("formulation") = _formulation;
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
        params.applyParameters(parameters(), {"displacements", "formulation"});
        params.set<std::string>("formulation") = _formulation;
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
