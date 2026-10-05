//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1PhasePhysics.h"
#include "THMNames.h"
#include "THMUtils.h"
#include "MooseUtils.h"
#include "Function.h"
#include "ClosuresRegistry.h"
#include "ClosuresBase.h"
#include "InputParameterWarehouse.h"

registerPhysicsBaseTasks("ThermalHydraulicsApp", FlowChannel1PhasePhysics);
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_variables_physics");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_aux_variable");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_ics_physics");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_kernel");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_dg_kernel");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_aux_kernel");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_material");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "add_user_object");
registerMooseAction("ThermalHydraulicsApp", FlowChannel1PhasePhysics, "check_integrity");

InputParameters
FlowChannel1PhasePhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params += GravityInterface::validParams();
  params.addClassDescription(
      "Single-phase flow (mass, momentum, energy) physics for a 1D flow channel "
      "ActionComponent, without wall friction or wall heat transfer.");

  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object that defines fluid "
                                          "properties");
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addRequiredParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addRequiredParam<FunctionName>("initial_vel", "Initial velocity [m/s]");
  params.addRequiredParam<std::vector<Real>>(
      "scaling_factor_1phase",
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");

  params.addParam<std::vector<std::string>>(
      "closures",
      {},
      "Closures object(s) providing wall friction. This is optional since closure relations can "
      "be supplied directly by Materials as well.");
  params.addParam<FunctionName>("f", "Wall friction factor [-]");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter [m]");
  params.addParam<Real>("roughness", 0.0, "Roughness [m]");
  params.addParam<Real>("PoD", 1, "Pitch-to-diameter ratio for parallel bundle heat transfer [-]");

  return params;
}

FlowChannel1PhasePhysics::FlowChannel1PhasePhysics(const InputParameters & params)
  : PhysicsBase(params),
    GravityInterface(params),
    _fp_name(getParam<UserObjectName>("fp")),
    _area_fn_name(getParam<FunctionName>("A")),
    _initial_p_fn(getParam<FunctionName>("initial_p")),
    _initial_T_fn(getParam<FunctionName>("initial_T")),
    _initial_vel_fn(getParam<FunctionName>("initial_vel")),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factor_1phase")),
    _numerical_flux_name(genName(name(), "numerical_flux")),
    _coordinator(THMVariableCoordinator::findOrCreate(_awh, _action_factory))
{
  if (_scaling_factors.size() != 3)
    paramError("scaling_factor_1phase", "Must have exactly 3 entries (for rhoA, rhouA, rhoEA)");
}

void
FlowChannel1PhasePhysics::connectClosuresObject(const InputParameters & obj_params,
                                                const std::string & obj_name,
                                                const std::string & param) const
{
  MooseObjectParameterName alias("physics", name(), param, "::");
  MooseObjectParameterName obj_controlled_param(obj_params.getBase(), obj_name, param);
  _app.getInputParameterWarehouse().addControllableParameterAlias(alias, obj_controlled_param);
}

void
FlowChannel1PhasePhysics::addHydraulicDiameterMaterial()
{
  const std::string mat_name = genName(name(), "D_h_material");

  if (isParamValid("D_h"))
  {
    const FunctionName & D_h_fn_name = getParam<FunctionName>("D_h");

    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    params.set<std::vector<std::string>>("prop_names") = {THM::HYDRAULIC_DIAMETER};
    params.set<std::vector<FunctionName>>("prop_values") = {D_h_fn_name};
    getProblem().addMaterial(class_name, mat_name, params);
  }
  else
  {
    const std::string class_name = "ADHydraulicDiameterCircularMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    params.set<MaterialPropertyName>("D_h_name") = THM::HYDRAULIC_DIAMETER;
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    getProblem().addMaterial(class_name, mat_name, params);
  }
}

void
FlowChannel1PhasePhysics::checkIntegrity() const
{
  ClosuresRegistry & registry = ClosuresRegistry::findOrCreate(_awh, _action_factory);
  for (const auto & closures_name : getParam<std::vector<std::string>>("closures"))
    registry.getClosures(closures_name)->checkFlowChannel(*this);

  registry.emitLoggedMessages();
}

