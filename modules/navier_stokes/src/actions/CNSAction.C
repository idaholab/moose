//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "CNSAction.h"

#include "NS.h"
#include "AddVariableAction.h"
#include "MooseObject.h"

// MOOSE includes
#include "FEProblem.h"

#include "libmesh/fe.h"
#include "libmesh/vector_value.h"
#include "libmesh/string_to_enum.h"

using namespace libMesh;

registerMooseAction("NavierStokesApp", CNSAction, "add_navier_stokes_variables");
registerMooseAction("NavierStokesApp", CNSAction, "add_navier_stokes_kernels");
registerMooseAction("NavierStokesApp", CNSAction, "add_navier_stokes_bcs");
registerMooseAction("NavierStokesApp", CNSAction, "add_navier_stokes_ics");

InputParameters
CNSAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("This class allows us to have a section of the input file like the "
                             "following which automatically adds Kernels and AuxKernels for all "
                             "the required nonlinear and auxiliary variables.");

  MooseEnum type("steady-state transient", "steady-state");
  params.addParam<MooseEnum>("equation_type", type, "Navier-Stokes equation type");

  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "The list of block ids (SubdomainID) on which NS equation is defined on");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  params.addParam<std::vector<BoundaryName>>(
      "stagnation_boundary", std::vector<BoundaryName>(), "Stagnation boundaries");
  params.addParam<std::vector<Real>>(
      "stagnation_pressure", std::vector<Real>(), "Pressure on stagnation boundaries");
  params.addParam<std::vector<Real>>(
      "stagnation_temperature", std::vector<Real>(), "Temperature on stagnation boundaries");
  params.addParam<std::vector<Real>>(
      "stagnation_flow_direction", std::vector<Real>(), "Flow directions on stagnation boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "no_penetration_boundary", std::vector<BoundaryName>(), "No-penetration boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "static_pressure_boundary", std::vector<BoundaryName>(), "Static pressure boundaries");
  params.addParam<std::vector<Real>>(
      "static_pressure", std::vector<Real>(), "Static pressure on boundaries");

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies(), "LAGRANGE");
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders(), "FIRST");
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<Real>("density_scaling", 1, "Scaling for the density variable");
  params.addParam<RealVectorValue>(
      "momentum_scaling", RealVectorValue(1, 1, 1), "Scaling for the momentum variables");
  params.addParam<Real>("total_energy_density_scaling", 1, "Scaling for the total-energy variable");

  params.addRequiredParam<Real>("initial_pressure",
                                "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature",
                                "The initial temperature, assumed constant everywhere");
  params.addRequiredParam<RealVectorValue>("initial_velocity",
                                           "The initial velocity, assumed constant everywhere");

  params.addParamNamesToGroup("equation_type block fluid_properties", "Base");
  params.addParamNamesToGroup(
      "stagnation_boundary stagnation_pressure stagnation_temperature "
      "stagnation_flow_direction no_penetration_boundary static_pressure_boundary static_pressure",
      "BoundaryCondition");
  params.addParamNamesToGroup(
      "family order density_scaling momentum_scaling total_energy_density_scaling", "Variable");
  params.addParam<std::string>("pressure_variable_name",
                               "A name for the pressure variable. If this is not provided, a "
                               "sensible default will be used.");
  return params;
}

CNSAction::CNSAction(const InputParameters & parameters)
  : Action(parameters),
    _type(getParam<MooseEnum>("equation_type")),
    _fp_name(getParam<UserObjectName>("fluid_properties")),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _stagnation_boundary(getParam<std::vector<BoundaryName>>("stagnation_boundary")),
    _stagnation_pressure(getParam<std::vector<Real>>("stagnation_pressure")),
    _stagnation_temperature(getParam<std::vector<Real>>("stagnation_temperature")),
    _stagnation_direction(getParam<std::vector<Real>>("stagnation_flow_direction")),
    _no_penetration_boundary(getParam<std::vector<BoundaryName>>("no_penetration_boundary")),
    _static_pressure_boundary(getParam<std::vector<BoundaryName>>("static_pressure_boundary")),
    _static_pressure(getParam<std::vector<Real>>("static_pressure")),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _initial_velocity(getParam<RealVectorValue>("initial_velocity")),
    _pressure_variable_name(isParamValid("pressure_variable_name")
                                ? getParam<std::string>("pressure_variable_name")
                                : "p")
{
  if (_stagnation_pressure.size() != _stagnation_boundary.size())
    paramError("stagnation_pressure",
               "Size is not the same as the number of boundaries in 'stagnation_boundary'");
  if (_stagnation_temperature.size() != _stagnation_boundary.size())
    paramError("stagnation_temperature",
               "Size is not the same as the number of boundaries in 'stagnation_boundary'");
  if (_static_pressure.size() != _static_pressure_boundary.size())
    paramError("static_pressure",
               "Size is not the same as the number of boundaries in 'static_pressure_boundary'");
}

