//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumFriction.h"
#include "NS.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumFriction);

InputParameters
PINSFVMomentumFriction::validParams()
{
  InputParameters params = INSFVElementalKernel::validParams();
  params.addClassDescription(
      "Computes a friction force term on fluid in porous media in the "
      "Navier Stokes i-th momentum equation in Rhie-Chow (incompressible) contexts.");
  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addParam<MooseFunctorName>("Forchheimer_name",
                                    "Name of the Forchheimer coefficients property.");
  params.addParam<bool>("is_porous_medium", true, "Boolean to choose the type of medium.");
  params.addParam<bool>(
      "standard_friction_formulation", true, "Boolean to choose the type of friction formulation.");
  params.addParam<MooseFunctorName>(NS::mu, "The dynamic viscosity");
  params.addParam<MooseFunctorName>(NS::speed, "The magnitude of the interstitial velocity.");
  params.addParam<MooseFunctorName>("u",
                                    "The velocity in the x direction. Superficial in the case of "
                                    "porous treatment, interstitial otherwise");
  params.addParam<MooseFunctorName>("v",
                                    "The velocity in the y direction. Superficial in the case of "
                                    "porous treatment, interstitial otherwise");
  params.addParam<MooseFunctorName>("w",
                                    "The velocity in the z direction. Superficial in the case of "
                                    "porous treatment, interstitial otherwise");
  params.addParam<MooseFunctorName>(NS::porosity, 1.0, "The porosity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density.");
  return params;
}

PINSFVMomentumFriction::PINSFVMomentumFriction(const InputParameters & params)
  : INSFVElementalKernel(params),
    _D(isParamValid("Darcy_name") ? &getFunctor<ADRealVectorValue>("Darcy_name") : nullptr),
    _F(isParamValid("Forchheimer_name") ? &getFunctor<ADRealVectorValue>("Forchheimer_name")
                                        : nullptr),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _use_Forchheimer_friction_model(isParamValid("Forchheimer_name")),
    _is_porous_medium(getParam<bool>("is_porous_medium")),
    _standard_friction_formulation(getParam<bool>("standard_friction_formulation")),
    _mu(isParamValid(NS::mu) ? &getFunctor<ADReal>(NS::mu) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _speed(isParamValid(NS::speed) ? &getFunctor<ADReal>(NS::speed) : nullptr),
    _dim(_subproblem.mesh().dimension()),
    _u_var(params.isParamValid("u") ? &(getFunctor<ADReal>("u")) : nullptr),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _epsilon(getFunctor<ADReal>(NS::porosity))
{
  if (!_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");

  if (_standard_friction_formulation && _use_Darcy_friction_model && !_mu)
    mooseError("If using the standard Darcy friction model, then the '",
               NS::mu,
               "' parameter must be provided");
  if (_use_Forchheimer_friction_model && !_speed)
  {
    if (_dim >= 1)
    {
      if (!_u_var)
        mooseError("The velocity variable 'u' should be defined if no speed functor material is "
                   "defined for this kernel when using the Forchheimer fromulation.");
    }
    if (_dim >= 2)
    {
      if (!_v_var)
        mooseError(
            "The velocity variable 'v' should be defined if no speed functor material is "
            "defined for this kernel and dimensions >= 2 when using the Forchheimer fromulation.");
    }
    if (_dim >= 3)
    {
      if (!_w_var)
        mooseError(
            "The velocity variable 'w' should be defined if no speed functor material is "
            "defined for this kernel and dimensions >= 3 when using the Forchheimer fromulation.");
    }
  }
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
// and for the Forchheimer term
//
// \frac{\rho}{2} F |v|
//
// For the non standard formulation we define :
// \nabla p = D v + F |v| v

ADReal
PINSFVMomentumFriction::computeFrictionWCoefficient(const Moose::ElemArg & elem_arg,
                                                    const Moose::StateArg & state)
{
  // Forward declaration of the coeffcients to be used and returned by the model
  ADReal coefficient = 0.0;
  ADReal speed = 0.0;
  ADReal rho = 0.0;
  ADReal mu = 0.0;
  /// Speed treatment
  if (_use_Forchheimer_friction_model && _speed)
    speed = (*_speed)(elem_arg, state);
  else if (_use_Forchheimer_friction_model && !_speed)
  {
    ADRealVectorValue superficial_velocity;
    if (_dim >= 1)
    {
      superficial_velocity(0) = (*_u_var)(elem_arg, state);
      if (_dim >= 2)
      {
        superficial_velocity(1) = (*_v_var)(elem_arg, state);
        if (_dim >= 3)
          superficial_velocity(2) = (*_w_var)(elem_arg, state);
      }
    }
    speed = NS::computeSpeed(superficial_velocity);
    if (_is_porous_medium)
    {
      speed *= (1 / _epsilon(elem_arg, state));
    }
  }

  /// Fluid properies
  if (_use_Darcy_friction_model && _standard_friction_formulation)
  {
    mu = (*_mu)(elem_arg, state);
  }

  if (_use_Forchheimer_friction_model && _standard_friction_formulation)
  {
    rho = _rho(elem_arg, state);
  }

  ////////////////////////////////////////////////////////////////////
  ///// Switching across formulation cases
  ////////////////////////////////////////////////////////////////////
  if (_standard_friction_formulation)
  {
    if (_use_Darcy_friction_model)
      coefficient += mu * (*_D)(elem_arg, state)(_index);
    if (_use_Forchheimer_friction_model)
      coefficient += rho / 2 * (*_F)(elem_arg, state)(_index)*speed;
  }
  else
  {
    if (_use_Darcy_friction_model)
      coefficient += (*_D)(elem_arg, state)(_index);
    if (_use_Forchheimer_friction_model)
      coefficient += (*_F)(elem_arg, state)(_index)*speed;
  }

  return coefficient;
}

ADReal
PINSFVMomentumFriction::computeSegregatedContribution()
{
  const auto & elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return raw_value(computeFrictionWCoefficient(elem_arg, state)) * _u_functor(elem_arg, state);
}

void
PINSFVMomentumFriction::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  const auto state = determineState();
  const auto coefficient =
      computeFrictionWCoefficient(elem_arg, state) * _assembly.elementVolume(&elem);
  _rc_uo.addToA(&elem, _index, coefficient);
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  addResidualAndJacobian(coefficient * _u_functor(elem_arg, state), dof_number);
}
