//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisotropicDragCoefficients.h"

defineADValidParams(AnisotropicDragCoefficients,
                    DragCoefficients,
                    params.addParam<RealVectorValue>(
                        "multipliers",
                        RealVectorValue(1.0, 1.0, 1.0),
                        "Multipliers to be applied to the components of the drag coefficient"););

AnisotropicDragCoefficients::AnisotropicDragCoefficients(
    const InputParameters & parameters)
  : DragCoefficients(parameters),
    _multipliers(getParam<RealVectorValue>("multipliers"))
{
}

void
AnisotropicDragCoefficients::computeQpProperties()
{
  _cL[_qp] = _multipliers * computeDarcyPrefactor();
  _cQ[_qp] = _multipliers * computeForchheimerPrefactor();

  for (unsigned int i = 0; i < 3; ++i)
  {
    _cL[_qp](i) *= computeDarcyCoefficient(i);
    _cQ[_qp](i) *= computeForchheimerCoefficient(i);
  }
}