void
CNSAction::act()
{
  if (_current_task == "add_navier_stokes_variables")
  {
    _dim = _mesh->dimension();
    for (const auto & subdomain_name : _blocks)
    {
      SubdomainID id = _mesh->getSubdomainID(subdomain_name);
      _block_ids.insert(id);
    }
    if (_stagnation_direction.size() != _stagnation_boundary.size() * _dim)
      paramError("stagnation_flow_direction",
                 "Size is not the same as the number of boundaries in 'stagnation_boundary' times "
                 "the mesh dimension");

    // FIXME: need to check boundaries are non-overlapping and enclose the blocks

    auto var_type = AddVariableAction::variableType(_fe_type);
    auto base_params = _factory.getValidParams(var_type);
    base_params.set<MooseEnum>("order") = _fe_type.order.get_order();
    base_params.set<MooseEnum>("family") = Moose::stringify(_fe_type.family);
    if (_block_ids.size() != 0)
      for (const SubdomainID & id : _block_ids)
        base_params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

    // add primal variables
    InputParameters params(base_params);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("density_scaling")};
    _problem->addVariable(var_type, NS::density, params);

    auto mscaling = getParam<RealVectorValue>("momentum_scaling");
    params.set<std::vector<Real>>("scaling") = {mscaling(0)};
    _problem->addVariable(var_type, NS::momentum_x, params);
    if (_dim >= 2)
    {
      params.set<std::vector<Real>>("scaling") = {mscaling(1)};
      _problem->addVariable(var_type, NS::momentum_y, params);
    }
    if (_dim >= 3)
    {
      params.set<std::vector<Real>>("scaling") = {mscaling(2)};
      _problem->addVariable(var_type, NS::momentum_z, params);
    }
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("total_energy_density_scaling")};
    _problem->addVariable(var_type, NS::total_energy_density, params);

    // Add Aux variables.  These are all required in order for the code
    // to run, so they should not be independently selectable by the
    // user.
    _problem->addAuxVariable(var_type, NS::velocity_x, base_params);
    if (_dim >= 2)
      _problem->addAuxVariable(var_type, NS::velocity_y, base_params);
    if (_dim >= 3)
      _problem->addAuxVariable(var_type, NS::velocity_z, base_params);
    _problem->addAuxVariable(var_type, _pressure_variable_name, base_params);
    _problem->addAuxVariable(var_type, NS::temperature, base_params);
    _problem->addAuxVariable(var_type, NS::specific_total_enthalpy, base_params);
    _problem->addAuxVariable(var_type, NS::mach_number, base_params);

    // Needed for FluidProperties calculations
    _problem->addAuxVariable(var_type, NS::specific_internal_energy, base_params);
    _problem->addAuxVariable(var_type, NS::specific_volume, base_params);
  }

  if (_current_task == "add_navier_stokes_kernels")
  {
    if (_type == "transient")
      addNSTimeKernels();

    // Add all the inviscid flux Kernels.
    addNSMassInviscidFlux();
    addNSEnergyInviscidFlux();
    for (unsigned int component = 0; component < _dim; ++component)
      addNSMomentumInviscidFlux(component);

    // Add SUPG Kernels
    addNSSUPGMass();
    addNSSUPGEnergy();
    for (unsigned int component = 0; component < _dim; ++component)
      addNSSUPGMomentum(component);

    // Add AuxKernels.
    addPressureOrTemperatureAux("PressureAux");
    addPressureOrTemperatureAux("TemperatureAux");
    addSpecificTotalEnthalpyAux();
    addNSMachAux();
    addNSInternalEnergyAux();
    addSpecificVolumeComputation();
    for (unsigned int component = 0; component < _dim; ++component)
      addNSVelocityAux(component);
  }

  if (_current_task == "add_navier_stokes_bcs")
  {
    if (_stagnation_boundary.size() > 0)
    {
      addNSMassWeakStagnationBC();
      addNSEnergyWeakStagnationBC();
      for (unsigned int component = 0; component < _dim; ++component)
        addNSMomentumWeakStagnationBC(component);
    }

    if (_no_penetration_boundary.size() > 0)
    {
      for (unsigned int component = 0; component < _dim; ++component)
        addNoPenetrationBC(component);
    }

    if (_static_pressure_boundary.size() > 0)
    {
      addNSMassUnspecifiedNormalFlowBC();
      addNSEnergyInviscidSpecifiedPressureBC();
      for (unsigned int component = 0; component < _dim; ++component)
        addNSMomentumInviscidSpecifiedPressureBC(component);
    }
  }

  if (_current_task == "add_navier_stokes_ics")
  {
    // add ICs for primal variables
    std::vector<VariableName> vars;
    vars.push_back(NS::density);
    vars.push_back(NS::momentum_x);
    if (_dim >= 2)
      vars.push_back(NS::momentum_y);
    if (_dim >= 3)
      vars.push_back(NS::momentum_z);
    vars.push_back(NS::total_energy_density);
    for (const auto & name : vars)
    {
      InputParameters params = _factory.getValidParams("NSInitialCondition");
      params.set<VariableName>("variable") = name;
      params.set<Real>("initial_pressure") = _initial_pressure;
      params.set<Real>("initial_temperature") = _initial_temperature;
      params.set<RealVectorValue>("initial_velocity") = _initial_velocity;
      params.set<UserObjectName>("fluid_properties") = _fp_name;
      _problem->addInitialCondition("NSInitialCondition", name + std::string("_ic"), params);
    }

    // add ICs for aux variables (possibly we do not need this)
    std::vector<VariableName> auxs;
    auxs.push_back(NS::velocity_x);
    if (_dim >= 2)
      auxs.push_back(NS::velocity_y);
    if (_dim >= 3)
      auxs.push_back(NS::velocity_z);

    auxs.push_back(_pressure_variable_name);
    auxs.push_back(NS::temperature);
    auxs.push_back(NS::specific_total_enthalpy);
    auxs.push_back(NS::mach_number);

    // Needed for FluidProperties calculations
    auxs.push_back(NS::specific_internal_energy);
    auxs.push_back(NS::specific_volume);
    for (const auto & name : auxs)
    {
      InputParameters params = _factory.getValidParams("NSInitialCondition");
      params.set<VariableName>("variable") = name;
      params.set<Real>("initial_pressure") = _initial_pressure;
      params.set<Real>("initial_temperature") = _initial_temperature;
      params.set<RealVectorValue>("initial_velocity") = _initial_velocity;
      params.set<UserObjectName>("fluid_properties") = _fp_name;
      if (name == _pressure_variable_name)
        params.set<MooseEnum>("variable_type") = NS::pressure;
      _problem->addInitialCondition("NSInitialCondition", name + std::string("_ic"), params);
    }
  }
}

