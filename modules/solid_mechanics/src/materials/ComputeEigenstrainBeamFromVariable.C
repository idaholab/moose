//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainBeamFromVariable.h"

registerMooseObject("TensorMechanicsApp", ComputeEigenstrainBeamFromVariable);

InputParameters
ComputeEigenstrainBeamFromVariable::validParams()
{
  InputParameters params = ComputeEigenstrainBeamBase::validParams();
  params.addClassDescription("Computes an eigenstrain from a set of variables");
  params.addCoupledVar("displacement_eigenstrain_variables",
                       "A list of variable names describing the "
                       "displacement eigenstrain. If provided, there must be 3 "
                       "entries, corresponding to the axial and shear "
                       "eigenstrains in the global coordinate system.");
  params.addCoupledVar("rotational_eigenstrain_variables",
                       "A list of variable names describing the rotational "
                       "eigenstrain. If provided, there must be 3 entries, "
                       "corresponding to the rotational eigenstrain in the "
                       "global coordinate system.");
  return params;
}

ComputeEigenstrainBeamFromVariable::ComputeEigenstrainBeamFromVariable(
    const InputParameters & parameters)
  : ComputeEigenstrainBeamBase(parameters),
    _ndisp(coupledComponents("displacement_eigenstrain_variables")),
    _nrot(coupledComponents("rotational_eigenstrain_variables")),
    _disp(_ndisp > 0 ? coupledValues("displacement_eigenstrain_variables")
                     : std::vector<const VariableValue *>(3, &_zero)),
    _rot(_nrot > 0 ? coupledValues("rotational_eigenstrain_variables")
                   : std::vector<const VariableValue *>(3, &_zero))
{
  if ((_ndisp != 3 && _ndisp != 0) || (_nrot != 3 && _nrot != 0))
    mooseError("ComputeEigenstrainBeamFromVariable: If the displacement or rotational eigenstrains "
               "are provided, it should contain 3 variables corresponding to the three "
               "components in the global coordinate system.");
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
