//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaplacianJumpIndicator.h"

registerMooseObject("MooseApp", LaplacianJumpIndicator);

InputParameters
LaplacianJumpIndicator::validParams()
{
  InputParameters params = InternalSideIndicator::validParams();
  params.addClassDescription(
      "Compute the jump of the solution laplacian across element bondaries.");
  return params;
}

LaplacianJumpIndicator::LaplacianJumpIndicator(const InputParameters & parameters)
  : InternalSideIndicator(parameters), _second_u(second()), _second_u_neighbor(neighborSecond())
{
}

Real
LaplacianJumpIndicator::computeQpIntegral()
{
  Real jump = (_second_u[_qp].tr() - _second_u_neighbor[_qp].tr());

  return jump * jump;
}
