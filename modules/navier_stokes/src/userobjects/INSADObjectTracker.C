//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSADObjectTracker);

InputParameters
INSADObjectTracker::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User object used to track the kernels added to an INS simulation "
                             "and determine what properties to calculate in INSADMaterial");
  return params;
}

INSADObjectTracker::INSADObjectTracker(const InputParameters & parameters)
  : GeneralUserObject(parameters), _tracker_params(emptyInputParameters())
{
  _tracker_params.addParam<bool>("integrate_p_by_parts",
                                 "Whether to integrate the pressure by parts");
  MooseEnum viscous_form("traction laplace", "laplace");
  _tracker_params.addParam<MooseEnum>(
      "viscous_form",
      viscous_form,
      "The form of the viscous term. Options are 'traction' or 'laplace'");
  _tracker_params.addParam<bool>(
      "has_boussinesq", false, "Whether the simulation has the boussinesq approximation");
  _tracker_params.addParam<const ADMaterialProperty<Real> *>(
      "alpha", "The alpha material property for the boussinesq approximation");
  _tracker_params.addParam<const MaterialProperty<Real> *>(
      "ref_temp", "The reference temperature material property");
  _tracker_params.addParam<const ADVariableValue *>("temperature", "The temperature variable");
  _tracker_params.addParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  _tracker_params.addParam<bool>(
      "has_gravity",
      false,
      "Whether the simulation has a gravity force imposed on the momentum equation");
  _tracker_params.addParam<bool>("has_transient", false, "Whether the simulation is transient");

  addAmbientConvectionParams(_tracker_params);

  _tracker_params.addParam<bool>(
      "has_heat_source", false, "Whether there is a heat source function object in the simulation");
  _tracker_params.addParam<const Function *>("heat_source_function",
                                             "The function describing the heat source");
  _tracker_params.addParam<const ADVariableValue *>(
      "heat_source_var",
      "Variable describing the volumetric heat source. Note that if this variable evaluates to a "
      "negative number, then this object will be an energy sink");

  _tracker_params.addParam<bool>(
      "has_coupled_force",
      false,
      "Whether the simulation has a force due to a coupled vector variable/vector function");
  _tracker_params.addParam<std::vector<const ADVectorVariableValue *>>(
      "coupled_force_var", "Variables imposing coupled forces");
  _tracker_params.addParam<std::vector<const Function *>>(
      "coupled_force_vector_function", "The function(s) standing in as a coupled force(s)");
}

void
addAmbientConvectionParams(InputParameters & params)
{
  params.addParam<bool>(
      "has_ambient_convection",
      false,
      "Whether for the energy equation there is a heat source/sink due to convection "
      "from ambient surroundings");
  params.addParam<Real>("ambient_convection_alpha",
                        "The heat transfer coefficient from from ambient surroundings");
  params.addParam<Real>("ambient_temperature", "The ambient temperature");
}
