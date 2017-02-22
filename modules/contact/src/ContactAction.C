/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ContactAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "Conversion.h"

#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<ContactAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");
  MooseEnum formulation("DEFAULT KINEMATIC PENALTY AUGMENTED_LAGRANGE TANGENTIAL_PENALTY", "DEFAULT");
  MooseEnum system("DiracKernel Constraint", "DiracKernel");

  InputParameters params = validParams<Action>();

  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  No tension release if negative.");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("capture_tolerance", 0, "Normal distance from surface within which nodes are captured");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  params.addParam<MooseEnum>("formulation", formulation, "The contact formulation: default, penalty, augmented_lagrange, tangential_penalty");
  params.addParam<MooseEnum>("system", system, "System to use for constraint enforcement.  Options are: " + system.getRawNames());

  return params;
}

ContactAction::ContactAction(const InputParameters & params) :
  Action(params),
  _master(getParam<BoundaryName>("master")),
  _slave(getParam<BoundaryName>("slave")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _penalty(getParam<Real>("penalty")),
  _friction_coefficient(getParam<Real>("friction_coefficient")),
  _tension_release(getParam<Real>("tension_release")),
  _model(getParam<std::string>("model")),
  _formulation(getParam<MooseEnum>("formulation")),
  _order(getParam<MooseEnum>("order")),
  _system(getParam<MooseEnum>("system"))
{
  if (_formulation == "tangential_penalty")
  {
    if (_system != "Constraint")
      mooseError ("The 'tangential_penalty' formulation can only be used with the 'Constraint' system");
    if (_model != "coulomb")
      mooseError ("The 'tangential_penalty' formulation can only be used with the 'coulomb' model");
  }
}

void
ContactAction::act()
{
  if (!_problem->getDisplacedProblem())
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the Mesh block.");

  // Determine number of dimensions
  unsigned int numdims(1);
  if (_disp_y != "")
    ++numdims;

  if (_disp_z != "")
    ++numdims;

  std::string action_name = MooseUtils::shortName(name());
  std::vector<NonlinearVariableName> vars = {_disp_x, _disp_y, _disp_z};

  if ( _current_task == "add_dirac_kernel" )
  {
    // MechanicalContactConstraint has to be added after the init_problem task, so it cannot be
    // added for the add_constraint task.
    if (_system == "Constraint")
    {
      InputParameters params = _factory.getValidParams("MechanicalContactConstraint");

      // Extract global params
      if (isParamValid("parser_syntax"))
        _app.parser().extractParams(getParam<std::string>("parser_syntax"), params);
      else
        mooseError("The 'parser_syntax' parameter is not valid, which indicates that this actions was not created by the Parser, which is not currently supported.");

      // Create Constraint objects
      params.set<std::string>("model") = _model;
      params.set<std::string>("formulation") = _formulation;
      params.set<MooseEnum>("order") = _order;
      params.set<BoundaryName>("boundary") = _master;
      params.set<BoundaryName>("slave") = _slave;
      params.set<Real>("penalty") = _penalty;
      params.set<Real>("friction_coefficient") = _friction_coefficient;
      params.set<Real>("tension_release") = _tension_release;
      params.addRequiredCoupledVar("nodal_area", "The nodal area");
      params.set<std::vector<VariableName> >("nodal_area") = {"nodal_area_" + name()};

      if (isParamValid("tangential_tolerance"))
        params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

      if (isParamValid("capture_tolerance"))
        params.set<Real>("capture_tolerance") = getParam<Real>("capture_tolerance");

      if (isParamValid("normal_smoothing_distance"))
        params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

      if (isParamValid("normal_smoothing_method"))
        params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

      params.addCoupledVar("disp_x", "The x displacement");
      params.set<std::vector<VariableName> >("disp_x") = {_disp_x};

      params.addCoupledVar("disp_y", "The y displacement");
      if (numdims > 1)
        params.set<std::vector<VariableName> >("disp_y") = {_disp_y};

      params.addCoupledVar("disp_z", "The z displacement");
      if (numdims == 3)
        params.set<std::vector<VariableName> >("disp_z") = {_disp_z};

      params.set<bool>("use_displaced_mesh") = true;

      for (unsigned int i(0); i < numdims; ++i)
      {
        std::string name = action_name + "_constraint_" + Moose::stringify(i);

        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = vars[i];
        params.set<std::vector<VariableName> >("master_variable") = {vars[i]};

        _problem->addConstraint("MechanicalContactConstraint", name, params);
      }
    }

    if (_system == "DiracKernel")
    {
      {
        InputParameters params = _factory.getValidParams("ContactMaster");

        // Extract global params
        if (isParamValid("parser_syntax"))
          _app.parser().extractParams(getParam<std::string>("parser_syntax"), params);
        else
          mooseError("The 'parser_syntax' parameter is not valid, which indicates that this actions was not created by the Parser, which is not currently supported.");


        // Create master objects
        params.set<std::string>("model") = _model;
        params.set<std::string>("formulation") = _formulation;
        params.set<MooseEnum>("order") = _order;
        params.set<BoundaryName>("boundary") = _master;
        params.set<BoundaryName>("slave") = _slave;
        params.set<Real>("penalty") = _penalty;
        params.set<Real>("friction_coefficient") = _friction_coefficient;
        params.set<Real>("tension_release") = _tension_release;
        params.addRequiredCoupledVar("nodal_area", "The nodal area");
        params.set<std::vector<VariableName> >("nodal_area") = {"nodal_area_" + name()};

        if (isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

        if (isParamValid("capture_tolerance"))
          params.set<Real>("capture_tolerance") = getParam<Real>("capture_tolerance");

        if (isParamValid("normal_smoothing_distance"))
          params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

        if (isParamValid("normal_smoothing_method"))
          params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

        params.addCoupledVar("disp_x", "The x displacement");
        params.set<std::vector<VariableName> >("disp_x") = {_disp_x};

        params.addCoupledVar("disp_y", "The y displacement");
        if (numdims > 1)
          params.set<std::vector<VariableName> >("disp_y") = {_disp_y};

        params.addCoupledVar("disp_z", "The z displacement");
        if (numdims == 3)
          params.set<std::vector<VariableName> >("disp_z") = {_disp_z};

        params.set<bool>("use_displaced_mesh") = true;

        for (unsigned int i(0); i < numdims; ++i)
        {
          std::string name = action_name + "_master_" + Moose::stringify(i);

          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = vars[i];

          _problem->addDiracKernel("ContactMaster", name, params);
        }
      }

      {
        InputParameters params = _factory.getValidParams("SlaveConstraint");

        // Extract global params
        if (isParamValid("parser_syntax"))
          _app.parser().extractParams(getParam<std::string>("parser_syntax"), params);
        else
          mooseError("The 'parser_syntax' parameter is not valid, which indicates that this actions was not created by the Parser, which is not currently supported.");

        // Create slave objects
        params.set<std::string>("model") = _model;
        params.set<std::string>("formulation") = _formulation;
        params.set<MooseEnum>("order") = _order;
        params.set<BoundaryName>("boundary") = _slave;
        params.set<BoundaryName>("master") = _master;
        params.set<Real>("penalty") = _penalty;
        params.set<Real>("friction_coefficient") = _friction_coefficient;
        params.addRequiredCoupledVar("nodal_area", "The nodal area");
        params.set<std::vector<VariableName> >("nodal_area") = {"nodal_area_" +name()};
        if (isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

        if (isParamValid("capture_tolerance"))
          params.set<Real>("capture_tolerance") = getParam<Real>("capture_tolerance");

        if (isParamValid("normal_smoothing_distance"))
          params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

        if (isParamValid("normal_smoothing_method"))
          params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

        params.addCoupledVar("disp_x", "The x displacement");
        params.set<std::vector<VariableName> >("disp_x") = {_disp_x};

        params.addCoupledVar("disp_y", "The y displacement");
        if (numdims > 1)
          params.set<std::vector<VariableName> >("disp_y") = {_disp_y};

        params.addCoupledVar("disp_z", "The z displacement");
        if (numdims == 3)
          params.set<std::vector<VariableName> >("disp_z") = {_disp_z};

        params.set<bool>("use_displaced_mesh") = true;

        for (unsigned int i(0); i < numdims; ++i)
        {
          std::string name = action_name + "_slave_" + Moose::stringify(i);

          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = vars[i];

          _problem->addDiracKernel("SlaveConstraint", name, params);
        }
      }
    }
  }
}