void
FlowChannel1PhasePhysics::addFlowVariable(bool nl,
                                          const VariableName & var_name,
                                          const std::string & family,
                                          const std::string & order,
                                          Real scaling_factor)
{
  _coordinator.requestVariable(nl, var_name, family, order, scaling_factor, _blocks);

  if (nl)
    saveSolverVariableName(var_name);
  else
    saveAuxVariableName(var_name);
}

void
FlowChannel1PhasePhysics::addSolverVariables()
{
  addFlowVariable(true, THM::RHOA, "MONOMIAL", "CONSTANT", _scaling_factors[0]);
  addFlowVariable(true, THM::RHOUA, "MONOMIAL", "CONSTANT", _scaling_factors[1]);
  addFlowVariable(true, THM::RHOEA, "MONOMIAL", "CONSTANT", _scaling_factors[2]);
}

void
FlowChannel1PhasePhysics::addAuxiliaryVariables()
{
  addFlowVariable(false, THM::AREA);
  addFlowVariable(false, THM::AREA_LINEAR, "LAGRANGE", "FIRST");
  addFlowVariable(false, THM::DENSITY);
  addFlowVariable(false, THM::VELOCITY);
  addFlowVariable(false, THM::PRESSURE);
  addFlowVariable(false, THM::SPECIFIC_VOLUME);
  addFlowVariable(false, THM::SPECIFIC_INTERNAL_ENERGY);
  addFlowVariable(false, THM::TEMPERATURE);
  addFlowVariable(false, THM::SPECIFIC_TOTAL_ENTHALPY);
}

void
FlowChannel1PhasePhysics::addFunctionIC(const VariableName & var_name,
                                        const FunctionName & function_name)
{
  const std::string class_name = "FunctionIC";
  InputParameters params = getFactory().getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  assignBlocks(params, _blocks);
  params.set<FunctionName>("function") = function_name;
  getProblem().addInitialCondition(class_name, genName(name(), var_name, "ic"), params);
}

void
FlowChannel1PhasePhysics::addAreaInitialCondition()
{
  // AREA/AREA_LINEAR are otherwise only set by AuxKernels (see addAuxiliaryKernels()), which run
  // after initial conditions are projected - too late for ICs that read AREA's value (e.g.
  // VariableProductIC for rhoA). Seed them here too, mirroring
  // FlowModel::addCommonInitialConditions().
  if (!getProblem().hasFunction(_area_fn_name))
  {
    // 'A' was given as a plain constant, not a named function
    const Function & fn = getProblem().getFunction(_area_fn_name);
    const Real area_value = fn.value(0, Point());
    for (const auto & var_name : {THM::AREA, THM::AREA_LINEAR})
    {
      const std::string class_name = "ConstantIC";
      InputParameters params = getFactory().getValidParams(class_name);
      params.set<VariableName>("variable") = var_name;
      assignBlocks(params, _blocks);
      params.set<Real>("value") = area_value;
      getProblem().addInitialCondition(class_name, genName(name(), var_name, "ic"), params);
    }
  }
  else
  {
    addFunctionIC(THM::AREA_LINEAR, _area_fn_name);

    const std::string class_name = "FunctionNodalAverageIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::AREA;
    assignBlocks(params, _blocks);
    params.set<FunctionName>("function") = _area_fn_name;
    getProblem().addInitialCondition(class_name, genName(name(), THM::AREA, "ic"), params);
  }
}

