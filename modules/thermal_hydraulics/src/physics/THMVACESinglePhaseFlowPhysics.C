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
#include "SlopeReconstruction1DInterface.h"
#include "ClosuresBase.h"
#include "Numerics.h"

// For implementing component-specific behavior
#include "FlowChannelBase.h"
#include "PhysicsFlowBoundary.h"
#include "PhysicsFlowJunction.h"
#include "PhysicsVolumeJunction.h"
#include "PhysicsHeatTransferBase.h"

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
const std::string VACE1P::RHOV = "rhoV";
const std::string VACE1P::RHOEV = "rhoEV";
const std::string VACE1P::RHOUV = "rhouV";
const std::string VACE1P::RHOVV = "rhovV";
const std::string VACE1P::RHOWV = "rhowV";
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
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_bc");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_user_object");
registerMooseAction("ThermalHydraulicsApp", THMVACESinglePhaseFlowPhysics, "add_postprocessor");

InputParameters
THMVACESinglePhaseFlowPhysics::validParams()
{
  InputParameters params = ThermalHydraulicsFlowPhysics::validParams();

  params.addRequiredParam<MooseEnum>(
      "rdg_slope_reconstruction",
      SlopeReconstruction1DInterface<true>::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction type for rDG");
  params.addParam<std::vector<Real>>(
      "scaling_factor_1phase",
      {1, 1, 1},
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  // Note that these are single phase volume-integrated variables (as opposed to area-integrated)
  // They could be more general than volume junctions
  params.addParam<std::vector<Real>>(
      "scaling_factor_volume_junctions",
      {1, 1, 1, 1, 1},
      "Scaling factors for the volume junction variables (rhoU, rhouV, rhovV, rhowV, rhoEV)");

  params.addRequiredParam<bool>("output_vector_velocity",
                                "True if velocity is output as a vector field.");
  return params;
}

THMVACESinglePhaseFlowPhysics::THMVACESinglePhaseFlowPhysics(const InputParameters & params)
  : PhysicsBase(params),
    ThermalHydraulicsFlowPhysics(params),
    _rdg_slope_reconstruction(params.get<MooseEnum>("rdg_slope_reconstruction")),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factor_1phase")),
    _scaling_factors_volume_junctions(
        getParam<std::vector<Real>>("scaling_factor_volume_junctions")),
    _output_vector_velocity(params.get<bool>("output_vector_velocity"))
{
}

void
THMVACESinglePhaseFlowPhysics::initializePhysicsAdditional()
{
  // Force a 1D dimension. THM VACE implementations are only 1D
  if (dimension() > 1)
    mooseInfo("Lowering Physics dimension from '" + std::to_string(dimension()) +
              "' to 1, as the thermal hydraulics flow physics only support 1-dimensional problems");
  setDimension(1);

  ThermalHydraulicsFlowPhysics::initializePhysicsAdditional();
}

void
THMVACESinglePhaseFlowPhysics::actOnAdditionalTasks()
{
  // The THMProblem adds ICs on THM:add_variables, which happens before add_ic
  if (_current_task == "THMPhysics:add_ic")
    addTHMInitialConditions();
}

void
THMVACESinglePhaseFlowPhysics::addSolverVariables()
{
  ThermalHydraulicsFlowPhysics::addCommonVariables();

  // Flow channels
  for (const auto flow_channel : _flow_channels)
  {
    const std::vector<SubdomainName> & subdomains = flow_channel->getSubdomainNames();

    _sim->addSimVariable(true, RHOA, _fe_type, subdomains, _scaling_factors[0]);
    _sim->addSimVariable(true, RHOUA, _fe_type, subdomains, _scaling_factors[1]);
    _sim->addSimVariable(true, RHOEA, _fe_type, subdomains, _scaling_factors[2]);
  }

  saveSolverVariableName(RHOA);
  saveSolverVariableName(RHOUA);
  saveSolverVariableName(RHOEA);
  _derivative_vars = {RHOA, RHOUA, RHOEA};

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    const auto & junc_name = _junction_components[i];
    const auto & junc = _sim->getComponentByName<PhysicsFlowJunction>(junc_name);
    const auto & junction_type = _junction_types[i];
    if (junction_type == ThermalHydraulicsFlowPhysics::Volume ||
        junction_type == ThermalHydraulicsFlowPhysics::Pump ||
        junction_type == ThermalHydraulicsFlowPhysics::ParallelChannels)
    {
      const libMesh::FEType fe_type(CONSTANT, MONOMIAL);
      const auto & subdomains = junc.getSubdomainNames();

      _sim->addSimVariable(true, RHOV, fe_type, subdomains, _scaling_factors_volume_junctions[0]);
      _sim->addSimVariable(true, RHOUV, fe_type, subdomains, _scaling_factors_volume_junctions[1]);
      _sim->addSimVariable(true, RHOVV, fe_type, subdomains, _scaling_factors_volume_junctions[2]);
      _sim->addSimVariable(true, RHOWV, fe_type, subdomains, _scaling_factors_volume_junctions[3]);
      _sim->addSimVariable(true, RHOEV, fe_type, subdomains, _scaling_factors_volume_junctions[4]);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addAuxiliaryVariables()
{
  // Flow channels
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

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    const auto & junc_name = _junction_components[i];
    const auto & junc = _sim->getComponentByName<PhysicsFlowJunction>(junc_name);
    const auto & junction_type = _junction_types[i];
    if (junction_type == ThermalHydraulicsFlowPhysics::Volume ||
        junction_type == ThermalHydraulicsFlowPhysics::Pump ||
        junction_type == ThermalHydraulicsFlowPhysics::ParallelChannels)
    {
      const libMesh::FEType fe_type(CONSTANT, MONOMIAL);
      const auto & subdomains = junc.getSubdomainNames();

      _sim->addSimVariable(false, VACE1P::PRESSURE, fe_type, subdomains);
      _sim->addSimVariable(false, VACE1P::TEMPERATURE, fe_type, subdomains);
      _sim->addSimVariable(false, VACE1P::VELOCITY, fe_type, subdomains);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addTHMInitialConditions()
{
  ThermalHydraulicsFlowPhysics::addCommonInitialConditions();

  // Flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto & comp_name = flow_channel->name();

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
          params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
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
          params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
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
        params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
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

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    const auto & junc_name = _junction_components[i];
    const auto & junc = _sim->getComponentByName<PhysicsFlowJunction>(junc_name);
    const auto & junction_type = _junction_types[i];
    if (junction_type == ThermalHydraulicsFlowPhysics::Volume ||
        junction_type == ThermalHydraulicsFlowPhysics::Pump ||
        junction_type == ThermalHydraulicsFlowPhysics::ParallelChannels)
    {
      const libMesh::FEType fe_type(CONSTANT, MONOMIAL);
      const auto & subdomains = junc.getSubdomainNames();
      const auto & volume = junc.getParam<Real>("volume");
      Real initial_pressure, initial_temperature, initial_rho, initial_E;
      RealVectorValue initial_vel;
      _sim->getComponentByName<PhysicsVolumeJunction>(junc_name).getInitialConditions(
          initial_pressure, initial_temperature, initial_rho, initial_E, initial_vel);

      _sim->addConstantIC(VACE1P::RHOV, initial_rho * volume, subdomains);
      _sim->addConstantIC(VACE1P::RHOUV, initial_rho * initial_vel(0) * volume, subdomains);
      _sim->addConstantIC(VACE1P::RHOVV, initial_rho * initial_vel(1) * volume, subdomains);
      _sim->addConstantIC(VACE1P::RHOWV, initial_rho * initial_vel(2) * volume, subdomains);
      _sim->addConstantIC(VACE1P::RHOEV, initial_rho * initial_E * volume, subdomains);

      _sim->addConstantIC(VACE1P::PRESSURE, initial_pressure, subdomains);
      _sim->addConstantIC(VACE1P::TEMPERATURE, initial_temperature, subdomains);
      _sim->addConstantIC(VACE1P::VELOCITY, initial_vel.norm(), subdomains);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addMaterials()
{
  ThermalHydraulicsFlowPhysics::addCommonMaterials();

  // Flow channels
  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto & comp_name = flow_channel->name();
    {
      std::string class_name = "ADFluidProperties3EqnMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
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
      params.set<UserObjectName>("fp_1phase") = flow_channel->getFluidPropertiesName();
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
      params.set<UserObjectName>("fluid_properties") = flow_channel->getFluidPropertiesName();
      params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();
      _sim->addMaterial(class_name, genName(comp_name, "rdg_3egn_mat"), params);
    }
  }

  // Heat transfer components
  // Convert functor from heat flux boundaries to expected material properties
  for (const auto & [comp_name, heat_flux_type] : _heat_transfer_types)
  {
    const auto & comp = _sim->getComponentByName<PhysicsHeatTransferBase>(comp_name);

    if (heat_flux_type == ThermalHydraulicsFlowPhysics::FixedHeatFlux)
    {
      const std::string class_name = "MaterialFunctorConverter";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<std::vector<MooseFunctorName>>("functors_in") = {
          comp.getWallHeatFluxFunctorName()};
      params.set<std::vector<MaterialPropertyName>>("ad_props_out") = {comp.getWallHeatFluxName()};
      _sim->addMaterial(class_name, genName(name(), comp.name(), "q_wall_material"), params);
    }
  }

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    const auto & junc_name = _junction_components[i];
    const auto & junc = _sim->getComponentByName<PhysicsFlowJunction>(junc_name);
    const auto & junction_type = _junction_types[i];
    if (junction_type == ThermalHydraulicsFlowPhysics::Volume ||
        junction_type == ThermalHydraulicsFlowPhysics::Pump ||
        junction_type == ThermalHydraulicsFlowPhysics::ParallelChannels)
    {
      // An error message results if there is any block without a material, so
      // until this restriction is removed, we must add a dummy material that
      // computes no material properties.

      const std::string class_name = "GenericConstantMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = junc.getSubdomainNames();
      params.set<std::vector<std::string>>("prop_names") = {};
      params.set<std::vector<Real>>("prop_values") = {};
      _sim->addMaterial(class_name, genName(junc.name(), "dummy_mat"), params);
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addFEKernels()
{
  // Flow channels
  // TODO: unroll the loop and create one material for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto & comp_name = flow_channel->name();
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
    if (_gravity_vector.norm() > 0)
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

  addHeatTransferKernels();
  addFlowJunctionsKernels();
}

void
THMVACESinglePhaseFlowPhysics::addDGKernels()
{
  // TODO: unroll the loop and create one DG kernel for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto & comp_name = flow_channel->name();
    // mass
    const std::string class_name = "ADNumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<UserObjectName>("numerical_flux") =
        libmesh_map_find(_numerical_flux_names, flow_channel->getFluidPropertiesName());
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
  // Flow channels
  // TODO: unroll the loop and create one object for all flow channels
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const auto & comp_name = flow_channel->name();
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
      params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
      _sim->addAuxKernel(class_name, genName(comp_name, "pressure_uv_auxkernel"), params);
    }
    {
      std::string class_name = "TemperatureAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = TEMPERATURE;
      params.set<std::vector<SubdomainName>>("block") = flow_channel->getSubdomainNames();
      params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
      params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
      params.set<UserObjectName>("fp") = flow_channel->getFluidPropertiesName();
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

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];

    if (junction_type == OneToOne)
    {
    }
    else if (junction_type == Volume || junction_type == Pump || junction_type == ParallelChannels)
    {
      const std::vector<std::pair<std::string, VariableName>> quantities = {
          {"pressure", VACE1P::PRESSURE},
          {"temperature", VACE1P::TEMPERATURE},
          {"speed", VACE1P::VELOCITY}};
      for (const auto & quantity_and_name : quantities)
      {
        const std::string class_name = "VolumeJunction1PhaseAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = quantity_and_name.second;
        params.set<MooseEnum>("quantity") = quantity_and_name.first;
        params.set<Real>("volume") = comp.getParam<Real>("volume");
        params.set<std::vector<VariableName>>("rhoV") = {VACE1P::RHOV};
        params.set<std::vector<VariableName>>("rhouV") = {VACE1P::RHOUV};
        params.set<std::vector<VariableName>>("rhovV") = {VACE1P::RHOVV};
        params.set<std::vector<VariableName>>("rhowV") = {VACE1P::RHOWV};
        params.set<std::vector<VariableName>>("rhoEV") = {VACE1P::RHOEV};
        params.set<UserObjectName>("fp") = comp.getFluidPropertiesName();
        const std::string obj_name = genName(comp.name(), quantity_and_name.first + "_aux");
        params.set<std::vector<SubdomainName>>("block") = comp.getSubdomainNames();
        _sim->addAuxKernel(class_name, obj_name, params);
      }
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addUserObjects()
{
  // Flow channels
  // Keep track of which fluid properties we added a flux UO for
  for (const auto i : index_range(_flow_channels))
  {
    const auto flow_channel = _flow_channels[i];
    const std::string class_name = "ADNumericalFlux3EqnHLLC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = flow_channel->getFluidPropertiesName();
    params.set<MooseEnum>("emit_on_nan") = "none";
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    if (!_numerical_flux_names.count(flow_channel->getFluidPropertiesName()))
    {
      _numerical_flux_names[flow_channel->getFluidPropertiesName()] =
          genName(name(), "HLLC_3eqn_UO", flow_channel->getFluidPropertiesName());
      _sim->addUserObject(
          class_name,
          libmesh_map_find(_numerical_flux_names, flow_channel->getFluidPropertiesName()),
          params);
    }
  }

  // Junctions
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];
    const auto & boundary_names = comp.getBoundaryNames();
    const auto & normals = comp.getBoundaryNormals();

    // Junction fluxes should be updated as often as possible
    ExecFlagEnum junction_execute_on(MooseUtils::getDefaultExecFlagEnum());
    junction_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    // Add user object for computing and storing the fluxes
    if (junction_type == OneToOne)
    {
      const std::string class_name = "ADJunctionOneToOne1PhaseUserObject";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = boundary_names;
      params.set<std::vector<Real>>("normals") = normals;
      params.set<std::vector<processor_id_type>>("processor_ids") = comp.getProcIds();
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      // It is assumed that each channel should have the same numerical flux, so
      // just use the first one.
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {VACE1P::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {VACE1P::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {VACE1P::RHOEA};
      params.set<std::string>("junction_name") = comp_name;
      params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
      params.set<ExecFlagEnum>("execute_on") = junction_execute_on;
      _sim->addUserObject(class_name, comp.getJunctionUOName(), params);
    }
    else if (junction_type == Volume || junction_type == Pump || junction_type == ParallelChannels)
    {
      std::string class_name;
      if (junction_type == Volume)
        class_name = "ADVolumeJunction1PhaseUserObject";
      else if (junction_type == Pump)
        class_name = "ADPump1PhaseUserObject";
      else if (junction_type == ParallelChannels)
        class_name = "ADJunctionParallelChannels1PhaseUserObject";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<bool>("use_scalar_variables") = false;
      mooseAssert(comp.getSubdomainNames().size() == 1, "Should have 1 subdomain on that junction");
      params.set<subdomain_id_type>("junction_subdomain_id") =
          MooseMeshUtils::getSubdomainID(comp.getSubdomainNames()[0], _mesh->getMesh());
      params.set<std::vector<BoundaryName>>("boundary") = boundary_names;
      params.set<std::vector<Real>>("normals") = normals;
      params.set<std::vector<processor_id_type>>("processor_ids") = comp.getConnectedProcessorIDs();
      params.set<std::vector<UserObjectName>>("numerical_flux_names") = std::vector<UserObjectName>(
          boundary_names.size(),
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName()));
      params.set<Real>("volume") = comp.getParam<Real>("volume");
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("rhoA") = {VACE1P::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {VACE1P::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {VACE1P::RHOEA};
      params.set<std::vector<VariableName>>("rhoV") = {VACE1P::RHOV};
      params.set<std::vector<VariableName>>("rhouV") = {VACE1P::RHOUV};
      params.set<std::vector<VariableName>>("rhovV") = {VACE1P::RHOVV};
      params.set<std::vector<VariableName>>("rhowV") = {VACE1P::RHOWV};
      params.set<std::vector<VariableName>>("rhoEV") = {VACE1P::RHOEV};
      params.set<Real>("K") = comp.getParam<Real>("K");
      params.set<Real>("A_ref") = comp.isParamValid("A_ref") ? comp.getParam<Real>("A_ref") : 0;
      params.set<UserObjectName>("fp") = comp.getFluidPropertiesName();
      params.set<ExecFlagEnum>("execute_on") = junction_execute_on;
      if (junction_type == Pump)
      {
        params.set<Real>("head") = comp.getParam<Real>("head");
        params.set<Real>("gravity_magnitude") = THM::gravity_const;
      }
      else if (junction_type == ParallelChannels)
      {
        params.set<std::string>("component_name") = comp.name();
        params.set<RealVectorValue>("dir_c0") = comp.getConnectedComponentDirections()[0];
      }
      _sim->addUserObject(class_name, comp.getJunctionUOName(), params);
      comp.connectObject(params, comp.getJunctionUOName(), "K");
      if (junction_type == Pump)
        comp.connectObject(params, comp.getJunctionUOName(), "head");
    }
  }
}

void
THMVACESinglePhaseFlowPhysics::addFEBCs()
{
  addInletBoundaries();
  addOutletBoundaries();
  addFlowJunctionBCs();
}

void
THMVACESinglePhaseFlowPhysics::addInletBoundaries()
{
  for (const auto i : index_range(_inlet_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _inlet_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowBoundary>(comp_name);
    const auto & boundary_type = _inlet_types[i];

    // Boundary fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    // boundary flux user object
    // we add them in addBCs for convenience
    if (boundary_type == InletTypeEnum::MdotTemperature)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostMassFlowRateTemperature";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("mass_flow_rate") = comp.getParam<Real>("m_dot");
      params.set<Real>("T") = comp.getParam<Real>("T");
      params.set<Real>("normal") = comp.getNormal();
      params.set<bool>("reversible") = comp.isReversible();
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
      comp.connectObject(params, comp.getBoundaryUOName(), "m_dot", "mass_flow_rate");
      comp.connectObject(params, comp.getBoundaryUOName(), "T");
    }
    else if (boundary_type == InletTypeEnum::VelocityTemperature)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostVelocityTemperature";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("vel") = comp.getParam<Real>("vel");
      params.set<Real>("T") = comp.getParam<Real>("T");
      params.set<Real>("normal") = comp.getNormal();
      params.set<bool>("reversible") = comp.isReversible();
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
      comp.connectObject(params, comp.getBoundaryUOName(), "vel");
      comp.connectObject(params, comp.getBoundaryUOName(), "T");
    }
    else if (boundary_type == InletTypeEnum::StagnationPressureTemperature)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostStagnationPressureTemperature";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("p0") = comp.getParam<Real>("p0");
      params.set<Real>("T0") = comp.getParam<Real>("T0");
      params.set<Real>("normal") = comp.getNormal();
      params.set<bool>("reversible") = comp.isReversible();
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
      comp.connectObject(params, comp.getBoundaryUOName(), "p0");
      comp.connectObject(params, comp.getBoundaryUOName(), "T0");
    }
    else
      mooseError("Unsupported inlet boundary type ", boundary_type);

    // Boundary flux BC
    addBoundaryFluxBC(comp, comp.getBoundaryUOName());
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
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
      comp.connectObject(params, comp.getBoundaryUOName(), "p");
    }
    else if (boundary_type == OutletTypeEnum::FreeBoundary)
    {
      const std::string class_name = "ADBoundaryFlux3EqnFreeOutflow";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("fluid_properties") = comp.getFluidPropertiesName();
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
    }
    else if (boundary_type == OutletTypeEnum::SolidWall)
    {
      const std::string class_name = "ADBoundaryFlux3EqnGhostWall";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("numerical_flux") =
          libmesh_map_find(_numerical_flux_names, comp.getFluidPropertiesName());
      params.set<Real>("normal") = comp.getNormal();
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim->addUserObject(class_name, comp.getBoundaryUOName(), params);
    }
    else
      mooseError("Unimplemented outlet boundary type", boundary_type);

    // Boundary flux BC
    addBoundaryFluxBC(comp, comp.getBoundaryUOName());
  }
}

void
THMVACESinglePhaseFlowPhysics::addBoundaryFluxBC(const PhysicsFlowBoundary & comp,
                                                 const UserObjectName & boundary_flux_uo_name)
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = comp.getBoundaryNames();
  params.set<Real>("normal") = comp.getNormal();
  params.set<UserObjectName>("boundary_flux") = boundary_flux_uo_name;
  params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
  params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();

  for (const auto & var : solverVariableNames())
  {
    params.set<NonlinearVariableName>("variable") = var;
    _sim->addBoundaryCondition(class_name, genName(comp.name(), var, "bnd_flux_3eqn_bc"), params);
  }
}

void
THMVACESinglePhaseFlowPhysics::addFlowJunctionBCs()
{
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];
    const auto & boundary_names = comp.getBoundaryNames();
    const auto & normals = comp.getBoundaryNormals();

    // Junction fluxes should be updated as often as possible
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    // Add BC to each of the connected flow channels
    const auto var_names = solverVariableNames();
    for (std::size_t i = 0; i < boundary_names.size(); i++)
      for (std::size_t j = 0; j < var_names.size(); j++)
      {
        std::string class_name;
        if (junction_type == OneToOne)
          class_name = "ADJunctionOneToOne1PhaseBC";
        else if (junction_type == Volume || junction_type == Pump ||
                 junction_type == ParallelChannels)
          class_name = "ADVolumeJunction1PhaseBC";
        else
          mooseAssert(false, "Not implemented");
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_names[i]};
        params.set<Real>("normal") = normals[i];
        params.set<NonlinearVariableName>("variable") = var_names[j];
        if (junction_type == OneToOne)
          params.set<UserObjectName>("junction_uo") = comp.getJunctionUOName();
        else if (junction_type == Volume || junction_type == Pump ||
                 junction_type == ParallelChannels)
        {
          params.set<UserObjectName>("volume_junction_uo") = comp.getJunctionUOName();
          params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
          params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
        }
        params.set<unsigned int>("connection_index") = i;
        params.set<std::vector<VariableName>>("rhoA") = {VACE1P::RHOA};
        params.set<std::vector<VariableName>>("rhouA") = {VACE1P::RHOUA};
        params.set<std::vector<VariableName>>("rhoEA") = {VACE1P::RHOEA};
        params.set<bool>("implicit") = _sim->getImplicitTimeIntegrationFlag();
        // TODO check real naming convention
        _sim->addBoundaryCondition(
            class_name,
            genName(name() + "_" + boundary_names[i], i, var_names[j] + "_" + class_name),
            params);
      }
  }
}

void
THMVACESinglePhaseFlowPhysics::addWallHeatFlux(const std::string & heat_transfer_component,
                                               const HeatFluxWallEnum & heat_flux_type)
{
  _heat_transfer_types[heat_transfer_component] = heat_flux_type;
}

void
THMVACESinglePhaseFlowPhysics::addHeatTransferKernels()
{
  for (const auto & [comp_name, heat_flux_type] : _heat_transfer_types)
  {
    const auto & comp = _sim->getComponentByName<PhysicsHeatTransferBase>(comp_name);

    if (heat_flux_type == ThermalHydraulicsFlowPhysics::FixedWallTemperature)
    {
      const std::string class_name = "ADOneDEnergyWallHeating";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = VACE1P::RHOEA;
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<std::vector<VariableName>>("T_wall") = {comp.getWallTemperatureName()};
      params.set<MaterialPropertyName>("Hw") = comp.getWallHeatTransferCoefficientName();
      params.set<std::vector<VariableName>>("P_hf") = {comp.getHeatedPerimeterName()};
      params.set<MaterialPropertyName>("T") = VACE1P::TEMPERATURE;
      _sim->addKernel(class_name, genName(comp_name, "wall_heat_transfer"), params);
    }
    else if (heat_flux_type == ThermalHydraulicsFlowPhysics::FixedHeatFlux)
    {
      const std::string class_name = "ADOneDEnergyWallHeatFlux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = VACE1P::RHOEA;
      params.set<std::vector<SubdomainName>>("block") = comp.getFlowChannelSubdomains();
      params.set<MaterialPropertyName>("q_wall") = comp.getWallHeatFluxName();
      params.set<std::vector<VariableName>>("P_hf") = {comp.getHeatedPerimeterName()};
      _sim->addKernel(class_name, genName(comp_name, "wall_heat"), params);
    }
    else
      mooseAssert(false, "Heat flux type not implemented");
  }
}

void
THMVACESinglePhaseFlowPhysics::addFlowJunctionsKernels()
{
  for (const auto i : index_range(_junction_components))
  {
    // Get component, which has reference to the controllable parameters
    const auto & comp_name = _junction_components[i];
    const auto & comp = _sim->getComponentByName<PhysicsFlowJunction>(comp_name);
    const auto & junction_type = _junction_types[i];

    if (junction_type == OneToOne)
    {
    }
    else if (junction_type == Volume || junction_type == Pump || junction_type == ParallelChannels)
    {
      // Add scalar kernels for the junction
      std::vector<NonlinearVariableName> var_names = {RHOV, RHOUV, RHOVV, RHOWV, RHOEV};
      for (std::size_t i = 0; i < 5; i++)
      {
        {
          const std::string class_name = "ADTimeDerivative";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<NonlinearVariableName>("variable") = var_names[i];
          const std::string obj_name = genName(comp.name(), var_names[i], "td");
          params.set<std::vector<SubdomainName>>("block") = comp.getSubdomainNames();
          _sim->addKernel(class_name, obj_name, params);
        }
        {
          const std::string class_name = "ADVolumeJunctionAdvectionKernel";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<NonlinearVariableName>("variable") = var_names[i];
          params.set<UserObjectName>("volume_junction_uo") = comp.getJunctionUOName();
          params.set<unsigned int>("equation_index") = i;
          const std::string obj_name = genName(comp.name(), var_names[i], "vja_sk");
          params.set<std::vector<SubdomainName>>("block") = comp.getSubdomainNames();
          _sim->addKernel(class_name, obj_name, params);
        }
      }
    }
    else
      mooseError("Unsupported junction type ", junction_type);
  }
}

void
THMVACESinglePhaseFlowPhysics::checkIntegrity() const
{
  // Check that closures on components are creating the materials we need
  // Not all components have closures: check every type supported
  for (const auto flow_channel : _flow_channels)
  {
    const auto & closures = flow_channel->getClosures();
    if (!closures->createsRegularMaterials())
      closures->paramError("add_regular_materials",
                           "Should add regular materials for this closure to work with Physics '" +
                               name() + "'");
  }
}