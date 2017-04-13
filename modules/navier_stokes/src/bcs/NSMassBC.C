/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassBC.h"

template <>
InputParameters
validParams<NSMassBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();
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
