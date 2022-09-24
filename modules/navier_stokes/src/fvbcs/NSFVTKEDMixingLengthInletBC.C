//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVTKEDMixingLengthInletBC.h"

registerMooseObject("MooseApp", NSFVTKEDMixingLengthInletBC);

InputParameters
NSFVTKEDMixingLengthInletBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Inlet boundary condition for the turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Fuild density");
  params.addParam<Real>("turbulent_intensity", "The value of the field at the inlet.");
  params.addParam<MooseFunctorName>("C_mu", "The value of the field at the inlet.");
  params.addParam<MooseFunctorName>("mixing_length", "The value of the field at the inlet.");
  return params;
}

NSFVTKEDMixingLengthInletBC::NSFVTKEDMixingLengthInletBC(const InputParameters & params)
  : FVFluxBC(params),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v")
             ? &(getFunctor<ADReal>("v"))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? &(getFunctor<ADReal>("w"))
               : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _mixing_length(getFunctor<ADReal>("mixing_length")),
    _intensity(getParam<Real>("turbulent_intensity")) // this is not Functor<ADReal> becasue it would break ergodicity
{
}

ADReal
NSFVTKEDMixingLengthInletBC::computeQpResidual()
{

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(singleSidedFaceArg()));
  if (_v_var)
      velocity(1) = (*_v_var)(singleSidedFaceArg());
  if (_w_var)
      velocity(2) = (*_w_var)(singleSidedFaceArg());

  auto k0 = 1.5 * std::pow(velocity.norm() * _intensity, 2);

  auto eps0 = std::pow(_C_mu(singleSidedFaceArg()), 0.75)
              * std::pow(k0, 1.5) 
              / _mixing_length(singleSidedFaceArg());

  return (_normal * velocity) * _rho(singleSidedFaceArg()) * eps0;
}
