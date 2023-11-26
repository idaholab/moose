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
  params.addParam<MooseFunctorName>(NS::mu, "The dynamic viscosity");
  params.addParam<MooseFunctorName>(
      NS::speed,
      "The norm of the interstitial velocity. This is required for Forchheimer calculations");
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
    _mu(isParamValid(NS::mu) ? &getFunctor<ADReal>(NS::mu) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _speed(isParamValid(NS::speed) ? &getFunctor<ADReal>(NS::speed) : nullptr)
{
  if (!_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");

  if (_use_Forchheimer_friction_model && !_speed)
    mooseError("If using a Forchheimer friction model, then the '",
               NS::speed,
               "' parameter must be provided");

  if (_use_Darcy_friction_model && !_mu)
    mooseError(
        "If using a Darcy friction model, then the '", NS::mu, "' parameter must be provided");
}

// We are going to follow the formulations in
// https://holzmann-cfd.com/community/blog-and-tools/darcy-forchheimer and
// https://www.simscale.com/knowledge-base/predict-darcy-and-forchheimer-coefficients-for-perforated-plates-using-analytical-approach/
// for Darcy and Forchheimer which define:
//
// \nabla p = \mu D v + \frac{\rho}{2} F |v| v
//
// where v denotes the interstitial velocity (e.g. the true fluid velocity). Note that to be
// consistent with our overall porous Navier-Stokes formulation, we must multiply the above equation
// by porosity:
//
// \epsilon \nabla p = \epsilon \mu D v + \epsilon \frac{\rho}{2} F |v| v =
// \mu D v_D + \frac{\rho}{2} F |v| v_D
//
// where v_D is the superficial velocity, which is the variable this kernel is acting on. The two
// terms on the final RHS are what this kernel implements. Because both monolithic and segregated
// solves multiply by v_D in the gatherRCData and computeSegregatedContribution methods
// respectively, it is the job of the computeFrictionWCoefficient method to compute, for the Darcy
// term:
//
// \mu D
//
// and for the Forchheimer term
//
// \frac{\rho}{2} F |v|

ADReal
PINSFVMomentumFriction::computeFrictionWCoefficient(const Moose::ElemArg & elem_arg,
                                                    const Moose::StateArg & state)
{
  ADReal coefficient = 0.0;
  if (_use_Darcy_friction_model)
    coefficient += (*_mu)(elem_arg, state) * (*_D)(elem_arg, state)(_index);
  if (_use_Forchheimer_friction_model)
    coefficient +=
        _rho(elem_arg, state) / 2 * (*_F)(elem_arg, state)(_index) * (*_speed)(elem_arg, state);

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
