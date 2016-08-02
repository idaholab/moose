/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AddNavierStokesKernelsAction.h"

// MOOSE includes
#include "FEProblem.h"

template<>
InputParameters validParams<AddNavierStokesKernelsAction>()
{
  InputParameters params = validParams<NSAction>();
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");
  return params;
}

AddNavierStokesKernelsAction::AddNavierStokesKernelsAction(InputParameters parameters) :
    NSAction(parameters),
    _fp_name(getParam<UserObjectName>("fluid_properties"))
{
}

AddNavierStokesKernelsAction::~AddNavierStokesKernelsAction()
{
}

void
AddNavierStokesKernelsAction::act()
{
  // Call the base class's act() function to initialize the _vars and _auxs names.
  NSAction::act();

  // Add time derivative Kernel for all the _vars
  for (const auto & name : _vars)
  {
    const std::string kernel_type = "TimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = name;
    _problem->addKernel(kernel_type, name + std::string("_time_deriv"), params);
  }

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
  addPressureOrTemperatureAux("NSPressureAux");
  addPressureOrTemperatureAux("NSTemperatureAux");
  addNSEnthalpyAux();
  addNSMachAux();
  addNSInternalEnergyAux();
  addNSSpecificVolumeAux();
  for (unsigned int component = 0; component < _dim; ++component)
    addNSVelocityAux(component);
}



void
AddNavierStokesKernelsAction::addNSSUPGMass()
{
  const std::string kernel_type = "NSSUPGMass";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = "rho";
  setCommonParams(params);

  // SUPG Kernels also need temperature and enthalpy currently.
  params.set<std::vector<VariableName> >("temperature") = {"temperature"};
  params.set<std::vector<VariableName> >("enthalpy") = {"enthalpy"};

  _problem->addKernel(kernel_type, "rho_supg", params);
}



void
AddNavierStokesKernelsAction::addNSSUPGMomentum(unsigned int component)
{
  const static std::string momentums[3] = {"rhou", "rhov", "rhow"};
  const std::string kernel_type = "NSSUPGMomentum";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setCommonParams(params);

  // SUPG Kernels also need temperature and enthalpy currently.
  params.set<std::vector<VariableName> >("temperature") = {"temperature"};
  params.set<std::vector<VariableName> >("enthalpy") = {"enthalpy"};

  // Momentum Kernels also need the component.
  params.set<unsigned int>("component") = component;

  _problem->addKernel(kernel_type, momentums[component] + std::string("_supg"), params);
}



void
AddNavierStokesKernelsAction::addNSSUPGEnergy()
{
  const std::string kernel_type = "NSSUPGEnergy";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = "rhoE";
  setCommonParams(params);

  // SUPG Kernels also need temperature and enthalpy currently.
  params.set<std::vector<VariableName> >("temperature") = {"temperature"};
  params.set<std::vector<VariableName> >("enthalpy") = {"enthalpy"};

  _problem->addKernel(kernel_type, "rhoE_supg", params);
}



void
AddNavierStokesKernelsAction::addNSSpecificVolumeAux()
{
  const std::string kernel_type = "NSSpecificVolumeAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = "specific_volume";

  // coupled variables
  params.set<std::vector<VariableName> >("rho") = {"rho"};

  _problem->addAuxKernel(kernel_type, "specific_volume_auxkernel", params);
}



void
AddNavierStokesKernelsAction::addNSInternalEnergyAux()
{
  const std::string kernel_type = "NSInternalEnergyAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = "internal_energy";

  // coupled variables
  params.set<std::vector<VariableName> >("rho") = {"rho"};
  params.set<std::vector<VariableName> >("rhoE") = {"rhoE"};

  // Couple the appropriate number of velocities
  coupleVelocities(params);

  _problem->addAuxKernel(kernel_type, "internal_energy_auxkernel", params);
}


void
AddNavierStokesKernelsAction::addNSMachAux()
{
  const std::string kernel_type = "NSMachAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = "Mach";

  // coupled variables
  params.set<std::vector<VariableName> >("internal_energy") = {"internal_energy"};
  params.set<std::vector<VariableName> >("specific_volume") = {"specific_volume"};

  // Couple the appropriate number of velocities
  coupleVelocities(params);

  params.set<UserObjectName>("fluid_properties") = _fp_name;

  _problem->addAuxKernel(kernel_type, "mach_auxkernel", params);
}



