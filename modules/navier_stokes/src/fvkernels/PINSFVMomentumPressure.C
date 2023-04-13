//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressure.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressure);

InputParameters
PINSFVMomentumPressure::validParams()
{
  InputParameters params = INSFVMomentumPressure::validParams();
  params.addClassDescription("Introduces the coupled pressure term $eps \nabla P$ into the "
                             "Navier-Stokes porous media momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");

  return params;
}

PINSFVMomentumPressure::PINSFVMomentumPressure(const InputParameters & params)
  : INSFVMomentumPressure(params), _eps(getFunctor<ADReal>(NS::porosity))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressure may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressure::computeQpResidual()
{
  return _eps(makeElemArg(_current_elem), determineState()) *
         INSFVMomentumPressure::computeQpResidual();
}
