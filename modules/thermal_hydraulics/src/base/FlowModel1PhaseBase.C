//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModel1PhaseBase.h"
#include "FlowChannel1PhaseBase.h"
#include "THMNames.h"

InputParameters
FlowModel1PhaseBase::validParams()
{
  InputParameters params = FlowModel::validParams();
  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");
  params.addRequiredParam<MooseEnum>("rdg_slope_reconstruction",
                                     "Slope reconstruction type for rDG");
  return params;
}

FlowModel1PhaseBase::FlowModel1PhaseBase(const InputParameters & params)
  : FlowModel(params),
    _rdg_slope_reconstruction(params.get<MooseEnum>("rdg_slope_reconstruction")),
    _numerical_flux_name(params.get<UserObjectName>("numerical_flux"))
{
}

void
FlowModel1PhaseBase::addVariables()
{
  FlowModel::addCommonVariables();

  const std::vector<SubdomainName> & subdomains = _flow_channel.getSubdomainNames();

  // Nonlinear variables
  _sim.addSimVariable(true, THM::RHOA, _fe_type, subdomains, getScalingFactorRhoA());
  _sim.addSimVariable(true, THM::RHOUA, _fe_type, subdomains, getScalingFactorRhoUA());
  _sim.addSimVariable(true, THM::RHOEA, _fe_type, subdomains, getScalingFactorRhoEA());

  // Auxiliary variables
  _sim.addSimVariable(false, THM::DENSITY, _fe_type, subdomains);
  if (_output_vector_velocity)
  {
    _sim.addSimVariable(false, THM::VELOCITY_X, _fe_type, subdomains);
    _sim.addSimVariable(false, THM::VELOCITY_Y, _fe_type, subdomains);
    _sim.addSimVariable(false, THM::VELOCITY_Z, _fe_type, subdomains);
  }
  else
    _sim.addSimVariable(false, THM::VELOCITY, _fe_type, subdomains);
  _sim.addSimVariable(false, THM::PRESSURE, _fe_type, subdomains);
  _sim.addSimVariable(false, THM::SPECIFIC_VOLUME, _fe_type, subdomains);
  _sim.addSimVariable(false, THM::SPECIFIC_INTERNAL_ENERGY, _fe_type, subdomains);
  _sim.addSimVariable(false, THM::TEMPERATURE, _fe_type, subdomains);
  _sim.addSimVariable(false, THM::SPECIFIC_TOTAL_ENTHALPY, _fe_type, subdomains);

  _solution_vars = solutionVariableNames();
  _derivative_vars = solutionVariableNames();
}

void
FlowModel1PhaseBase::addInitialConditions()
{
  FlowModel::addCommonInitialConditions();

  if (ICParametersAreValid())
  {
    addRhoAIC();
    addRhoUAIC();
    addRhoEAIC();

    addFunctionIC(THM::PRESSURE, _flow_channel.getParam<FunctionName>("initial_p"));
    addFunctionIC(THM::TEMPERATURE, _flow_channel.getParam<FunctionName>("initial_T"));
    addVelocityIC();
    addDensityIC();
    addSpecificVolumeIC();
    addSpecificInternalEnergyIC();
    addSpecificTotalEnthalpyIC();
  }
}

bool
FlowModel1PhaseBase::ICParametersAreValid() const
{
  const auto & flow_channel_1phase_base =
      dynamic_cast<const FlowChannel1PhaseBase &>(_flow_channel);

  for (const auto & param : flow_channel_1phase_base.ICParameters())
    if (!_flow_channel.isParamValid(param))
      return false;

  return true;
}

void
FlowModel1PhaseBase::addFunctionIC(const VariableName & var_name,
                                   const FunctionName & function_name)
{
  const std::string class_name = "FunctionIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<FunctionName>("function") = function_name;
  _sim.addSimInitialCondition(class_name, genName(_comp_name, var_name + "_ic"), params);
}

void
FlowModel1PhaseBase::addRhoAIC()
{
  const std::string class_name = "VariableProductIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::RHOA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("values") = {THM::DENSITY, THM::AREA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoA_ic"), params);
}

void
FlowModel1PhaseBase::addRhoUAIC()
{
  const std::string class_name = "VariableFunctionProductIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::RHOUA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("var") = {THM::RHOA};
  params.set<FunctionName>("fn") = _flow_channel.getParam<FunctionName>("initial_vel");
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhouA_ic"), params);
}

void
FlowModel1PhaseBase::addVelocityIC()
{
  if (_output_vector_velocity)
  {
    std::vector<VariableName> var_name = {THM::VELOCITY_X, THM::VELOCITY_Y, THM::VELOCITY_Z};
    for (const auto i : make_range(Moose::dim))
    {
      const std::string class_name = "VectorVelocityIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = var_name[i];
      params.set<FunctionName>("vel_fn") = _flow_channel.getParam<FunctionName>("initial_vel");
      params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
      params.set<unsigned int>("component") = i;
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "vel_ic", i), params);
    }
  }
  else
    addFunctionIC(THM::VELOCITY, _flow_channel.getParam<FunctionName>("initial_vel"));
}

void
FlowModel1PhaseBase::addSpecificVolumeIC()
{
  const std::string class_name = "SpecificVolumeIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::SPECIFIC_VOLUME;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "v_ic"), params);
}

void
FlowModel1PhaseBase::addSpecificInternalEnergyIC()
{
  const std::string class_name = "SpecificInternalEnergyIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::SPECIFIC_INTERNAL_ENERGY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "e_ic"), params);
}

