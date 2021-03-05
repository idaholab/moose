//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVFluidSolidConvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVFluidSolidConvection);

InputParameters
NSFVFluidSolidConvection::validParams()
{
  auto params = FVElementalKernel::validParams();
  params.addRequiredCoupledVar(NS::T_solid, "solid temperature");
  params.addClassDescription("Interphase convective heat transfer $\\alpha(T_f-T_s)$ "
                             "in the fluid energy conservation equation.");
  return params;
}

NSFVFluidSolidConvection::NSFVFluidSolidConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _T_solid(adCoupledValue(NS::T_solid)),
    _T_fluid(getADMaterialProperty<Real>(NS::T_fluid)),
    _alpha(getADMaterialProperty<Real>(NS::alpha))
{
}

ADReal
NSFVFluidSolidConvection::computeQpResidual()
{
  return _alpha[_qp] * (_T_fluid[_qp] - _T_solid[_qp]);
}
