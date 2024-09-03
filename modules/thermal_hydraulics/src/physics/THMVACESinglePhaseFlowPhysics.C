//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMVACESinglePhaseFlowPhysics.h"
#include "THMProblem.h"
#include "FlowChannelBase.h"
#include "SlopeReconstruction1DInterface.h"
#include "PhysicsFlowBoundary.h"

// TODO: consolidate those at the THMPhysics parent class level
typedef THMVACESinglePhaseFlowPhysics VACE1P;
const std::string VACE1P::DENSITY = "rho";
const std::string VACE1P::FRICTION_FACTOR_DARCY = "f_D";
const std::string VACE1P::DYNAMIC_VISCOSITY = "mu";
const std::string VACE1P::HEAT_TRANSFER_COEFFICIENT_WALL = "Hw";
const std::string VACE1P::HYDRAULIC_DIAMETER = "D_h";
const std::string VACE1P::PRESSURE = "p";
const std::string VACE1P::RHOA = "rhoA";
const std::string VACE1P::RHOEA = "rhoEA";
const std::string VACE1P::RHOUA = "rhouA";
const std::string VACE1P::SOUND_SPEED = "c";
const std::string VACE1P::SPECIFIC_HEAT_CONSTANT_PRESSURE = "cp";
const std::string VACE1P::SPECIFIC_HEAT_CONSTANT_VOLUME = "cv";
const std::string VACE1P::SPECIFIC_INTERNAL_ENERGY = "e";
const std::string VACE1P::SPECIFIC_TOTAL_ENTHALPY = "H";
const std::string VACE1P::SPECIFIC_VOLUME = "v";
const std::string VACE1P::TEMPERATURE = "T";
const std::string VACE1P::THERMAL_CONDUCTIVITY = "k";
const std::string VACE1P::VELOCITY = "vel";
const std::string VACE1P::VELOCITY_X = "vel_x";
const std::string VACE1P::VELOCITY_Y = "vel_y";
const std::string VACE1P::VELOCITY_Z = "vel_z";
const std::string VACE1P::REYNOLDS_NUMBER = "Re";

registerTHMFlowModelPhysicsBaseTasks("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics);
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_aux_variable");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "THMPhysics:add_ic");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_kernel");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_dg_kernel");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_aux_kernel");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_user_object");