void
CNSAction::addNSTimeKernels()
{
  const std::string kernel_type = "TimeDerivative";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<std::vector<SubdomainName>>("block") = _blocks;

  params.set<NonlinearVariableName>("variable") = NS::density;
  _problem->addKernel(kernel_type, NS::density + "_time_deriv", params);

  params.set<NonlinearVariableName>("variable") = NS::momentum_x;
  _problem->addKernel(kernel_type, NS::momentum_x + "_time_deriv", params);
  if (_dim >= 2)
  {
    params.set<NonlinearVariableName>("variable") = NS::momentum_y;
    _problem->addKernel(kernel_type, NS::momentum_y + "_time_deriv", params);
  }
  if (_dim >= 3)
  {
    params.set<NonlinearVariableName>("variable") = NS::momentum_z;
    _problem->addKernel(kernel_type, NS::momentum_z + "_time_deriv", params);
  }

  params.set<NonlinearVariableName>("variable") = NS::total_energy_density;
  _problem->addKernel(kernel_type, NS::total_energy_density + "_time_deriv", params);
}

void
CNSAction::addNSSUPGMass()
{
  const std::string kernel_type = "NSSUPGMass";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setKernelCommonParams(params);

  // SUPG Kernels also need temperature and specific_total_enthalpy currently.
  params.set<CoupledName>(NS::temperature) = {NS::temperature};
  params.set<CoupledName>(NS::specific_total_enthalpy) = {NS::specific_total_enthalpy};

  _problem->addKernel(kernel_type, "rho_supg", params);
}

