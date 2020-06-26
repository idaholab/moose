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
  : GeneralUserObject(parameters), InputParameters(emptyInputParameters())
{
  addParam<bool>("integrate_p_by_parts", "Whether to integrate the pressure by parts");
  MooseEnum viscous_form("traction laplace", "laplace");
  addParam<MooseEnum>("viscous_form",
                      viscous_form,
                      "The form of the viscous term. Options are 'traction' or 'laplace'");
  addParam<bool>(
      "has_boussinesq", false, "Whether the simulation has the boussinesq approximation");
  addParam<const ADMaterialProperty<Real> *>(
      "alpha", "The alpha material property for the boussinesq approximation");
  addParam<const MaterialProperty<Real> *>("ref_temp",
                                           "The reference temperature material property");
  addParam<const ADVariableValue *>("temperature", "The temperature variable");
  addParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  addParam<bool>("has_gravity",
                 false,
                 "Whether the simulation has a gravity force imposed on the momentum equation");
  addParam<bool>("has_transient", false, "Whether the simulation is transient");

  addWallConvectionParams(*this);
}

void
addWallConvectionParams(InputParameters & params)
{
  params.addParam<bool>(
      "has_wall_convection",
      false,
      "Whether for the energy equation there is a heat source/sink due to convection "
      "from ambient walls/surroundings");
  params.addParam<Real>("wall_convection_alpha",
                        "The heat transfer coefficient from from ambient walls/surroundings");
  params.addParam<Real>("wall_temperature", "The wall/ambient temperature");
}
