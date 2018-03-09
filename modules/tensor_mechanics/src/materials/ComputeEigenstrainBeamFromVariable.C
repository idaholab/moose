//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainBeamFromVariable.h"

template <>
InputParameters
validParams<ComputeEigenstrainBeamFromVariable>()
{
  InputParameters params = validParams<ComputeEigenstrainBeamBase>();
  params.addClassDescription("Computes an eigenstrain from a set of variables");
  params.addCoupledVar("disp_eigenstrain",
                       "A list of variable names describing the "
                       "displacement eigenstrain. If provided, there must be 3 "
                       "of these, corresponding to the axial and shear "
                       "eigenstrains in the global coordinate system.");
  params.addCoupledVar("rot_eigenstrain",
                       "A list of variable names describing the rotational "
                       "eigenstrain. If provided, there must be 3 of these, "
                       "corresponding to the rotational eigenstrain in the "
                       "global coordinate system.");
  return params;
}

ComputeEigenstrainBeamFromVariable::ComputeEigenstrainBeamFromVariable(
    const InputParameters & parameters)
  : ComputeEigenstrainBeamBase(parameters),
    _ndisp(coupledComponents("disp_eigenstrain")),
    _nrot(coupledComponents("rot_eigenstrain")),
    _disp(3),
    _rot(3)
{
  if ((_ndisp != 3 && _ndisp != 0) || (_nrot != 3 && _nrot != 0))
    mooseError("ComputeEigenstrainBeamFromVariable: If the displacement or rotational eigenstrains "
               "are provided, it should contain 3 variables corresponding to the three "
               "components in the global coordinate system.");

  // fetch coupled variables
  if (_ndisp > 0)
    for (unsigned int i = 0; i < _ndisp; ++i)
      _disp[i] = &coupledValue("disp_eigenstrain", i);
  else
    for (unsigned int i = 0; i < 3; ++i)
      _disp[i] = &_zero;

  if (_nrot > 0)
    for (unsigned int i = 0; i < _nrot; ++i)
      _rot[i] = &coupledValue("rot_eigenstrain", i);
  else
    for (unsigned int i = 0; i < 3; ++i)
      _rot[i] = &_zero;
}

void
ComputeEigenstrainBeamFromVariable::computeQpEigenstrain()
{
  for (unsigned int i = 0; i < 3; ++i)
  {
    _disp_eigenstrain[_qp](i) = (*_disp[i])[_qp];
    _rot_eigenstrain[_qp](i) = (*_rot[i])[_qp];
  }
}
