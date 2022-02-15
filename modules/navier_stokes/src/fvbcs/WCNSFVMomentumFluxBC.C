//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMomentumFluxBC.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMomentumFluxBC);

InputParameters
WCNSFVMomentumFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();

  // this is not really necessary, since advection kernel wont be executed on
  // this flux boundary
  params += INSFVFlowBC::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Flux boundary conditions for momentum advection.");

  params.addParam<Real>("scaling_factor", 1, "To scale the momentum flux");

  // Two different ways to input velocity
  // 1) Postprocessor with the velocity value
  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");

  // 2) Postprocessors with an inlet mass flow rate directly
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");

  return params;
}

WCNSFVMomentumFluxBC::WCNSFVMomentumFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    INSFVMomentumResidualObject(*this),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to WCNSFVMomentumFluxBC must be of type INSFVVelocityVariable");

  // Density is often set as global parameters so it is not checked
  if (_mdot_pp && _velocity_pp)
    mooseWarning("If setting the mass flow rate directly, no need for inlet velocity");

  // Need enough information to compute the mass flux
  if (_mdot_pp && !_area_pp)
    mooseError("The inlet area should be provided along with the mass flow rate");
  if (!_mdot_pp && (!_velocity_pp || !_rho))
    mooseError("Velocity and density should be provided if the mass flow rate is not");
}

ADReal
WCNSFVMomentumFluxBC::computeQpResidual()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  if (_velocity_pp)
    return -_scaling_factor * std::pow((*_velocity_pp), 2) * (*_rho)(singleSidedFaceArg());
  else
    return -_scaling_factor * std::pow((*_mdot_pp), 2) / (*_rho)(singleSidedFaceArg()) /
           (*_area_pp);
}
