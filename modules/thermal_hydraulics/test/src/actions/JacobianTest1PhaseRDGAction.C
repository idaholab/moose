//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JacobianTest1PhaseRDGAction.h"
#include "TestAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"

registerMooseAction("ThermalHydraulicsTestApp", JacobianTest1PhaseRDGAction, "meta_action");

InputParameters
JacobianTest1PhaseRDGAction::validParams()
{
  InputParameters params = emptyInputParameters();
  params += JacobianTestAction::validParams();

  params.addClassDescription("Sets up a Jacobian test for rDG");

  params.addParam<bool>("add_dg_kernel", false, "Option to add DG kernel");
  params.addParam<bool>("add_bc", false, "Option to add BC");

  params.addParam<UserObjectName>(
      "numerical_flux", "", "Name of the numerical flux user object to test");
  params.addParam<UserObjectName>(
      "boundary_flux", "", "Name of the boundary flux user object to test");

  MooseEnum ic_option("constant riemann_L riemann_LM riemann_RM riemann_R");
  params.addRequiredParam<MooseEnum>("ic_option", ic_option, "IC option");

  params.addParam<FunctionName>("A_function", "2.0", "Area function");

  params.addParam<bool>("use_slope_reconstruction", true, "Use slope reconstruction?");

  params.addParam<bool>(
      "use_elem_area", false, "Use the elemental area variable instead of linear area");

  params.set<std::string>("fe_family") = "MONOMIAL";
  params.set<std::string>("fe_order") = "CONSTANT";

  return params;
}

JacobianTest1PhaseRDGAction::JacobianTest1PhaseRDGAction(const InputParameters & params)
  : JacobianTestAction(params),
    _A_name("A"),
    _A_linear_name("A_linear"),
    _rhoA_name("rhoA"),
    _rhouA_name("rhouA"),
    _rhoEA_name("rhoEA"),
    _add_dg_kernel(getParam<bool>("add_dg_kernel")),
    _add_bc(getParam<bool>("add_bc")),
    _numerical_flux_name(getParam<UserObjectName>("numerical_flux")),
    _boundary_flux_name(getParam<UserObjectName>("boundary_flux")),
    _ic_option(getParam<MooseEnum>("ic_option")),
    _A_fn_name(getParam<FunctionName>("A_function")),
    _use_slope_reconstruction(getParam<bool>("use_slope_reconstruction")),
    _reconstruction_material_name("reconstruction_material"),
    _direction_name("direction"),
    _fp_name("fluid_properties")
{
  if (_add_dg_kernel && !_pars.isParamSetByUser("numerical_flux"))
    mooseError("The parameter 'numerical_flux' must be provided when adding a DG kernel.");
  if (_add_bc && !_pars.isParamSetByUser("boundary_flux"))
    mooseError("The parameter 'boundary_flux' must be provided when adding a BC.");
}

void
JacobianTest1PhaseRDGAction::addObjects()
{
  TestAction::addObjects();

  const std::vector<VariableName> variables = {_rhoA_name, _rhouA_name, _rhoEA_name};

  if (_add_dg_kernel)
  {
    // add the DG kernel to use the internal flux
    for (unsigned int i = 0; i < variables.size(); i++)
    {
      const std::string class_name = "AddDGKernelAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "NumericalFlux3EqnDGKernel";
      params.set<NonlinearVariableName>("variable") = variables[i];
      params.set<std::vector<SubdomainName>>("block") = {"0"};

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, "internal_side_flux_" + variables[i], params));

      action->getObjectParams().set<NonlinearVariableName>("variable") = variables[i];
      action->getObjectParams().set<std::vector<SubdomainName>>("block") = {"0"};

      action->getObjectParams().set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

      action->getObjectParams().set<UserObjectName>("numerical_flux") = _numerical_flux_name;

      _awh.addActionBlock(action);
    }
  }

  if (_add_bc)
  {
    // add the BC to use the boundary flux
    for (unsigned int i = 0; i < variables.size(); i++)
    {
      const std::string class_name = "AddBCAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "BoundaryFlux3EqnBC";
      params.set<NonlinearVariableName>("variable") = variables[i];
      params.set<Real>("normal") = 1.;
      params.set<std::vector<BoundaryName>>("boundary") = {"0"};

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, "bc_" + variables[i], params));

      action->getObjectParams().set<NonlinearVariableName>("variable") = variables[i];
      action->getObjectParams().set<Real>("normal") = 1.;
      action->getObjectParams().set<std::vector<BoundaryName>>("boundary") = {"0"};

      action->getObjectParams().set<std::vector<VariableName>>("A_elem") = {_A_name};
      action->getObjectParams().set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
      action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

      action->getObjectParams().set<UserObjectName>("boundary_flux") = _boundary_flux_name;

      _awh.addActionBlock(action);
    }
  }
}

void
JacobianTest1PhaseRDGAction::addMesh()
{
  // for internal side flux testing, use 2 elements (need an internal side)
  if (_add_dg_kernel)
    addMeshInternal(2);
  else
    addMeshInternal(1);
}

