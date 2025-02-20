//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumFriction.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumFriction);

InputParameters
LinearFVMomentumFriction::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Computes a friction force term on fluid in porous media in the "
      "Navier Stokes i-th momentum equation in Rhie-Chow (incompressible) contexts.");
  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addParam<MooseFunctorName>(NS::mu, "The dynamic viscosity");

  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVMomentumFriction::LinearFVMomentumFriction(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _D(isParamValid("Darcy_name") ? &getFunctor<RealVectorValue>("Darcy_name") : nullptr),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _mu(isParamValid(NS::mu) ? &getFunctor<Real>(NS::mu) : nullptr)
{
  if (!_use_Darcy_friction_model)
    mooseError("At least one friction model needs to be specified.");

  if (_use_Darcy_friction_model && !_mu)
    mooseError("If using the standard Darcy friction model, then the '",
               NS::mu,
               "' parameter must be provided");
}

// We are going to follow the formulations in
// https://holzmann-cfd.com/community/blog-and-tools/darcy-forchheimer and
// https://www.simscale.com/knowledge-base/predict-darcy-and-forchheimer-coefficients-for-perforated-plates-using-analytical-approach/
// for Darcy and Forchheimer which define:
//
// \nabla p = \mu D v + \frac{\rho}{2} F |v| v
//
// where v denotes the superficial velocity.
// Both monolithic and segregated solves get multiplied by v in the gatherRCData and
// computeSegregatedContribution methods respectively, it is the job of the
// computeFrictionWCoefficient method to compute, for the Darcy term:
//
// \mu D
//
// and for the Forchheimer term (not implemented yet)
//
// \frac{\rho}{2} F |v|
//
// For the non standard formulation we define :
// \nabla p = D v + F |v| v

Real
LinearFVMomentumFriction::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  return computeFrictionWCoefficient(elem_arg, state) * _current_elem_volume;
}

Real
LinearFVMomentumFriction::computeRightHandSideContribution()
{
  return 0;
}

Real
LinearFVMomentumFriction::computeFrictionWCoefficient(const Moose::ElemArg & elem_arg,
                                                      const Moose::StateArg & state)
{
  // Forward declaration of the coeffcients to be used and returned by the model
  Real coefficient = 0.0;
  Real mu = 0.0;

  /// Fluid properties
  if (_use_Darcy_friction_model)
    mu = (*_mu)(elem_arg, state);

  ////////////////////////////////////////////////////////////////////
  ///// Switching across formulation cases
  ////////////////////////////////////////////////////////////////////

  if (_use_Darcy_friction_model)
    coefficient += mu * (*_D)(elem_arg, state)(_index);

  return coefficient;
}
