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
  MooseEnum formulation("DEFAULT KINEMATIC PENALTY AUGMENTED_LAGRANGE", "DEFAULT");
  MooseEnum system("DiracKernel Constraint", "DiracKernel");

  InputParameters params = validParams<Action>();
  params += validParams<OutputInterface>();

  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  Must be positive.");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  params.addParam<MooseEnum>("formulation", formulation, "The contact formulation: default, penalty, augmented_lagrange");
  params.addParam<MooseEnum>("system", system, "System to use for constraint enforcement.  Options are: " + system.getRawNames());

  // Hide the auto-generated aux variables.
  params.set<std::vector<OutputName> >("outputs") = std::vector<OutputName>(1, "none");
  params.addParam<bool>("output_penetration_info_vars", false, "Show the aux variables for the PenetrationInfo objects in the output files.");

  return params;
}

ContactAction::ContactAction(const std::string & name, InputParameters params) :
  Action(name, params),
  OutputInterface(params, false),
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

  std::string action_name = getShortName();

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);

  std::vector<AuxVariableName> contact_force_component_name(numdims);
  std::vector<std::string> contact_force_arg_name(numdims);
  const std::string dim_to_letter[] = { "x", "y", "z" };
  for ( unsigned dim = 0; dim < numdims; ++dim )
  {
    contact_force_component_name[dim] = action_name + "_contact_force_" + Moose::stringify(dim);
    contact_force_arg_name[dim] = "force_" + dim_to_letter[dim];
  }
  AuxVariableName accumulated_slip_name = action_name + "_accumulated_slip";
  AuxVariableName frictional_energy_name = action_name + "_frictional_energy";

  // By default, we hide the state variables for the PenetrationInfo so that they do not add bulk
  // to the output files.
  std::set<std::string> hidden_variables;
  hidden_variables.insert(contact_force_component_name.begin(),contact_force_component_name.end());
  hidden_variables.insert(accumulated_slip_name);
  hidden_variables.insert(frictional_energy_name);

  if ( _current_task == "add_aux_variable" )
  {
    const FEType fetype(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
      Utility::string_to_enum<FEFamily>("LAGRANGE"));

    for ( unsigned dim = 0; dim < numdims; ++dim )
      _problem->addAuxVariable( contact_force_component_name[dim], fetype );
    _problem->addAuxVariable( accumulated_slip_name, fetype );
    _problem->addAuxVariable( frictional_energy_name, fetype );
  }

  else if ( _current_task == "add_aux_kernel" )
  {
    // Add the aux kernels that take care of state in PenetrationInfo.
    {
      InputParameters params = _factory.getValidParams("PenetrationAux");

      // Extract global params
      _app.parser().extractParams(_name, params);

      params.set<std::vector<BoundaryName> >("boundary") = std::vector<BoundaryName>(1,_slave);
      params.set<BoundaryName>("paired_boundary") = _master;
      params.set<MooseEnum>("order") = _order;
      params.set<MultiMooseEnum>("execute_on") = "timestep_end";
      params.set<bool>("added_by_contact_action") = true;

      for ( unsigned dim = 0; dim < numdims; ++dim )
      {
        params.set<AuxVariableName>("variable") = contact_force_component_name[dim];
        params.set<MooseEnum>("quantity") = contact_force_arg_name[dim];
        const std::string kernel_name = action_name + "_penetration_aux_force_" + dim_to_letter[dim];
        _problem->addAuxKernel("PenetrationAux", kernel_name, params);
      }

      params.set<AuxVariableName>("variable") = accumulated_slip_name;
      params.set<MooseEnum>("quantity") = "accumulated_slip";
      _problem->addAuxKernel("PenetrationAux", action_name + "_penetration_aux_accumulated_slip", params);

      params.set<AuxVariableName>("variable") = frictional_energy_name;
      params.set<MooseEnum>("quantity") = "frictional_energy";
      _problem->addAuxKernel("PenetrationAux", action_name + "_penetration_aux_frictional_energy", params);
    }
  }

  else if ( _current_task == "output_penetration_info_vars" )
  {
    // The hide list has to be built after the outputs are built, therefore it has to be in this
    // task rather than in the add_aux_variable task.
    if ( ! getParam<bool>("output_penetration_info_vars") )
      buildOutputHideVariableList(hidden_variables);
  }

  else if ( _current_task == "add_dirac_kernel" )
  {
    // MechanicalContactConstraint has to be added after the init_problem task, so it cannot be
    // added for the add_constraint task.
    if (_system == "Constraint")
    {
      InputParameters params = _factory.getValidParams("MechanicalContactConstraint");

      // Extract global params
      _app.parser().extractParams(_name, params);

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
      params.set<std::vector<VariableName> >("nodal_area") = std::vector<VariableName>(1, "nodal_area_"+action_name);

      if (isParamValid("tangential_tolerance"))
        params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

      if (isParamValid("normal_smoothing_distance"))
        params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

      if (isParamValid("normal_smoothing_method"))
        params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

      params.addCoupledVar("disp_x", "The x displacement");
      params.set<std::vector<VariableName> >("disp_x") = std::vector<VariableName>(1, _disp_x);

      params.addCoupledVar("disp_y", "The y displacement");
      if (numdims > 1)
        params.set<std::vector<VariableName> >("disp_y") = std::vector<VariableName>(1, _disp_y);

      params.addCoupledVar("disp_z", "The z displacement");
      if (numdims == 3)
        params.set<std::vector<VariableName> >("disp_z") = std::vector<VariableName>(1, _disp_z);

      params.set<bool>("use_displaced_mesh") = true;

      for (unsigned int i(0); i < numdims; ++i)
      {
        std::string name = action_name + "_constraint_" + Moose::stringify(i);

        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = vars[i];
        params.set<std::vector<VariableName> >("master_variable") = std::vector<VariableName>(1,vars[i]);

        _problem->addConstraint("MechanicalContactConstraint", name, params);
      }
    }

    if (_system == "DiracKernel")
    {
      {
        InputParameters params = _factory.getValidParams("ContactMaster");

        // Extract global params
        _app.parser().extractParams(_name, params);

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
        params.set<std::vector<VariableName> >("nodal_area") = std::vector<VariableName>(1, "nodal_area_"+action_name);

        if (isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

        if (isParamValid("normal_smoothing_distance"))
          params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

        if (isParamValid("normal_smoothing_method"))
          params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

        params.addCoupledVar("disp_x", "The x displacement");
        params.set<std::vector<VariableName> >("disp_x") = std::vector<VariableName>(1, _disp_x);

        params.addCoupledVar("disp_y", "The y displacement");
        if (numdims > 1)
          params.set<std::vector<VariableName> >("disp_y") = std::vector<VariableName>(1, _disp_y);

        params.addCoupledVar("disp_z", "The z displacement");
        if (numdims == 3)
          params.set<std::vector<VariableName> >("disp_z") = std::vector<VariableName>(1, _disp_z);

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
        _app.parser().extractParams(_name, params);

        // Create slave objects
        params.set<std::string>("model") = _model;
        params.set<std::string>("formulation") = _formulation;
        params.set<MooseEnum>("order") = _order;
        params.set<BoundaryName>("boundary") = _slave;
        params.set<BoundaryName>("master") = _master;
        params.set<Real>("penalty") = _penalty;
        params.set<Real>("friction_coefficient") = _friction_coefficient;
        params.addRequiredCoupledVar("nodal_area", "The nodal area");
        params.set<std::vector<VariableName> >("nodal_area") = std::vector<VariableName>(1, "nodal_area_"+action_name);
        if (isParamValid("tangential_tolerance"))
          params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

        if (isParamValid("normal_smoothing_distance"))
          params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

        if (isParamValid("normal_smoothing_method"))
          params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

        params.addCoupledVar("disp_x", "The x displacement");
        params.set<std::vector<VariableName> >("disp_x") = std::vector<VariableName>(1, _disp_x);

        params.addCoupledVar("disp_y", "The y displacement");
        if (numdims > 1)
          params.set<std::vector<VariableName> >("disp_y") = std::vector<VariableName>(1, _disp_y);

        params.addCoupledVar("disp_z", "The z displacement");
        if (numdims == 3)
          params.set<std::vector<VariableName> >("disp_z") = std::vector<VariableName>(1, _disp_z);

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
