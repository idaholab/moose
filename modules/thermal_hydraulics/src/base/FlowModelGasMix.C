//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelGasMix.h"
#include "FlowChannelBase.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", FlowModelGasMix);

InputParameters
FlowModelGasMix::validParams()
{
  InputParameters params = FlowModel1PhaseBase::validParams();

  params.addRequiredParam<Real>("scaling_factor_xirhoA",
                                "Scaling factor for the secondary component mass equation");
  params.addRequiredParam<Real>("scaling_factor_rhoA",
                                "Scaling factor for the mixture mass equation");
  params.addRequiredParam<Real>("scaling_factor_rhouA", "Scaling factor for the momentum equation");
  params.addRequiredParam<Real>("scaling_factor_rhoEA", "Scaling factor for the energy equation");

  return params;
}

FlowModelGasMix::FlowModelGasMix(const InputParameters & params) : FlowModel1PhaseBase(params) {}

void
FlowModelGasMix::addVariables()
{
  FlowModel1PhaseBase::addVariables();

  const std::vector<SubdomainName> & subdomains = _flow_channel.getSubdomainNames();

  _sim.addSimVariable(
      true, THM::XIRHOA, _fe_type, subdomains, getParam<Real>("scaling_factor_xirhoA"));
  _sim.addSimVariable(false, THM::MASS_FRACTION, _fe_type, subdomains);
}

Real
FlowModelGasMix::getScalingFactorRhoA() const
{
  return getParam<Real>("scaling_factor_rhoA");
}

Real
FlowModelGasMix::getScalingFactorRhoUA() const
{
  return getParam<Real>("scaling_factor_rhouA");
}

Real
FlowModelGasMix::getScalingFactorRhoEA() const
{
  return getParam<Real>("scaling_factor_rhoEA");
}

std::vector<VariableName>
FlowModelGasMix::solutionVariableNames() const
{
  return {THM::XIRHOA, THM::RHOA, THM::RHOUA, THM::RHOEA};
}

void
FlowModelGasMix::addInitialConditions()
{
  FlowModel1PhaseBase::addInitialConditions();

  if (ICParametersAreValid())
  {
    addXiRhoAIC();
    addFunctionIC(THM::MASS_FRACTION,
                  _flow_channel.getParam<FunctionName>("initial_mass_fraction"));
  }
}

void
FlowModelGasMix::addXiRhoAIC()
{
  const std::string class_name = "VariableProductIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::XIRHOA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("values") = {THM::MASS_FRACTION, THM::RHOA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "xirhoA_ic"), params);
}