void
JacobianTest1PhaseRDGAction::addInitialConditions()
{
}

void
JacobianTest1PhaseRDGAction::addSolutionVariables()
{
  const std::vector<VariableName> variables = {_rhoA_name, _rhouA_name, _rhoEA_name};

  if (_ic_option == "constant")
  {
    const std::vector<Real> values = {2.0, 4.0, 30.0};

    for (std::size_t i = 0; i < variables.size(); ++i)
    {
      addSolutionVariable(variables[i]);
      addConstantIC(variables[i], values[i]);
    }
  }
  else if (_ic_option == "riemann_L") // left region
  {
    const std::vector<Real> values_left = {2.2, 4.4, 11.2};
    const std::vector<Real> values_right = {2.6, 6.4, 12.2};

    addSolutionVariablesRiemannIC(variables, values_left, values_right);
  }
  else if (_ic_option == "riemann_LM") // left star region
  {
    const std::vector<Real> values_left = {2.2, -4.4, 11.2};
    const std::vector<Real> values_right = {3.2, 8.4, 12.2};

    addSolutionVariablesRiemannIC(variables, values_left, values_right);
  }
  else if (_ic_option == "riemann_RM") // right star region
  {
    const std::vector<Real> values_left = {2.2, 4.4, 11.2};
    const std::vector<Real> values_right = {1.8, -5.0, 12.2};

    addSolutionVariablesRiemannIC(variables, values_left, values_right);
  }
  else if (_ic_option == "riemann_R") // right region
  {
    const std::vector<Real> values_left = {2.2, 4.4, 11.2};
    const std::vector<Real> values_right = {2.4, -16.0, 53.4};

    addSolutionVariablesRiemannIC(variables, values_left, values_right);
  }
  else
  {
    mooseError("Invalid IC option");
  }
}

void
JacobianTest1PhaseRDGAction::addSolutionVariablesRiemannIC(
    const std::vector<VariableName> & variables,
    const std::vector<Real> & values_left,
    const std::vector<Real> & values_right)
{
  for (unsigned int i = 0; i < variables.size(); ++i)
  {
    // add the variable
    addSolutionVariable(variables[i], "MONOMIAL", "CONSTANT");

    // add the IC function
    FunctionName ic_fn_name;
    {
      const std::string class_name = "AddFunctionAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "PiecewiseConstant";

      ic_fn_name = variables[i] + "_IC_fn";
      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, ic_fn_name, params));

      action->getObjectParams().set<MooseEnum>("axis") = "x";
      action->getObjectParams().set<std::vector<Real>>("x") = {0.0, 0.5};
      action->getObjectParams().set<std::vector<Real>>("y") = {values_left[i], values_right[i]};

      _awh.addActionBlock(action);
    }

    addFunctionIC(variables[i], ic_fn_name);
  }
}

void
JacobianTest1PhaseRDGAction::addAuxVariables()
{
  addAuxVariable(_A_name, _fe_family, _fe_order);
  addFunctionIC(_A_name, _A_fn_name);
  if (getParam<bool>("use_elem_area"))
    addAuxVariable(_A_linear_name, _fe_family, _fe_order);
  else
    addAuxVariable(_A_linear_name, "LAGRANGE", "FIRST");
  addFunctionIC(_A_linear_name, _A_fn_name);
}

void
JacobianTest1PhaseRDGAction::addMaterials()
{
  // direction
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "DirectionMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "direction_material", params));

    _awh.addActionBlock(action);
  }

  // fluid properties
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    if (_ad)
      params.set<std::string>("type") = "ADFluidProperties3EqnMaterial";
    else
      params.set<std::string>("type") = "FluidProperties3EqnMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "fluid_properties_material", params));

    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

    action->getObjectParams().set<UserObjectName>("fp") = _fp_name;

    _awh.addActionBlock(action);
  }

  // reconstruction material
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "RDG3EqnMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, _reconstruction_material_name, params));

    if (_use_slope_reconstruction)
      action->getObjectParams().set<MooseEnum>("scheme") = "Minmod";
    else
      action->getObjectParams().set<MooseEnum>("scheme") = "None";

    action->getObjectParams().set<std::vector<VariableName>>("A_elem") = {_A_name};
    action->getObjectParams().set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    action->getObjectParams().set<MaterialPropertyName>("direction") = _direction_name;

    action->getObjectParams().set<UserObjectName>("fluid_properties") = _fp_name;

    _awh.addActionBlock(action);
  }
}

void
JacobianTest1PhaseRDGAction::addUserObjects()
{
  // fluid properties
  {
    const std::string class_name = "AddFluidPropertiesAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "IdealGasFluidProperties";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, _fp_name, params));

    action->getObjectParams().set<Real>("gamma") = 1.5;
    action->getObjectParams().set<Real>("molar_mass") = 0.83144598;

    _awh.addActionBlock(action);
  }
}