InputParameters
THMVACESinglePhaseFlowPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();

  params.addRequiredParam<MooseEnum>(
      "rdg_slope_reconstruction",
      SlopeReconstruction1DInterface<true>::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction type for rDG");
  params.addRequiredParam<std::vector<Real>>(
      "scaling_factor_1phase",
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  return params;
}

THMVACESinglePhaseFlowPhysics::THMVACESinglePhaseFlowPhysics(const InputParameters & params)
  : ThermalHydraulicsFlowPhysics(params),
    _rdg_slope_reconstruction(params.get<MooseEnum>("rdg_slope_reconstruction")),
    _numerical_flux_name(prefix() + "VACE_uo"),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factor_1phase"))
{
}

void
THMVACESinglePhaseFlowPhysics::actOnAdditionalTasks()
{
  // The THMProblem adds ICs on THM:add_variables, which happens before add_ic
  if (_current_task == "THMPhysics:add_ic")
    addTHMInitialConditions();
}

void
THMVACESinglePhaseFlowPhysics::addNonlinearVariables()
{
  ThermalHydraulicsFlowPhysics::addCommonVariables();

  for (const auto flow_channel : _flow_channels)
  {
    const std::vector<SubdomainName> & subdomains = flow_channel->getSubdomainNames();

    _sim->addSimVariable(true, RHOA, _fe_type, subdomains, _scaling_factors[0]);
    _sim->addSimVariable(true, RHOUA, _fe_type, subdomains, _scaling_factors[1]);
    _sim->addSimVariable(true, RHOEA, _fe_type, subdomains, _scaling_factors[2]);
  }

  saveNonlinearVariableName(RHOA);
  saveNonlinearVariableName(RHOUA);
  saveNonlinearVariableName(RHOEA);
  _derivative_vars = {RHOA, RHOUA, RHOEA};
}

void
THMVACESinglePhaseFlowPhysics::addAuxiliaryVariables()
{
  for (const auto flow_channel : _flow_channels)
  {
    const std::vector<SubdomainName> & subdomains = flow_channel->getSubdomainNames();
    _sim->addSimVariable(false, DENSITY, _fe_type, subdomains);
    if (_output_vector_velocity)
    {
      _sim->addSimVariable(false, VELOCITY_X, _fe_type, subdomains);
      _sim->addSimVariable(false, VELOCITY_Y, _fe_type, subdomains);
      _sim->addSimVariable(false, VELOCITY_Z, _fe_type, subdomains);
    }
    else
      _sim->addSimVariable(false, VELOCITY, _fe_type, subdomains);
    _sim->addSimVariable(false, PRESSURE, _fe_type, subdomains);
    _sim->addSimVariable(false, SPECIFIC_VOLUME, _fe_type, subdomains);
    _sim->addSimVariable(false, SPECIFIC_INTERNAL_ENERGY, _fe_type, subdomains);
    _sim->addSimVariable(false, TEMPERATURE, _fe_type, subdomains);
    _sim->addSimVariable(false, SPECIFIC_TOTAL_ENTHALPY, _fe_type, subdomains);
  }
}

void
THMVACESinglePhaseFlowPhysics::addTHMInitialConditions()
{
  ThermalHydraulicsFlowPhysics::addCommonInitialConditions();

  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto comp_name = _component_names[i];

    if (flow_channel->isParamValid("initial_p") && flow_channel->isParamValid("initial_T") &&
        flow_channel->isParamValid("initial_vel"))
    {
      const std::vector<SubdomainName> & block = flow_channel->getSubdomainNames();

      const FunctionName & p_fn = getVariableFn("initial_p");
      _sim->addFunctionIC(PRESSURE, p_fn, block);

      const FunctionName & T_fn = getVariableFn("initial_T");
      _sim->addFunctionIC(TEMPERATURE, T_fn, block);

      const FunctionName & vel_fn = getVariableFn("initial_vel");
      if (_output_vector_velocity)
      {
        std::vector<VariableName> var_name = {VELOCITY_X, VELOCITY_Y, VELOCITY_Z};
        for (const auto i : make_range(Moose::dim))
        {
          std::string class_name = "VectorVelocityIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = var_name[i];
          params.set<FunctionName>("vel_fn") = vel_fn;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<unsigned int>("component") = i;
          _sim->addSimInitialCondition(class_name, genName(comp_name, "vel_ic", i), params);
        }

        {
          std::string class_name = "VariableFunctionProductIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = RHOUA;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<std::vector<VariableName>>("var") = {RHOA};
          params.set<FunctionName>("fn") = vel_fn;
          _sim->addSimInitialCondition(class_name, genName(comp_name, "rhouA_ic"), params);
        }
        {
          std::string class_name = "RhoEAFromPressureTemperatureFunctionVelocityIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = RHOEA;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<std::vector<VariableName>>("p") = {PRESSURE};
          params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
          params.set<FunctionName>("vel") = vel_fn;
          params.set<std::vector<VariableName>>("A") = {ThermalHydraulicsFlowPhysics::AREA};
          params.set<UserObjectName>("fp") = _fp_name;
          _sim->addSimInitialCondition(class_name, genName(comp_name, "rhoEA_ic"), params);
        }
      }
      else
      {
        _sim->addFunctionIC(VELOCITY, vel_fn, block);

        {
          std::string class_name = "VariableProductIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = RHOUA;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<std::vector<VariableName>>("values") = {
              DENSITY, VELOCITY, ThermalHydraulicsFlowPhysics::AREA};
          _sim->addSimInitialCondition(class_name, genName(comp_name, "rhouA_ic"), params);
        }
        {
          std::string class_name = "RhoEAFromPressureTemperatureVelocityIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = RHOEA;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<std::vector<VariableName>>("p") = {PRESSURE};
          params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
          params.set<std::vector<VariableName>>("vel") = {VELOCITY};
          params.set<std::vector<VariableName>>("A") = {ThermalHydraulicsFlowPhysics::AREA};
          params.set<UserObjectName>("fp") = _fp_name;
          _sim->addSimInitialCondition(class_name, genName(comp_name, "rhoEA_ic"), params);
        }
      }

      {
        std::string class_name = "RhoFromPressureTemperatureIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = DENSITY;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("p") = {PRESSURE};
        params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
        params.set<UserObjectName>("fp") = _fp_name;
        _sim->addSimInitialCondition(class_name, genName(comp_name, "rho_ic"), params);
      }

      {
        std::string class_name = "VariableProductIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = RHOA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("values") = {DENSITY,
                                                           ThermalHydraulicsFlowPhysics::AREA};
        _sim->addSimInitialCondition(class_name, genName(comp_name, "rhoA_ic"), params);
      }

      {
        std::string class_name = "SpecificVolumeIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = SPECIFIC_VOLUME;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("rhoA") = {RHOA};
        params.set<std::vector<VariableName>>("A") = {ThermalHydraulicsFlowPhysics::AREA};
        _sim->addSimInitialCondition(class_name, genName(comp_name, "v_ic"), params);
      }
      {
        std::string class_name = "SpecificInternalEnergyIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("rhoA") = {RHOA};
        params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
        params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
        _sim->addSimInitialCondition(class_name, genName(comp_name, "u_ic"), params);
      }
      {
        std::string class_name = "SpecificTotalEnthalpyIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = SPECIFIC_TOTAL_ENTHALPY;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("p") = {PRESSURE};
        params.set<std::vector<VariableName>>("rhoA") = {RHOA};
        params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
        params.set<std::vector<VariableName>>("A") = {ThermalHydraulicsFlowPhysics::AREA};
        _sim->addSimInitialCondition(class_name, genName(comp_name, "H_ic"), params);
      }
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addMaterials()
{
  ThermalHydraulicsFlowPhysics::addCommonMaterials();

  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto comp_name = _component_names[i];
    {
      std::string class_name = "ADFluidProperties3EqnMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<UserObjectName>("fp") = _fp_name;
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      params.set<std::vector<VariableName>>("A") = {ThermalHydraulicsFlowPhysics::AREA};
      _sim->addMaterial(class_name, genName(comp_name, "fp_mat"), params);
    }
    {
      const std::string class_name = "ADDynamicViscosityMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<UserObjectName>("fp_1phase") = _fp_name;
      params.set<MaterialPropertyName>("mu") = {DYNAMIC_VISCOSITY};
      params.set<MaterialPropertyName>("v") = {SPECIFIC_VOLUME};
      params.set<MaterialPropertyName>("e") = {SPECIFIC_INTERNAL_ENERGY};
      _sim->addMaterial(class_name, genName(comp_name, "mu_mat"), params);
    }
    // slope reconstruction material
    {
      const std::string class_name = "ADRDG3EqnMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
      params.set<std::vector<VariableName>>("A_elem") = {AREA};
      params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      params.set<MaterialPropertyName>("direction") = DIRECTION;
      params.set<UserObjectName>("fluid_properties") = _fp_name;
      params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();
      _sim->addMaterial(class_name, genName(comp_name, "rdg_3egn_mat"), params);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addFEKernels()
{
  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto comp_name = _component_names[i];
    // Density equation (transient term + advection term)
    if (flow_channel->problemIsTransient())
    {
      std::string class_name = "ADTimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      if (_lump_mass_matrix)
        params.set<bool>("lumping") = true;
      _sim->addKernel(class_name, genName(comp_name, "rho_ie"), params);
    }

    // Momentum equation, for 1-D flow channel, x-momentum equation only
    // (transient term + remaining terms[advection, pressure, body force, etc])
    if (flow_channel->problemIsTransient())
    {
      std::string class_name = "ADTimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOUA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      if (_lump_mass_matrix)
        params.set<bool>("lumping") = true;
      _sim->addKernel(class_name, genName(comp_name, "rhou_ie"), params);
    }
    {
      std::string class_name = "ADOneD3EqnMomentumAreaGradient";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOUA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("A") = {AREA_LINEAR};
      params.set<MaterialPropertyName>("direction") = DIRECTION;
      params.set<MaterialPropertyName>("p") = PRESSURE;
      _sim->addKernel(class_name, genName(comp_name, "rhou_ps"), params);
    }
    {
      std::string class_name = "ADOneD3EqnMomentumFriction";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOUA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("A") = {AREA};
      params.set<MaterialPropertyName>("D_h") = {HYDRAULIC_DIAMETER};
      params.set<MaterialPropertyName>("rho") = DENSITY;
      params.set<MaterialPropertyName>("vel") = VELOCITY;
      params.set<MaterialPropertyName>("f_D") = FRICTION_FACTOR_DARCY;
      _sim->addKernel(class_name, genName(comp_name, "rhou_friction"), params);
    }
    {
      std::string class_name = "ADOneD3EqnMomentumGravity";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOUA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("A") = {AREA};
      params.set<MaterialPropertyName>("direction") = DIRECTION;
      params.set<MaterialPropertyName>("rho") = DENSITY;
      params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
      _sim->addKernel(class_name, genName(comp_name, "rhou_gravity"), params);
    }

    // Total energy equation
    // (transient term + remaining terms[advection, wall heating, work from body force, etc])
    if (flow_channel->problemIsTransient())
    {
      std::string class_name = "ADTimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOEA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      if (_lump_mass_matrix)
        params.set<bool>("lumping") = true;
      _sim->addKernel(class_name, genName(comp_name, "rhoE_ie"), params);
    }
    {
      std::string class_name = "ADOneD3EqnEnergyGravity";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = RHOEA;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("A") = {AREA};
      params.set<MaterialPropertyName>("direction") = DIRECTION;
      params.set<MaterialPropertyName>("rho") = DENSITY;
      params.set<MaterialPropertyName>("vel") = VELOCITY;
      params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
      _sim->addKernel(class_name, genName(comp_name, "rhoE_gravity"), params);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addDGKernels()
{
  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto comp_name = _component_names[i];
    // mass
    const std::string class_name = "ADNumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();
    _sim->addDGKernel(class_name, genName(comp_name, "mass_advection"), params);

    // momentum
    params.set<NonlinearVariableName>("variable") = RHOUA;
    _sim->addDGKernel(class_name, genName(comp_name, "momentum_advection"), params);

    // energy
    params.set<NonlinearVariableName>("variable") = RHOEA;
    _sim->addDGKernel(class_name, genName(comp_name, "energy_advection"), params);
  }
}

void
THMVACESinglePhaseFlowPhysics::addAuxiliaryKernels()
{
  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto comp_name = _component_names[i];
    if (_output_vector_velocity)
    {
      // Vector-valued velocity
      {
        ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
        execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};

        std::vector<AuxVariableName> var_names = {VELOCITY_X, VELOCITY_Y, VELOCITY_Z};
        for (const auto i : make_range(Moose::dim))
        {
          std::string class_name = "ADVectorVelocityComponentAux";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<AuxVariableName>("variable") = var_names[i];
          params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
          params.set<std::vector<VariableName>>("arhoA") = {RHOA};
          params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
          params.set<MaterialPropertyName>("direction") = DIRECTION;
          params.set<unsigned int>("component") = i;
          params.set<ExecFlagEnum>("execute_on") = execute_on;
          _sim->addAuxKernel(class_name, genName(comp_name, i, "vel_vec"), params);
        }
      }
    }
    else
    {
      ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
      execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};

      // Velocity auxiliary kernel
      {
        std::string class_name = "QuotientAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = VELOCITY;
        params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
        params.set<std::vector<VariableName>>("numerator") = {RHOUA};
        params.set<std::vector<VariableName>>("denominator") = {RHOA};
        params.set<ExecFlagEnum>("execute_on") = execute_on;
        _sim->addAuxKernel(class_name, genName(comp_name, "vel"), params);
      }
    }

    ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
    ts_execute_on = EXEC_TIMESTEP_BEGIN;

    {
      // Computes rho = (rho*A)/A
      InputParameters params = _factory.getValidParams("QuotientAux");
      params.set<AuxVariableName>("variable") = DENSITY;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("numerator") = {RHOA};
      params.set<std::vector<VariableName>>("denominator") = {AREA};
      _sim->addAuxKernel("QuotientAux", genName(comp_name, "rho_auxkernel"), params);
    }

    {
      std::string class_name = "THMSpecificVolumeAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = SPECIFIC_VOLUME;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("A") = {AREA};
      // Less frequent than in FlowModelSinglePhase
      params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
      _sim->addAuxKernel(class_name, genName(comp_name, "v_aux"), params);
    }
    {
      std::string class_name = "THMSpecificInternalEnergyAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      _sim->addAuxKernel(class_name, genName(comp_name, "e_aux"), params);
    }

    {
      std::string class_name = "PressureAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = PRESSURE;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
      params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim->addAuxKernel(class_name, genName(comp_name, "pressure_uv_auxkernel"), params);
    }
    {
      std::string class_name = "TemperatureAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = TEMPERATURE;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
      params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim->addAuxKernel(class_name, genName(comp_name, "T_auxkernel"), params);
    }

    {
      std::string class_name = "SpecificTotalEnthalpyAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = SPECIFIC_TOTAL_ENTHALPY;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      params.set<std::vector<VariableName>>("p") = {PRESSURE};
      params.set<std::vector<VariableName>>("A") = {AREA};
      _sim->addAuxKernel(class_name, genName(comp_name, "H_auxkernel"), params);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addUserObjects()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<MooseEnum>("emit_on_nan") = "none";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  _sim->addUserObject(class_name, _numerical_flux_name, params);
}

void
THMVACESinglePhaseFlowPhysics::addInletBoundaries()
{
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _inlet_types[i];

    // Boundary fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    // boundary flux user object
    // we add them in addBCs for convenience
    if (boundary_type == InletTypeEnum::MdotTemperature)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostMassFlowRateTemperature";
      boundary_numerical_flux_name = comp_name + "_MdotTemperature";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("mass_flow_rate") = comp.getParam<Real>("m_dot");
      params.set<Real>("T") = comp.getParam<Real>("T");
      params.set<Real>("normal") = comp.getNormal();
      params.set<bool>("reversible") = comp.isReversible();
      params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
      params.set<UserObjectName>("fluid_properties") = _fp_name;
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, boundary_numerical_flux_name, params);
      comp.connectObject(params, boundary_numerical_flux_name, "m_dot", "mass_flow_rate");
      comp.connectObject(params, boundary_numerical_flux_name, "T");
    }

    // Boundary flux BC
    addBoundaryFluxBC(comp, boundary_numerical_flux_name);
  }
}