void
FlowChannel1PhasePhysics::addInitialConditions()
{
  addAreaInitialCondition();

  addFunctionIC(THM::PRESSURE, _initial_p_fn);
  addFunctionIC(THM::TEMPERATURE, _initial_T_fn);
  addFunctionIC(THM::VELOCITY, _initial_vel_fn);

  {
    const std::string class_name = "VariableProductIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::RHOA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("values") = {THM::DENSITY, THM::AREA};
    getProblem().addInitialCondition(class_name, genName(name(), "rhoA_ic"), params);
  }
  {
    const std::string class_name = "VariableFunctionProductIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::RHOUA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("var") = {THM::RHOA};
    params.set<FunctionName>("fn") = _initial_vel_fn;
    getProblem().addInitialCondition(class_name, genName(name(), "rhouA_ic"), params);
  }
  {
    const std::string class_name = "RhoEAFromPressureTemperatureFunctionVelocityIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::RHOEA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
    params.set<std::vector<VariableName>>("T") = {THM::TEMPERATURE};
    params.set<FunctionName>("vel") = _initial_vel_fn;
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<UserObjectName>("fp") = _fp_name;
    getProblem().addInitialCondition(class_name, genName(name(), "rhoEA_ic"), params);
  }
  {
    const std::string class_name = "RhoFromPressureTemperatureIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::DENSITY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
    params.set<std::vector<VariableName>>("T") = {THM::TEMPERATURE};
    params.set<UserObjectName>("fp") = _fp_name;
    getProblem().addInitialCondition(class_name, genName(name(), "rho_ic"), params);
  }
  {
    const std::string class_name = "SpecificVolumeIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::SPECIFIC_VOLUME;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    getProblem().addInitialCondition(class_name, genName(name(), "v_ic"), params);
  }
  {
    const std::string class_name = "SpecificInternalEnergyIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::SPECIFIC_INTERNAL_ENERGY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    getProblem().addInitialCondition(class_name, genName(name(), "e_ic"), params);
  }
  {
    const std::string class_name = "SpecificTotalEnthalpyIC";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<VariableName>("variable") = THM::SPECIFIC_TOTAL_ENTHALPY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    getProblem().addInitialCondition(class_name, genName(name(), "H_ic"), params);
  }
}

void
FlowChannel1PhasePhysics::addTimeDerivativeKernelIfTransient(const VariableName & var_name)
{
  if (isTransient())
  {
    const std::string class_name = "ADTimeDerivative";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var_name;
    assignBlocks(params, _blocks);
    getProblem().addKernel(class_name, genName(name(), var_name, "td"), params);
  }
}

void
FlowChannel1PhasePhysics::addFEKernels()
{
  // Mass equation
  addTimeDerivativeKernelIfTransient(THM::RHOA);

  // Momentum equation
  addTimeDerivativeKernelIfTransient(THM::RHOUA);
  if (!getParam<std::vector<std::string>>("closures").empty())
  {
    const std::string class_name = "ADOneD3EqnMomentumFriction";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = THM::RHOUA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<MaterialPropertyName>("D_h") = THM::HYDRAULIC_DIAMETER;
    params.set<MaterialPropertyName>("rho") = THM::DENSITY;
    params.set<MaterialPropertyName>("vel") = THM::VELOCITY;
    params.set<MaterialPropertyName>("f_D") = THM::FRICTION_FACTOR_DARCY;
    getProblem().addKernel(class_name, genName(name(), "mom_friction"), params);
  }
  {
    const std::string class_name = "ADOneD3EqnMomentumAreaGradient";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = THM::RHOUA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("A") = {THM::AREA_LINEAR};
    params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
    params.set<MaterialPropertyName>("p") = THM::PRESSURE;
    getProblem().addKernel(class_name, genName(name(), "mom_area_grad"), params);
  }
  {
    const std::string class_name = "ADOneD3EqnMomentumGravity";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = THM::RHOUA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
    params.set<MaterialPropertyName>("rho") = THM::DENSITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    getProblem().addKernel(class_name, genName(name(), "mom_gravity"), params);
  }

  // Energy equation
  addTimeDerivativeKernelIfTransient(THM::RHOEA);
  {
    const std::string class_name = "ADOneD3EqnEnergyGravity";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = THM::RHOEA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
    params.set<MaterialPropertyName>("rho") = THM::DENSITY;
    params.set<MaterialPropertyName>("vel") = THM::VELOCITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    getProblem().addKernel(class_name, genName(name(), "energy_gravity"), params);
  }
}

void
FlowChannel1PhasePhysics::addDGKernels()
{
  for (const auto & var : {THM::RHOA, THM::RHOUA, THM::RHOEA})
  {
    const std::string class_name = "ADNumericalFlux3EqnDGKernel";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<std::vector<VariableName>>("passives_times_area") = {};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = THM::implicitTimeIntegrationFlag(_app);
    getProblem().addDGKernel(class_name, genName(name(), "flux_kernel_" + var), params);
  }
}