void
FlowModel1PhaseBase::addSpecificTotalEnthalpyIC()
{
  const std::string class_name = "SpecificTotalEnthalpyIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::SPECIFIC_TOTAL_ENTHALPY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "H_ic"), params);
}

void
FlowModel1PhaseBase::addMooseObjects()
{
  FlowModel::addCommonMooseObjects();

  addKernels();
  addDGKernels();
  addAuxKernels();
  addFluidPropertiesMaterials();
  addNumericalFluxUserObject();
  addRDGMooseObjects();
}

void
FlowModel1PhaseBase::addKernels()
{
  // Mass equation
  addTimeDerivativeKernelIfTransient(THM::RHOA);

  // Momentum equation
  addTimeDerivativeKernelIfTransient(THM::RHOUA);
  addMomentumAreaGradientKernel();
  addMomentumFrictionKernel();
  addMomentumGravityKernel();

  // Energy equation
  addTimeDerivativeKernelIfTransient(THM::RHOEA);
  addEnergyGravityKernel();
}

void
FlowModel1PhaseBase::addTimeDerivativeKernelIfTransient(const VariableName & var_name)
{
  if (_flow_channel.problemIsTransient())
  {
    const std::string class_name = "ADTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    _sim.addKernel(class_name, genName(_comp_name, var_name + "_td"), params);
  }
}

void
FlowModel1PhaseBase::addMomentumAreaGradientKernel()
{
  const std::string class_name = "ADOneD3EqnMomentumAreaGradient";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::RHOUA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A") = {THM::AREA_LINEAR};
  params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
  params.set<MaterialPropertyName>("p") = THM::PRESSURE;
  _sim.addKernel(class_name, genName(_comp_name, "mom_area_grad"), params);
}

void
FlowModel1PhaseBase::addMomentumFrictionKernel()
{
  const std::string class_name = "ADOneD3EqnMomentumFriction";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::RHOUA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<MaterialPropertyName>("D_h") = {THM::HYDRAULIC_DIAMETER};
  params.set<MaterialPropertyName>("rho") = THM::DENSITY;
  params.set<MaterialPropertyName>("vel") = THM::VELOCITY;
  params.set<MaterialPropertyName>("f_D") = THM::FRICTION_FACTOR_DARCY;
  _sim.addKernel(class_name, genName(_comp_name, "mom_friction"), params);
}

void
FlowModel1PhaseBase::addMomentumGravityKernel()
{
  const std::string class_name = "ADOneD3EqnMomentumGravity";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::RHOUA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
  params.set<MaterialPropertyName>("rho") = THM::DENSITY;
  params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
  _sim.addKernel(class_name, genName(_comp_name, "mom_gravity"), params);
}

void
FlowModel1PhaseBase::addEnergyGravityKernel()
{
  const std::string class_name = "ADOneD3EqnEnergyGravity";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::RHOEA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
  params.set<MaterialPropertyName>("rho") = THM::DENSITY;
  params.set<MaterialPropertyName>("vel") = THM::VELOCITY;
  params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
  _sim.addKernel(class_name, genName(_comp_name, "energy_gravity"), params);
}

void
FlowModel1PhaseBase::addDGKernels()
{
}

void
FlowModel1PhaseBase::addAuxKernels()
{
  addPressureAux();
  addTemperatureAux();
  addVelocityAux();
  addDensityAux();
  addSpecificVolumeAux();
  addSpecificInternalEnergyAux();
  addSpecificTotalEnthalpyAux();
}

void
FlowModel1PhaseBase::addVelocityAux()
{
  if (_output_vector_velocity)
  {
    std::vector<AuxVariableName> var_names = {THM::VELOCITY_X, THM::VELOCITY_Y, THM::VELOCITY_Z};
    for (const auto i : make_range(Moose::dim))
    {
      const std::string class_name = "ADVectorVelocityComponentAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = var_names[i];
      params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
      params.set<std::vector<VariableName>>("arhoA") = {THM::RHOA};
      params.set<std::vector<VariableName>>("arhouA") = {THM::RHOUA};
      params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
      params.set<unsigned int>("component") = i;
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      _sim.addAuxKernel(class_name, genName(_comp_name, i, "vel_vec"), params);
    }
  }
  else
  {
    const std::string class_name = "QuotientAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = THM::VELOCITY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("denominator") = {THM::RHOA};
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    _sim.addAuxKernel(class_name, genName(_comp_name, "vel"), params);
  }
}

void
FlowModel1PhaseBase::addDensityAux()
{
  const std::string class_name = "QuotientAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::DENSITY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("numerator") = {THM::RHOA};
  params.set<std::vector<VariableName>>("denominator") = {THM::AREA};
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "rho_aux"), params);
}

void
FlowModel1PhaseBase::addSpecificVolumeAux()
{
  const std::string class_name = "THMSpecificVolumeAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::SPECIFIC_VOLUME;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "v_aux"), params);
}

void
FlowModel1PhaseBase::addSpecificInternalEnergyAux()
{
  const std::string class_name = "THMSpecificInternalEnergyAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::SPECIFIC_INTERNAL_ENERGY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "e_aux"), params);
}

void
FlowModel1PhaseBase::addSpecificTotalEnthalpyAux()
{
  const std::string class_name = "SpecificTotalEnthalpyAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::SPECIFIC_TOTAL_ENTHALPY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("p") = {THM::PRESSURE};
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "H_auxkernel"), params);
}

void
FlowModel1PhaseBase::addRDGMooseObjects()
{
  addSlopeReconstructionMaterial();
  addRDGAdvectionDGKernels();
}
