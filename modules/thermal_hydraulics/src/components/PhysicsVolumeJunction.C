//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsVolumeJunction.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "THMMesh.h"

registerMooseObjectAliased("ThermalHydraulicsApp", PhysicsVolumeJunction, "VolumeJunction");

InputParameters
PhysicsVolumeJunction::validParams()
{
  InputParameters params = PhysicsFlowJunction::validParams();

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");

  params.addParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addParam<FunctionName>("initial_vel_x", "Initial velocity in x-direction [m/s]");
  params.addParam<FunctionName>("initial_vel_y", "Initial velocity in y-direction [m/s]");
  params.addParam<FunctionName>("initial_vel_z", "Initial velocity in z-direction [m/s]");

  params.addParam<Real>("K", 0., "Form loss factor [-]");
  params.addParam<Real>("A_ref", "Reference area [m^2]");

  params.declareControllable("K");
  params.addClassDescription("Junction between 1-phase flow channels that has a non-zero volume, "
                             "implemented using Physics");

  return params;
}

PhysicsVolumeJunction::PhysicsVolumeJunction(const InputParameters & params)
  : PhysicsFlowJunction(params),

    _volume(getParam<Real>("volume")),
    _position(getParam<Point>("position")),

    _K(getParam<Real>("K")),
    _A_ref(isParamValid("A_ref") ? getParam<Real>("A_ref") : _zero)
{
  // Note: 'A_ref' can be required by child classes
  if (!params.isParamRequired("A_ref") && params.isParamSetByUser("A_ref") &&
      !params.isParamSetByUser("K"))
    logWarning("Parameter 'A_ref' is specified, but 'K' is not specified, so the junction will "
               "behave as if there were no form loss.");
}

void
PhysicsVolumeJunction::setupMesh()
{
  PhysicsFlowJunction::setupMesh();

  // NOTE: we might need to this differently for FV
  // Add a NodeElem to the mesh
  auto * node = addNode(_position);
  auto * elem = addNodeElement(node->id());
  _junction_subdomain_id = mesh().getNextSubdomainId();
  elem->subdomain_id() = _junction_subdomain_id;
  setSubdomainInfo(_junction_subdomain_id, name());

  // Add coupling between the flow channel end elements and the NodeElem
  const auto & elem_ids = getConnectedElementIDs();
  for (unsigned int i = 0; i < elem_ids.size(); i++)
    getTHMProblem().augmentSparsity(elem_ids[i], elem->id());
}

void
PhysicsVolumeJunction::check() const
{
  PhysicsFlowJunction::check();

  bool ics_set =
      getTHMProblem().hasInitialConditionsFromFile() ||
      (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel_x") &&
       isParamValid("initial_vel_y") && isParamValid("initial_vel_z"));

  if (!ics_set && !_app.isRestarting())
  {
    // create a list of the missing IC parameters
    const std::vector<std::string> ic_params{
        "initial_p", "initial_T", "initial_vel_x", "initial_vel_y", "initial_vel_z"};
    std::ostringstream oss;
    for (const auto & ic_param : ic_params)
      if (!isParamValid(ic_param))
        oss << " " << ic_param;

    logError("The following initial condition parameters are missing:", oss.str());
  }

  // https://github.com/idaholab/moose/issues/28670
  if (getTHMProblem().hasInitialConditionsFromFile() && libMesh::n_threads() > 1 &&
      _app.n_processors() > 1)
    mooseDocumentedError("moose",
                         28670,
                         "Using initial conditions from a file for PhysicsVolumeJunction is "
                         "currently not tested for parallel threading.");
}

void
PhysicsVolumeJunction::init()
{
  PhysicsFlowJunction::init();

  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setJunction(name(), ThermalHydraulicsFlowPhysics::Volume);
}

bool
PhysicsVolumeJunction::hasInitialConditions() const
{
  return (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel_x") &&
          isParamValid("initial_vel_y") && isParamValid("initial_vel_z"));
}

void
PhysicsVolumeJunction::getInitialConditions(Real & initial_p,
                                            Real & initial_T,
                                            Real & initial_rho,
                                            Real & initial_E,
                                            RealVectorValue & initial_vel) const
{
  mooseAssert(hasInitialConditions(), "Should not be called if initial conditions are not known");
  Function & initial_p_fn = getTHMProblem().getFunction(getParam<FunctionName>("initial_p"));
  Function & initial_T_fn = getTHMProblem().getFunction(getParam<FunctionName>("initial_T"));
  Function & initial_vel_x_fn =
      getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_x"));
  Function & initial_vel_y_fn =
      getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_y"));
  Function & initial_vel_z_fn =
      getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_z"));

  initial_p_fn.initialSetup();
  initial_T_fn.initialSetup();
  initial_vel_x_fn.initialSetup();
  initial_vel_y_fn.initialSetup();
  initial_vel_z_fn.initialSetup();

  // TODO: use real start time
  initial_p = initial_p_fn.value(0, _position);
  initial_T = initial_T_fn.value(0, _position);
  const Real initial_vel_x = initial_vel_x_fn.value(0, _position);
  const Real initial_vel_y = initial_vel_y_fn.value(0, _position);
  const Real initial_vel_z = initial_vel_z_fn.value(0, _position);

  SinglePhaseFluidProperties & fp =
      getTHMProblem().getUserObject<SinglePhaseFluidProperties>(_fp_name);
  fp.initialSetup();
  initial_rho = fp.rho_from_p_T(initial_p, initial_T);
  initial_vel(0) = initial_vel_x;
  initial_vel(1) = initial_vel_y;
  initial_vel(2) = initial_vel_z;
  initial_E = fp.e_from_p_rho(initial_p, initial_rho) + 0.5 * initial_vel * initial_vel;
}