void
CNSAction::addNSSUPGMomentum(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};

  const std::string kernel_type = "NSSUPGMomentum";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setKernelCommonParams(params);

  // SUPG Kernels also need temperature and specific_total_enthalpy currently.
  params.set<CoupledName>(NS::temperature) = {NS::temperature};
  params.set<CoupledName>(NS::specific_total_enthalpy) = {NS::specific_total_enthalpy};

  // Momentum Kernels also need the component.
  params.set<unsigned int>("component") = component;

  _problem->addKernel(kernel_type, momentums[component] + std::string("_supg"), params);
}

void
CNSAction::addNSSUPGEnergy()
{
  const std::string kernel_type = "NSSUPGEnergy";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy_density;
  setKernelCommonParams(params);

  // SUPG Kernels also need temperature and specific_total_enthalpy currently.
  params.set<CoupledName>(NS::temperature) = {NS::temperature};
  params.set<CoupledName>(NS::specific_total_enthalpy) = {NS::specific_total_enthalpy};

  _problem->addKernel(kernel_type, "rhoE_supg", params);
}

void
CNSAction::addSpecificVolumeComputation()
{
  const std::string kernel_type = "ParsedAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = NS::specific_volume;

  // arguments
  params.set<CoupledName>("args") = {NS::density};

  // expression
  std::string function = "if(" + NS::density + " = 0, 1e10, 1 / " + NS::density + ")";
  params.set<std::string>("function") = function;

  _problem->addAuxKernel(kernel_type, "specific_volume_auxkernel", params);
}

void
CNSAction::addNSInternalEnergyAux()
{
  const std::string kernel_type = "NSInternalEnergyAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = NS::specific_internal_energy;

  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>(NS::total_energy_density) = {NS::total_energy_density};

  // Couple the appropriate number of velocities
  coupleVelocities(params);

  _problem->addAuxKernel(kernel_type, "specific_internal_energy_auxkernel", params);
}

void
CNSAction::addNSMachAux()
{
  const std::string kernel_type = "NSMachAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = NS::mach_number;

  // coupled variables
  params.set<CoupledName>(NS::specific_internal_energy) = {NS::specific_internal_energy};
  params.set<CoupledName>(NS::specific_volume) = {NS::specific_volume};

  // Couple the appropriate number of velocities
  coupleVelocities(params);

  params.set<UserObjectName>("fluid_properties") = _fp_name;

  _problem->addAuxKernel(kernel_type, "mach_auxkernel", params);
}

void
CNSAction::addSpecificTotalEnthalpyAux()
{
  const std::string kernel_type = "NSSpecificTotalEnthalpyAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = NS::specific_total_enthalpy;

  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>(NS::total_energy_density) = {NS::total_energy_density};
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};

  _problem->addAuxKernel(kernel_type, "specific_total_enthalpy_auxkernel", params);
}