void
FlowChannel1PhasePhysics::addMaterials()
{
  {
    const std::string class_name = "DirectionMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    getProblem().addMaterial(class_name, genName(name(), "dir_mat"), params);
  }
  {
    const std::string class_name = "ADFluidProperties3EqnMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    getProblem().addMaterial(class_name, genName(name(), "fp_mat"), params);
  }
  {
    const std::string class_name = "ADDynamicViscosityMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("fp_1phase") = _fp_name;
    params.set<MaterialPropertyName>("mu") = THM::DYNAMIC_VISCOSITY;
    params.set<MaterialPropertyName>("v") = THM::SPECIFIC_VOLUME;
    params.set<MaterialPropertyName>("e") = THM::SPECIFIC_INTERNAL_ENERGY;
    getProblem().addMaterial(class_name, genName(name(), "mu_mat"), params);
  }
  {
    // No slope reconstruction (piecewise-constant / first-order) for now - see class documentation
    const std::string class_name = "ADRDG3EqnMaterial";
    InputParameters params = getFactory().getValidParams(class_name);
    assignBlocks(params, _blocks);
    params.set<MooseEnum>("scheme") = "None";
    params.set<std::vector<VariableName>>("A_elem") = {THM::AREA};
    params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<std::vector<VariableName>>("passives_times_area") = {};
    params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<bool>("implicit") = THM::implicitTimeIntegrationFlag(_app);
    getProblem().addMaterial(class_name, genName(name(), "rdg_3eqn_mat"), params);
  }

  const auto & closures_names = getParam<std::vector<std::string>>("closures");
  if (!closures_names.empty())
  {
    addHydraulicDiameterMaterial();

    ClosuresRegistry & registry = ClosuresRegistry::findOrCreate(_awh, _action_factory);
    for (const auto & closures_name : closures_names)
      registry.getClosures(closures_name)->addMooseObjectsFlowChannel(*this);
  }
}

void
FlowChannel1PhasePhysics::addUserObjects()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = getFactory().getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<MooseEnum>("emit_on_nan") = "none";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  addUserObject(class_name, _numerical_flux_name, params);
}

void
FlowChannel1PhasePhysics::addAuxiliaryKernels()
{
  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};
  ExecFlagEnum state_execute_on(MooseUtils::getDefaultExecFlagEnum());
  state_execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  {
    const std::string class_name = "FunctionAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::AREA_LINEAR;
    assignBlocks(params, _blocks);
    params.set<FunctionName>("function") = _area_fn_name;
    params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "area_linear_aux"), params);
  }
  {
    const std::string class_name = "ProjectionAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::AREA;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("v") = {THM::AREA_LINEAR};
    params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "area_aux"), params);
  }
  {
    const std::string class_name = "PressureAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::PRESSURE;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("e") = {THM::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {THM::SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    getProblem().addAuxKernel(class_name, genName(name(), "pressure_uv_auxkernel"), params);
  }
  {
    const std::string class_name = "TemperatureAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::TEMPERATURE;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("e") = {THM::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {THM::SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    getProblem().addAuxKernel(class_name, genName(name(), "T_auxkernel"), params);
  }
  {
    const std::string class_name = "QuotientAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::VELOCITY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("numerator") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("denominator") = {THM::RHOA};
    params.set<ExecFlagEnum>("execute_on") = state_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "vel"), params);
  }
  {
    const std::string class_name = "QuotientAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::DENSITY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("numerator") = {THM::RHOA};
    params.set<std::vector<VariableName>>("denominator") = {THM::AREA};
    params.set<ExecFlagEnum>("execute_on") = state_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "rho_aux"), params);
  }
  {
    const std::string class_name = "THMSpecificVolumeAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::SPECIFIC_VOLUME;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<ExecFlagEnum>("execute_on") = state_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "v_aux"), params);
  }
  {
    const std::string class_name = "THMSpecificInternalEnergyAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::SPECIFIC_INTERNAL_ENERGY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<ExecFlagEnum>("execute_on") = state_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "e_aux"), params);
  }
  {
    const std::string class_name = "SpecificTotalEnthalpyAux";
    InputParameters params = getFactory().getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::SPECIFIC_TOTAL_ENTHALPY;
    assignBlocks(params, _blocks);
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
    params.set<std::vector<VariableName>>("A") = {THM::AREA};
    params.set<ExecFlagEnum>("execute_on") = state_execute_on;
    getProblem().addAuxKernel(class_name, genName(name(), "H_auxkernel"), params);
  }
}
