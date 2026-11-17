//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearPWCNSFVMomentumPressure.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", LinearPWCNSFVMomentumPressure);

InputParameters
LinearPWCNSFVMomentumPressure::validParams()
{
  auto params = LinearFVMomentumPressure::validParams();
  params.addClassDescription(
      "Adds the porosity-weighted pressure gradient to a porous momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity functor.");
  return params;
}

LinearPWCNSFVMomentumPressure::LinearPWCNSFVMomentumPressure(const InputParameters & params)
  : LinearFVMomentumPressure(params), _porosity(getFunctor<Real>(NS::porosity))
{
}

Real
LinearPWCNSFVMomentumPressure::computeRightHandSideContribution()
{
  return _porosity(makeElemArg(_current_elem_info->elem()), determineState()) *
         LinearFVMomentumPressure::computeRightHandSideContribution();
}
