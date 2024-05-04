//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "INSFVRhieChowInterpolator.h"
#include "RelationshipManager.h"
#include "WCNSFVFlowPhysics.h"

InputParameters
WCNSFVCoupledAdvectionPhysicsHelper::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addClassDescription("Class to facilitate coupling between a Physics defining flow "
                             "equations and other objects, notably advection Physics.");

  params.addParam<PhysicsName>("coupled_flow_physics",
                               "WCNSFVFlowPhysics generating the velocities");
  params.addParamNamesToGroup("coupled_flow_physics", "Coupled Physics");

  return params;
}

WCNSFVCoupledAdvectionPhysicsHelper::WCNSFVCoupledAdvectionPhysicsHelper(
    const InputParameters & parameters, const NavierStokesPhysicsBase * derived_physics)
  : _advection_physics(derived_physics),
    // note: we could move all this initialization to an initalSetup routine to avoid
    // creation ordering difficulties
    _flow_equations_physics(getCoupledFlowPhysics(parameters)),
    _compressibility(_flow_equations_physics->compressibility()),
    _porous_medium_treatment(_flow_equations_physics->porousMediumTreatment()),
    _velocity_names(_flow_equations_physics->getVelocityNames()),
    _pressure_name(_flow_equations_physics->getPressureName()),
    _density_name(_flow_equations_physics->densityName()),
    _dynamic_viscosity_name(_flow_equations_physics->dynamicViscosityName()),
    _velocity_interpolation(_flow_equations_physics->getVelocityFaceInterpolationMethod())
{
}

MooseFunctorName
WCNSFVCoupledAdvectionPhysicsHelper::getPorosityFunctorName(bool smoothed) const
{
  return _flow_equations_physics->getPorosityFunctorName(smoothed);
}

const WCNSFVFlowPhysics *
WCNSFVCoupledAdvectionPhysicsHelper::getCoupledFlowPhysics(const InputParameters & parameters) const
{
  // User passed it, just use that
  if (parameters.isParamValid("coupled_flow_physics"))
    return _advection_physics->getCoupledPhysics<WCNSFVFlowPhysics>(
        parameters.get<PhysicsName>("coupled_flow_physics"));
  // Look for any physics of the right type, and check the block restriction
  else
  {
    const auto all_flow_physics = _advection_physics->getCoupledPhysics<const WCNSFVFlowPhysics>();
    for (const auto physics : all_flow_physics)
      if (_advection_physics->checkBlockRestrictionIdentical(
              physics->name(), physics->blocks(), /*error_if_not_identical=*/false))
      {
        return physics;
      }
  }
  mooseError(
      "No coupled flow Physics found of type 'WCNSFVFlowPhysics'. Use the 'coupled_flow_physics' "
      "parameter to give the name of the desired WCNSFVFlowPhysics to couple with");
}