void
FlowModelGasMix::addRhoEAIC()
{
  const std::string class_name = "FlowModelGasMixIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::RHOEA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("quantity") = "rhoEA";
  params.set<FunctionName>("mass_fraction") =
      _flow_channel.getParam<FunctionName>("initial_mass_fraction");
  params.set<FunctionName>("pressure") = _flow_channel.getParam<FunctionName>("initial_p");
  params.set<FunctionName>("temperature") = _flow_channel.getParam<FunctionName>("initial_T");
  params.set<FunctionName>("velocity") = _flow_channel.getParam<FunctionName>("initial_vel");
  params.set<std::vector<VariableName>>("area") = {THM::AREA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoEA_ic"), params);
}

void
FlowModelGasMix::addDensityIC()
{
  const std::string class_name = "FlowModelGasMixIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = THM::DENSITY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("quantity") = "rho";
  params.set<FunctionName>("mass_fraction") =
      _flow_channel.getParam<FunctionName>("initial_mass_fraction");
  params.set<FunctionName>("pressure") = _flow_channel.getParam<FunctionName>("initial_p");
  params.set<FunctionName>("temperature") = _flow_channel.getParam<FunctionName>("initial_T");
  params.set<FunctionName>("velocity") = _flow_channel.getParam<FunctionName>("initial_vel");
  params.set<std::vector<VariableName>>("area") = {THM::AREA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rho_ic"), params);
}

void
FlowModelGasMix::addKernels()
{
  FlowModel1PhaseBase::addKernels();

  addTimeDerivativeKernelIfTransient(THM::XIRHOA);
}

void
FlowModelGasMix::addDGKernels()
{
  FlowModel1PhaseBase::addDGKernels();

  addMassDiffusionSpeciesDGKernel();
  addMassDiffusionEnergyDGKernel();
}

void
FlowModelGasMix::addMassDiffusionSpeciesDGKernel()
{
  const std::string class_name = "MassDiffusionSpeciesGasMixDGKernel";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::XIRHOA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<MaterialPropertyName>("density") = THM::DENSITY;
  params.set<MaterialPropertyName>("diffusion_coefficient") = THM::MASS_DIFFUSION_COEFFICIENT;
  params.set<MaterialPropertyName>("mass_fraction") = THM::MASS_FRACTION;
  params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
  params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
  _sim.addDGKernel(class_name, genName(_comp_name, "mass_diffusion_species"), params);
}

void
FlowModelGasMix::addMassDiffusionEnergyDGKernel()
{
  const std::string class_name = "MassDiffusionEnergyGasMixDGKernel";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = THM::RHOEA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<MaterialPropertyName>("density") = THM::DENSITY;
  params.set<MaterialPropertyName>("pressure") = THM::PRESSURE;
  params.set<MaterialPropertyName>("temperature") = THM::TEMPERATURE;
  params.set<MaterialPropertyName>("velocity") = THM::VELOCITY;
  params.set<MaterialPropertyName>("diffusion_coefficient") = THM::MASS_DIFFUSION_COEFFICIENT;
  params.set<MaterialPropertyName>("mass_fraction") = THM::MASS_FRACTION;
  params.set<MaterialPropertyName>("direction") = THM::DIRECTION;
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
  _sim.addDGKernel(class_name, genName(_comp_name, "mass_diffusion_energy"), params);
}

void
FlowModelGasMix::addAuxKernels()
{
  FlowModel1PhaseBase::addAuxKernels();
  addMassFractionAux();
}

void
FlowModelGasMix::addPressureAux()
{
  const std::string class_name = "FlowModelGasMixAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::PRESSURE;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("quantity") = "p";
  params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("area") = {THM::AREA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "p_aux"), params);
}

void
FlowModelGasMix::addTemperatureAux()
{
  const std::string class_name = "FlowModelGasMixAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::TEMPERATURE;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("quantity") = "T";
  params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("area") = {THM::AREA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "T_aux"), params);
}

void
FlowModelGasMix::addMassFractionAux()
{
  const std::string class_name = "QuotientAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = THM::MASS_FRACTION;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("numerator") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("denominator") = {THM::RHOA};
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  _sim.addAuxKernel(class_name, genName(_comp_name, "xi_aux"), params);
}

void
FlowModelGasMix::addFluidPropertiesMaterials()
{
  const std::string class_name = "FluidPropertiesGasMixMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("area") = {THM::AREA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _sim.addMaterial(class_name, genName(_comp_name, "fp_mat"), params);
}

void
FlowModelGasMix::addNumericalFluxUserObject()
{
  const std::string class_name = "NumericalFluxGasMixHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<MooseEnum>("emit_on_nan") = "none";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  _sim.addUserObject(class_name, _numerical_flux_name, params);
}

void
FlowModelGasMix::addSlopeReconstructionMaterial()
{
  const std::string class_name = "SlopeReconstructionGasMixMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
  params.set<std::vector<VariableName>>("A_elem") = {THM::AREA};
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
  _sim.addMaterial(class_name, genName(_comp_name, "slope_mat"), params);
}

void
FlowModelGasMix::addRDGAdvectionDGKernels()
{
  const std::vector<NonlinearVariableName> vars{THM::XIRHOA, THM::RHOA, THM::RHOUA, THM::RHOEA};

  for (const auto & var : vars)
  {
    const std::string class_name = "NumericalFluxGasMixDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
    params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
    params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addDGKernel(class_name, genName(_comp_name, "advection:" + var), params);
  }
}
