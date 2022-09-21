//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelSetup1Phase.h"
#include "MooseObjectAction.h"

InputParameters
FlowModelSetup1Phase::validParams()
{
  InputParameters params = FlowModelSetup::validParams();

  params.addRequiredParam<FunctionName>("p", "Initial pressure function");
  params.addRequiredParam<FunctionName>("T", "Initial temperature function");
  params.addRequiredParam<FunctionName>("vel", "Initial velocity function");
  params.addRequiredParam<FunctionName>("A", "Area function");
  params.addParam<FunctionName>("D_h", 0, "Hydraulic diameter function");

  params.addParam<Real>("scaling_rhoA", 1.0, "Scaling factor for rho*A");
  params.addParam<Real>("scaling_rhouA", 1.0, "Scaling factor for rho*u*A");
  params.addParam<Real>("scaling_rhoEA", 1.0, "Scaling factor for rho*E*A");

  params.addRequiredParam<UserObjectName>("fp_1phase", "Single-phase fluid properties object name");

  return params;
}

FlowModelSetup1Phase::FlowModelSetup1Phase(const InputParameters & params)
  : FlowModelSetup(params),

    _p_fn(getParam<FunctionName>("p")),
    _T_fn(getParam<FunctionName>("T")),
    _vel_fn(getParam<FunctionName>("vel")),
    _A_fn(getParam<FunctionName>("A")),
    _D_h_fn(getParam<FunctionName>("D_h")),

    _fp_1phase_name(getParam<UserObjectName>("fp_1phase")),

    _unity_name("unity"),
    _A_name("A"),
    _D_h_name("D_h"),
    _rhoA_name("rhoA"),
    _rhouA_name("rhouA"),
    _rhoEA_name("rhoEA"),
    _rho_name("rho"),
    _vel_name("vel"),
    _p_name("p"),
    _T_name("T"),
    _v_name("v"),
    _e_name("e"),
    _H_name("H"),
    _mu_name("mu"),
    _ad(getParam<bool>("ad"))
{
}

void
FlowModelSetup1Phase::addInitialConditions()
{
  const std::string class_name = "AddInitialConditionAction";

  addFunctionIC(_A_name, _A_fn);
  addFunctionIC(_p_name, _p_fn);
  addFunctionIC(_T_name, _T_fn);
  addFunctionIC(_vel_name, _vel_fn);

  // rho
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "RhoFromPressureTemperatureIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _rho_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _rho_name;
    action->getObjectParams().set<UserObjectName>("fp") = _fp_1phase_name;
    action->getObjectParams().set<std::vector<VariableName>>("p") = {_p_name};
    action->getObjectParams().set<std::vector<VariableName>>("T") = {_T_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // rho*A
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "VariableProductIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _rhoA_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _rhoA_name;
    action->getObjectParams().set<std::vector<VariableName>>("values") = {_rho_name, _A_name};

    _this_action_warehouse.addActionBlock(action);
  }
  // rho*u*A
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "VariableProductIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _rhouA_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _rhouA_name;
    action->getObjectParams().set<std::vector<VariableName>>("values") = {
        _rho_name, _vel_name, _A_name};

    _this_action_warehouse.addActionBlock(action);
  }
  // rho*E*A
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "RhoEAFromPressureTemperatureVelocityIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _rhoEA_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _rhoEA_name;
    action->getObjectParams().set<UserObjectName>("fp") = _fp_1phase_name;
    action->getObjectParams().set<std::vector<VariableName>>("p") = {_p_name};
    action->getObjectParams().set<std::vector<VariableName>>("T") = {_T_name};
    action->getObjectParams().set<std::vector<VariableName>>("vel") = {_vel_name};
    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // specific volume
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "SpecificVolumeIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _v_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _v_name;
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }
  // specific internal energy
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "SpecificInternalEnergyIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _e_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _e_name;
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

    _this_action_warehouse.addActionBlock(action);
  }
  // total specific enthalpy
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "SpecificTotalEnthalpyIC";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _H_name + "_ic", params));

    action->getObjectParams().set<VariableName>("variable") = _H_name;
    action->getObjectParams().set<std::vector<VariableName>>("p") = {_p_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }
}

void
FlowModelSetup1Phase::addSolutionVariables()
{
  std::vector<VariableName> var_names{_rhoA_name, _rhouA_name, _rhoEA_name};
  for (const VariableName & var_name : var_names)
  {
    const Real scaling_factor = getParam<Real>("scaling_" + var_name);
    addSolutionVariable(var_name, scaling_factor);
  }
}