void
CNSAction::addNSVelocityAux(unsigned int component)
{
  const std::string kernel_type = "NSVelocityAux";
  const static std::string velocities[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = velocities[component];

  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>("momentum") = {momentums[component]};
  params.set<UserObjectName>("fluid_properties") = _fp_name;

  _problem->addAuxKernel(kernel_type, velocities[component] + "_auxkernel", params);
}

void
CNSAction::addPressureOrTemperatureAux(const std::string & kernel_type)
{
  InputParameters params = _factory.getValidParams(kernel_type);
  std::string var_name = (kernel_type == "PressureAux" ? _pressure_variable_name : NS::temperature);
  params.set<AuxVariableName>("variable") = var_name;

  // coupled variables
  params.set<CoupledName>("e") = {NS::specific_internal_energy};
  params.set<CoupledName>("v") = {NS::specific_volume};
  params.set<UserObjectName>("fp") = _fp_name;

  _problem->addAuxKernel(kernel_type, var_name + "_auxkernel", params);
}

void
CNSAction::addNSMassInviscidFlux()
{
  const std::string kernel_type = "NSMassInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setKernelCommonParams(params);
  _problem->addKernel(kernel_type, "rho_if", params);
}

void
CNSAction::addNSMomentumInviscidFlux(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};
  const std::string kernel_type = "NSMomentumInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setKernelCommonParams(params);

  // Extra stuff needed by momentum Kernels
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
  params.set<unsigned int>("component") = component;

  // Add the Kernel
  _problem->addKernel(kernel_type, momentums[component] + std::string("if"), params);
}

void
CNSAction::addNSEnergyInviscidFlux()
{
  const std::string kernel_type = "NSEnergyInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy_density;
  setKernelCommonParams(params);

  // Extra stuff needed by energy equation
  params.set<CoupledName>(NS::specific_total_enthalpy) = {NS::specific_total_enthalpy};

  // Add the Kernel
  _problem->addKernel(kernel_type, "rhoE_if", params);
}

void
CNSAction::addNSMassWeakStagnationBC()
{
  const std::string kernel_type = "NSMassWeakStagnationBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setBCCommonParams(params);

  for (unsigned int i = 0; i < _stagnation_boundary.size(); ++i)
  {
    setStagnationBCCommonParams(params, i);
    _problem->addBoundaryCondition(
        kernel_type, "weak_stagnation_mass_inflow_" + Moose::stringify(i), params);
  }
}

void
CNSAction::addNSEnergyWeakStagnationBC()
{
  const std::string kernel_type = "NSEnergyWeakStagnationBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy_density;
  setBCCommonParams(params);
  for (unsigned int i = 0; i < _stagnation_boundary.size(); ++i)
  {
    setStagnationBCCommonParams(params, i);
    _problem->addBoundaryCondition(
        kernel_type, "weak_stagnation_energy_inflow_" + Moose::stringify(i), params);
  }
}

void
CNSAction::addNSMomentumWeakStagnationBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};

  // Convective part
  {
    const std::string kernel_type = "NSMomentumConvectiveWeakStagnationBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = momentums[component];
    setBCCommonParams(params);
    // Momentum BCs also need the component.
    params.set<unsigned int>("component") = component;
    for (unsigned int i = 0; i < _stagnation_boundary.size(); ++i)
    {
      setStagnationBCCommonParams(params, i);
      _problem->addBoundaryCondition(kernel_type,
                                     std::string("weak_stagnation_") + momentums[component] +
                                         std::string("_convective_inflow_") + Moose::stringify(i),
                                     params);
    }
  }

  // Pressure part
  {
    const std::string kernel_type = "NSMomentumPressureWeakStagnationBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = momentums[component];
    setBCCommonParams(params);
    // Momentum BCs also need the component.
    params.set<unsigned int>("component") = component;

    for (unsigned int i = 0; i < _stagnation_boundary.size(); ++i)
    {
      setStagnationBCCommonParams(params, i);

      _problem->addBoundaryCondition(kernel_type,
                                     std::string("weak_stagnation_") + momentums[component] +
                                         std::string("_pressure_inflow_") + Moose::stringify(i),
                                     params);
    }
  }
}

void
CNSAction::addNoPenetrationBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};
  const std::string kernel_type = "NSPressureNeumannBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setBCCommonParams(params);

  // These BCs also need the component and couping to the pressure.
  params.set<unsigned int>("component") = component;
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};

  params.set<std::vector<BoundaryName>>("boundary") = _no_penetration_boundary;
  _problem->addBoundaryCondition(
      kernel_type, momentums[component] + std::string("_no_penetration"), params);
}

