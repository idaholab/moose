/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NS.h"
#include "AddNavierStokesBCsAction.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<AddNavierStokesBCsAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addClassDescription("This class allows us to have a section of the input file like the "
                             "following which adds BC objects for each requested boundary "
                             "condition.");
  return params;
}

AddNavierStokesBCsAction::AddNavierStokesBCsAction(InputParameters parameters)
  : MooseObjectAction(parameters)
{
}

AddNavierStokesBCsAction::~AddNavierStokesBCsAction() {}

void
AddNavierStokesBCsAction::act()
{
  // The Mesh dimension tells us how many momentum BCs to add.
  _dim = _mesh->dimension();

  if (_type == "NSWeakStagnationInletBC")
  {
    addNSMassWeakStagnationBC();
    addNSEnergyWeakStagnationBC();
    for (unsigned int component = 0; component < _dim; ++component)
      addNSMomentumWeakStagnationBC(component);
  }

  else if (_type == "NSNoPenetrationBC")
  {
    for (unsigned int component = 0; component < _dim; ++component)
      addNoPenetrationBC(component);
  }

  else if (_type == "NSStaticPressureOutletBC")
  {
    addNSMassUnspecifiedNormalFlowBC();
    addNSEnergyInviscidSpecifiedPressureBC();
    for (unsigned int component = 0; component < _dim; ++component)
      addNSMomentumInviscidSpecifiedPressureBC(component);
  }
}

void
AddNavierStokesBCsAction::addNSMassWeakStagnationBC()
{
  const std::string kernel_type = "NSMassWeakStagnationBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setCommonParams(params);

  // Pick up values specific to this BC from the Action's params.
  // This includes things like boundary, stagnation_pressure,
  // stagnation_temperature, etc.  This extra InputParameters object
  // is provided by the MooseObjectAction base class.
  params += _moose_object_pars;

  _problem->addBoundaryCondition(kernel_type, "weak_stagnation_mass_inflow", params);
}

void
AddNavierStokesBCsAction::addNSEnergyWeakStagnationBC()
{
  const std::string kernel_type = "NSEnergyWeakStagnationBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy;
  setCommonParams(params);
  params += _moose_object_pars;
  _problem->addBoundaryCondition(kernel_type, "weak_stagnation_energy_inflow", params);
}

void
AddNavierStokesBCsAction::addNSMomentumWeakStagnationBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};

  // Convective part
  {
    const std::string kernel_type = "NSMomentumConvectiveWeakStagnationBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = momentums[component];
    setCommonParams(params);
    params += _moose_object_pars;

    // Momentum Kernels also need the component.
    params.set<unsigned int>("component") = component;

    _problem->addBoundaryCondition(kernel_type,
                                   std::string("weak_stagnation_") + momentums[component] +
                                       std::string("_convective_inflow"),
                                   params);
  }

  // Pressure part
  {
    const std::string kernel_type = "NSMomentumPressureWeakStagnationBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = momentums[component];
    setCommonParams(params);
    params += _moose_object_pars;

    // Momentum Kernels also need the component.
    params.set<unsigned int>("component") = component;

    _problem->addBoundaryCondition(kernel_type,
                                   std::string("weak_stagnation_") + momentums[component] +
                                       std::string("_pressure_inflow"),
                                   params);
  }
}

void
AddNavierStokesBCsAction::addNoPenetrationBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};
  const std::string kernel_type = "NSPressureNeumannBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setCommonParams(params);
  params += _moose_object_pars;

  // These BCs also need the component and couping to the pressure.
  params.set<unsigned int>("component") = component;
  params.set<CoupledName>(NS::pressure) = {NS::pressure};

  _problem->addBoundaryCondition(
      kernel_type, momentums[component] + std::string("_no_penetration"), params);
}

void
AddNavierStokesBCsAction::addNSMassUnspecifiedNormalFlowBC()
{
  const std::string kernel_type = "NSMassUnspecifiedNormalFlowBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::density;
  setCommonParams(params);
  params += _moose_object_pars;
  _problem->addBoundaryCondition(kernel_type, "mass_outflow", params);
}

void
AddNavierStokesBCsAction::addNSMomentumInviscidSpecifiedPressureBC(unsigned int component)
{
  const static std::string momentums[3] = {NS::momentum_x, NS::momentum_y, NS::momentum_z};
  const std::string kernel_type = "NSMomentumInviscidSpecifiedPressureBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setCommonParams(params);
  params += _moose_object_pars;

  // These BCs also need the component.
  params.set<unsigned int>("component") = component;

  _problem->addBoundaryCondition(
      kernel_type, momentums[component] + std::string("_specified_pressure_outflow"), params);
}

void
AddNavierStokesBCsAction::addNSEnergyInviscidSpecifiedPressureBC()
{
  const std::string kernel_type = "NSEnergyInviscidSpecifiedPressureBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = NS::total_energy;
  setCommonParams(params);
  params += _moose_object_pars;
  // This BC also requires the current value of the temperature.
  params.set<CoupledName>(NS::temperature) = {NS::temperature};
  _problem->addBoundaryCondition(kernel_type, "rhoE_specified_pressure_outflow", params);
}

void
AddNavierStokesBCsAction::setCommonParams(InputParameters & params)
{
  // coupled variables
  params.set<CoupledName>(NS::density) = {NS::density};
  params.set<CoupledName>(NS::total_energy) = {NS::total_energy};

  // Couple the appropriate number of velocities
  coupleVelocities(params);
  coupleMomentums(params);
}

void
AddNavierStokesBCsAction::coupleVelocities(InputParameters & params)
{
  params.set<CoupledName>(NS::velocity_x) = {NS::velocity_x};

  if (_dim >= 2)
    params.set<CoupledName>(NS::velocity_y) = {NS::velocity_y};

  if (_dim >= 3)
    params.set<CoupledName>(NS::velocity_z) = {NS::velocity_z};
}

void
AddNavierStokesBCsAction::coupleMomentums(InputParameters & params)
{
  params.set<CoupledName>(NS::momentum_x) = {NS::momentum_x};

  if (_dim >= 2)
    params.set<CoupledName>(NS::momentum_y) = {NS::momentum_y};

  if (_dim >= 3)
    params.set<CoupledName>(NS::momentum_z) = {NS::momentum_z};
}
