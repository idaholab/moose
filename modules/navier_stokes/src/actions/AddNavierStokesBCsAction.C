/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AddNavierStokesBCsAction.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<AddNavierStokesBCsAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddNavierStokesBCsAction::AddNavierStokesBCsAction(InputParameters parameters) :
    MooseObjectAction(parameters)
{
}

AddNavierStokesBCsAction::~AddNavierStokesBCsAction()
{
}

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
}



void
AddNavierStokesBCsAction::addNSMassWeakStagnationBC()
{
  const std::string kernel_type = "NSMassWeakStagnationBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = "rho";
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
  params.set<NonlinearVariableName>("variable") = "rhoE";
  setCommonParams(params);
  params += _moose_object_pars;
  _problem->addBoundaryCondition(kernel_type, "weak_stagnation_energy_inflow", params);
}



void
AddNavierStokesBCsAction::addNSMomentumWeakStagnationBC(unsigned int component)
{
  const static std::string momentums[3] = {"rhou", "rhov", "rhow"};

  // Convective part
  {
    const std::string kernel_type = "NSMomentumConvectiveWeakStagnationBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = momentums[component];
    setCommonParams(params);
    params += _moose_object_pars;

    // Momentum Kernels also need the component.
    params.set<unsigned int>("component") = component;

    _problem->addBoundaryCondition(kernel_type, std::string("weak_stagnation_") + momentums[component] + std::string("_convective_inflow"), params);
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

    _problem->addBoundaryCondition(kernel_type, std::string("weak_stagnation_") + momentums[component] + std::string("_pressure_inflow"), params);
  }
}



void
AddNavierStokesBCsAction::addNoPenetrationBC(unsigned int component)
{
  const static std::string momentums[3] = {"rhou", "rhov", "rhow"};
  const std::string kernel_type = "NSPressureNeumannBC";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = momentums[component];
  setCommonParams(params);
  params += _moose_object_pars;

  // These BCs also need the component and couping to the pressure.
  params.set<unsigned int>("component") = component;
  params.set<std::vector<VariableName> >("pressure") = {"pressure"};

  _problem->addBoundaryCondition(kernel_type, momentums[component] + std::string("_no_penetration"), params);
}



void
AddNavierStokesBCsAction::setCommonParams (InputParameters & params)
{
  // coupled variables
  params.set<std::vector<VariableName> >("rho")  = {"rho"};
  params.set<std::vector<VariableName> >("rhoE") = {"rhoE"};

  // Couple the appropriate number of velocities
  coupleVelocities(params);
  coupleMomentums(params);
}



void
AddNavierStokesBCsAction::coupleVelocities (InputParameters & params)
{
  params.set<std::vector<VariableName> >("u") = {"vel_x"};

  if (_dim >= 2)
    params.set<std::vector<VariableName> >("v") = {"vel_y"};

  if (_dim >= 3)
    params.set<std::vector<VariableName> >("w") = {"vel_z"};
}



void
AddNavierStokesBCsAction::coupleMomentums (InputParameters & params)
{
  params.set<std::vector<VariableName> >("rhou") = {"rhou"};

  if (_dim >= 2)
    params.set<std::vector<VariableName> >("rhov") = {"rhov"};

  if (_dim >= 3)
    params.set<std::vector<VariableName> >("rhow") = {"rhow"};
}