void
CNSAction::addNSMassUnspecifiedNormalFlowBC()
{
  const std::string kernel_type = "NSMassUnspecifiedNormalFlowBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setBCCommonParams(params);
  for (unsigned int i = 0; i < _static_pressure_boundary.size(); ++i)
  {
    params.set<std::vector<BoundaryName>>("boundary") = {_static_pressure_boundary[i]};
    params.set<Real>("specified_pressure") = _static_pressure[i];
    _problem->addBoundaryCondition(kernel_type, "mass_outflow_" + Moose::stringify(i), params);
  }
}

void
CNSAction::addNSMomentumInviscidSpecifiedPressureBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};
  const std::string kernel_type = "NSMomentumInviscidSpecifiedPressureBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setBCCommonParams(params);

  // These BCs also need the component.
  params.set<unsigned int>("component") = component;

  for (unsigned int i = 0; i < _static_pressure_boundary.size(); ++i)
  {
    params.set<std::vector<BoundaryName>>("boundary") = {_static_pressure_boundary[i]};
    params.set<Real>("specified_pressure") = _static_pressure[i];
    _problem->addBoundaryCondition(
        kernel_type,
        momentums[component] + std::string("_specified_pressure_outflow_") + Moose::stringify(i),
        params);
  }
}

void
CNSAction::addNSEnergyInviscidSpecifiedPressureBC()
{
  const std::string kernel_type = "NSEnergyInviscidSpecifiedPressureBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy_density;
  setBCCommonParams(params);
  // This BC also requires the current value of the temperature.
  params.set<CoupledName>(NS::temperature) = {NS::temperature};
  for (unsigned int i = 0; i < _static_pressure_boundary.size(); ++i)
  {
    params.set<std::vector<BoundaryName>>("boundary") = {_static_pressure_boundary[i]};
    params.set<Real>("specified_pressure") = _static_pressure[i];
    _problem->addBoundaryCondition(
        kernel_type, "rhoE_specified_pressure_outflow_" + Moose::stringify(i), params);
  }
}

void
CNSAction::setKernelCommonParams(InputParameters & params)
{
  params.set<std::vector<SubdomainName>>("block") = _blocks;

  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>(NS::total_energy_density) = {NS::total_energy_density};

  // Couple the appropriate number of velocities
  coupleVelocities(params);
  coupleMomentums(params);

  // FluidProperties object
  params.set<UserObjectName>("fluid_properties") = _fp_name;
}

void
CNSAction::setBCCommonParams(InputParameters & params)
{
  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>(NS::total_energy_density) = {NS::total_energy_density};

  // Couple the appropriate number of velocities
  coupleVelocities(params);
  coupleMomentums(params);

  // FluidProperties object
  params.set<UserObjectName>("fluid_properties") = _fp_name;
}

void
CNSAction::setStagnationBCCommonParams(InputParameters & params, unsigned int i)
{
  params.set<std::vector<BoundaryName>>("boundary") = {_stagnation_boundary[i]};
  params.set<Real>("stagnation_pressure") = _stagnation_pressure[i];
  params.set<Real>("stagnation_temperature") = _stagnation_temperature[i];
  params.set<Real>("sx") = _stagnation_direction[_dim * i];
  if (_dim == 1)
    params.set<Real>("sy") = 0;
  if (_dim >= 2)
    params.set<Real>("sy") = _stagnation_direction[_dim * i + 1];
  if (_dim >= 3)
    params.set<Real>("sz") = _stagnation_direction[_dim * i + 2];
}

void
CNSAction::coupleVelocities(InputParameters & params)
{
  params.set<CoupledName>(NS::velocity_x) = {NS::velocity_x};

  if (_dim >= 2)
    params.set<CoupledName>(NS::velocity_y) = {NS::velocity_y};

  if (_dim >= 3)
    params.set<CoupledName>(NS::velocity_z) = {NS::velocity_z};
}

void
CNSAction::coupleMomentums(InputParameters & params)
{
  params.set<CoupledName>(NS::momentum_x) = {NS::momentum_x};

  if (_dim >= 2)
    params.set<CoupledName>(NS::momentum_y) = {NS::momentum_y};

  if (_dim >= 3)
    params.set<CoupledName>(NS::momentum_z) = {NS::momentum_z};
}