void
FlowModelSetup1Phase::addNonConstantAuxVariables()
{
  // area
  addAuxVariable(_A_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "FunctionAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _A_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _A_name;
    action->getObjectParams().set<FunctionName>("function") = _A_fn;

    _this_action_warehouse.addActionBlock(action);
  }

  // rho
  addAuxVariable(_rho_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "QuotientAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _rho_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _rho_name;
    action->getObjectParams().set<std::vector<VariableName>>("numerator") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("denominator") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // velocity
  addAuxVariable(_vel_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "QuotientAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _vel_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _vel_name;
    action->getObjectParams().set<std::vector<VariableName>>("numerator") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("denominator") = {_rhoA_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // pressure
  addAuxVariable(_p_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "PressureAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _p_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _p_name;
    action->getObjectParams().set<std::vector<VariableName>>("e") = {_e_name};
    action->getObjectParams().set<std::vector<VariableName>>("v") = {_v_name};
    action->getObjectParams().set<UserObjectName>("fp") = _fp_1phase_name;

    _this_action_warehouse.addActionBlock(action);
  }

  // temperature
  addAuxVariable(_T_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "TemperatureAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _T_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _T_name;
    action->getObjectParams().set<std::vector<VariableName>>("e") = {_e_name};
    action->getObjectParams().set<std::vector<VariableName>>("v") = {_v_name};
    action->getObjectParams().set<UserObjectName>("fp") = _fp_1phase_name;

    _this_action_warehouse.addActionBlock(action);
  }

  // specific volume
  addAuxVariable(_v_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "THMSpecificVolumeAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _v_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _v_name;
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // specific internal energy
  addAuxVariable(_e_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "THMSpecificInternalEnergyAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _e_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _e_name;
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // specific total enthalpy
  addAuxVariable(_H_name);
  {
    const std::string class_name = "AddKernelAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "SpecificTotalEnthalpyAux";
    params.set<std::string>("task") = "add_aux_kernel";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, _H_name + "_aux", params));

    action->getObjectParams().set<AuxVariableName>("variable") = _H_name;
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    action->getObjectParams().set<std::vector<VariableName>>("p") = {_p_name};
    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};

    _this_action_warehouse.addActionBlock(action);
  }
}

void
FlowModelSetup1Phase::addMaterials()
{
  FlowModelSetup::addMaterials();

  const std::string class_name = "AddMaterialAction";

  // unity
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "ConstantMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, "unity_material", params));

    action->getObjectParams().set<std::string>("property_name") = _unity_name;
    action->getObjectParams().set<Real>("value") = 1.0;
    action->getObjectParams().set<std::vector<VariableName>>("derivative_vars") = {
        _rhoA_name, _rhouA_name, _rhoEA_name};

    _this_action_warehouse.addActionBlock(action);
  }

  // fluid properties
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    if (_ad)
      params.set<std::string>("type") = "ADFluidProperties3EqnMaterial";
    else
      params.set<std::string>("type") = "FluidProperties3EqnMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, "fluid_properties_material", params));

    action->getObjectParams().set<std::vector<VariableName>>("A") = {_A_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    action->getObjectParams().set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};

    action->getObjectParams().set<UserObjectName>("fp") = _fp_1phase_name;

    _this_action_warehouse.addActionBlock(action);
  }

  // dynamic viscosity
  {
    const std::string class_name = "AddMaterialAction";

    InputParameters params = _this_action_factory.getValidParams(class_name);
    if (_ad)
      params.set<std::string>("type") = "ADDynamicViscosityMaterial";
    else
      params.set<std::string>("type") = "DynamicViscosityMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, "mu_material", params));

    if (!_ad)
    {
      action->getObjectParams().set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
      action->getObjectParams().set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
      action->getObjectParams().set<std::vector<VariableName>>("arhoEA") = {_rhoEA_name};
    }

    action->getObjectParams().set<MaterialPropertyName>("mu") = {_mu_name};
    action->getObjectParams().set<MaterialPropertyName>("v") = {_v_name};
    action->getObjectParams().set<MaterialPropertyName>("e") = {_e_name};
    action->getObjectParams().set<UserObjectName>("fp_1phase") = _fp_1phase_name;

    _this_action_warehouse.addActionBlock(action);
  }

  // hydraulic diameter
  {
    InputParameters params = _this_action_factory.getValidParams(class_name);
    if (_ad)
      params.set<std::string>("type") = "ADGenericFunctionMaterial";
    else
      params.set<std::string>("type") = "GenericFunctionMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, "D_h_material", params));

    action->getObjectParams().set<std::vector<std::string>>("prop_names") = {_D_h_name};
    action->getObjectParams().set<std::vector<FunctionName>>("prop_values") = {_D_h_fn};

    _this_action_warehouse.addActionBlock(action);
  }
}

void
FlowModelSetup1Phase::addUserObjects()
{
}