void
AddNavierStokesKernelsAction::addNSEnthalpyAux()
{
  const std::string kernel_type = "NSEnthalpyAux";

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = "enthalpy";

  // coupled variables
  params.set<std::vector<VariableName> >("rho") = {"rho"};
  params.set<std::vector<VariableName> >("rhoE") = {"rhoE"};
  params.set<std::vector<VariableName> >("pressure") = {"pressure"};

  _problem->addAuxKernel(kernel_type, "enthalpy_auxkernel", params);
}



void
AddNavierStokesKernelsAction::addNSVelocityAux(unsigned int component)
{
  const std::string kernel_type = "NSVelocityAux";
  const static std::string velocities[3] = {"vel_x", "vel_y", "vel_z"};
  const static std::string momentums[3] = {"rhou", "rhov", "rhow"};

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<AuxVariableName>("variable") = velocities[component];

  // coupled variables
  params.set<std::vector<VariableName> >("rho")  = {"rho"};
  params.set<std::vector<VariableName> >("momentum") = {momentums[component]};
  params.set<UserObjectName>("fluid_properties") = _fp_name;

  _problem->addAuxKernel(kernel_type, velocities[component] + "_auxkernel", params);
}



void
AddNavierStokesKernelsAction::addPressureOrTemperatureAux(const std::string & kernel_type)
{
  InputParameters params = _factory.getValidParams(kernel_type);
  std::string var_name = kernel_type == "NSPressureAux" ? "pressure" : "temperature";
  params.set<AuxVariableName>("variable") = var_name;

  // coupled variables
  params.set<std::vector<VariableName> >("internal_energy")  = {"internal_energy"};
  params.set<std::vector<VariableName> >("specific_volume")  = {"specific_volume"};
  params.set<UserObjectName>("fluid_properties") = _fp_name;

  _problem->addAuxKernel(kernel_type, var_name + "_auxkernel", params);
}



void
AddNavierStokesKernelsAction::addNSMassInviscidFlux()
{
  const std::string kernel_type = "NSMassInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = "rho";
  setCommonParams(params);
  _problem->addKernel(kernel_type, "rho_if", params);
}



void
AddNavierStokesKernelsAction::addNSMomentumInviscidFlux(unsigned int component)
{
  const static std::string momentums[3] = {"rhou", "rhov", "rhow"};
  const std::string kernel_type = "NSMomentumInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setCommonParams(params);

  // Extra stuff needed by momentum Kernels
  params.set<std::vector<VariableName> >("pressure") = {"pressure"};
  params.set<unsigned int>("component") = component;

  // Add the Kernel
  _problem->addKernel(kernel_type, momentums[component] + std::string("if"), params);
}



void
AddNavierStokesKernelsAction::addNSEnergyInviscidFlux()
{
  const std::string kernel_type = "NSEnergyInviscidFlux";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = "rhoE";
  setCommonParams(params);

  // Extra stuff needed by energy equation
  params.set<std::vector<VariableName> >("enthalpy") = {"enthalpy"};

  // Add the Kernel
  _problem->addKernel(kernel_type, "rhoE_if", params);
}



void
AddNavierStokesKernelsAction::setCommonParams (InputParameters & params)
{
  // coupled variables
  params.set<std::vector<VariableName> >("rho")  = {"rho"};
  params.set<std::vector<VariableName> >("rhoE") = {"rhoE"};

  // Couple the appropriate number of velocities
  coupleVelocities(params);
  coupleMomentums(params);

  // FluidProperties object
  params.set<UserObjectName>("fluid_properties") = _fp_name;
}



void
AddNavierStokesKernelsAction::coupleVelocities (InputParameters & params)
{
  params.set<std::vector<VariableName> >("u") = {"vel_x"};

  if (_dim >= 2)
    params.set<std::vector<VariableName> >("v") = {"vel_y"};

  if (_dim >= 3)
    params.set<std::vector<VariableName> >("w") = {"vel_z"};
}



void
AddNavierStokesKernelsAction::coupleMomentums (InputParameters & params)
{
  params.set<std::vector<VariableName> >("rhou") = {"rhou"};

  if (_dim >= 2)
    params.set<std::vector<VariableName> >("rhov") = {"rhov"};

  if (_dim >= 3)
    params.set<std::vector<VariableName> >("rhow") = {"rhow"};
}
