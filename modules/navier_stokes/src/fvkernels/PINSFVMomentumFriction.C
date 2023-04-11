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
  params.addParam<MooseFunctorName>(NS::porosity, NS::porosity, "the porosity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density.");
  return params;
}

PINSFVMomentumFriction::PINSFVMomentumFriction(const InputParameters & params)
  : INSFVElementalKernel(params),
    _cL(isParamValid("Darcy_name") ? &getFunctor<ADRealVectorValue>("Darcy_name") : nullptr),
    _cQ(isParamValid("Forchheimer_name") ? &getFunctor<ADRealVectorValue>("Forchheimer_name")
                                         : nullptr),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _use_Forchheimer_friction_model(isParamValid("Forchheimer_name")),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _rho(getFunctor<ADReal>(NS::density))
{
  if (!_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");
}

void
PINSFVMomentumFriction::gatherRCData(const Elem & elem)
{
  ADReal friction_term = 0;
  const auto elem_arg = makeElemArg(&elem);
  const auto state = determineState();

  if (_use_Darcy_friction_model)
    friction_term += (*_cL)(elem_arg, state)(_index)*_rho(elem_arg, state) / _eps(elem_arg, state);
  if (_use_Forchheimer_friction_model)
    friction_term += (*_cQ)(elem_arg, state)(_index)*_rho(elem_arg, state) / _eps(elem_arg, state);

  const auto coefficient = friction_term * _assembly.elementVolume(&elem);

  _rc_uo.addToA(&elem, _index, coefficient);

  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  processResidualAndJacobian(coefficient * _u_functor(elem_arg, state), dof_number);
}
