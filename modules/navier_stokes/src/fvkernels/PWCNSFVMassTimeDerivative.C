//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PWCNSFVMassTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PWCNSFVMassTimeDerivative);

InputParameters
PWCNSFVMassTimeDerivative::validParams()
{
  InputParameters params = WCNSFVMassTimeDerivative::validParams();
  params.addClassDescription("Adds the time derivative term to the porous weakly-compressible "
                             "Navier-Stokes continuity equation.");
  params.addParam<MooseFunctorName>(NS::porosity, NS::porosity, "the porosity");
  return params;
}

PWCNSFVMassTimeDerivative::PWCNSFVMassTimeDerivative(const InputParameters & params)
  : WCNSFVMassTimeDerivative(params), _eps(getFunctor<ADReal>(NS::porosity))
{
}

ADReal
PWCNSFVMassTimeDerivative::computeQpResidual()
{
  return _eps(makeElemArg(_current_elem), determineState()) *
         WCNSFVMassTimeDerivative::computeQpResidual();
}
