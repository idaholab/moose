//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMassBC.h"

InputParameters
NSMassBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addClassDescription(
      "This class corresponds to the 'natural' boundary condition for the mass equation.");
  return params;
}

NSMassBC::NSMassBC(const InputParameters & parameters) : NSIntegratedBC(parameters) {}

Real
NSMassBC::qpResidualHelper(Real rhoun)
{
  return rhoun * _test[_i][_qp];
}

Real
NSMassBC::qpJacobianHelper(unsigned var_number)
{
  switch (var_number)
  {
    case 0: // density
    case 4: // energy
      return 0.0;

    case 1:
    case 2:
    case 3: // momentums
      // If one of the momentums, the derivative is a mass
      // matrix times that normal component...
      return _phi[_j][_qp] * _test[_i][_qp] * _normals[_qp](var_number - 1);

    default:
      mooseError("Should not get here!");
      break;
  }
}