void
THMVACESinglePhaseFlowPhysics::addOutletBoundaries()
{
  for (const auto i : index_range(_outlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _outlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    UserObjectName boundary_numerical_flux_name = "invalid";
    const auto & boundary_type = _outlet_types[i];

    // Boundary fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    // boundary flux user object
    // we add them in addBCs for convenience
    if (boundary_type == OutletTypeEnum::FixedPressure)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostPressure";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("p") = comp.getParam<Real>("p");
      params.set<Real>("normal") = comp.getNormal();
      params.set<UserObjectName>("fluid_properties") = _fp_name;
      params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, boundary_numerical_flux_name, params);
      comp.connectObject(params, boundary_numerical_flux_name, "p");
    }

    // Boundary flux BC
    addBoundaryFluxBC(comp, boundary_numerical_flux_name);
  }
}

void
THMVACESinglePhaseFlowPhysics::addBoundaryFluxBC(
    const PhysicsFlowBoundary & comp, const UserObjectName & boundary_numerical_flux_name)
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = comp.getBoundaryNames();
  params.set<Real>("normal") = comp.getNormal();
  params.set<UserObjectName>("boundary_flux") = boundary_numerical_flux_name;
  params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
  params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();

  for (const auto & var : nonlinearVariableNames())
  {
    params.set<NonlinearVariableName>("variable") = var;
    _sim->addBoundaryCondition(class_name, genName(comp.name(), var, "bnd_flux_3eqn_bc"), params);
  }
}
