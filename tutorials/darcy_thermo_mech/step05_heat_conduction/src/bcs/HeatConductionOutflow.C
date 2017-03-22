/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "HeatConductionOutflow.h"

template <>
InputParameters
validParams<HeatConductionOutflow>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

HeatConductionOutflow::HeatConductionOutflow(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // IntegratedBCs can retrieve material properties!
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{
}

Real
HeatConductionOutflow::computeQpResidual()
{
  return -_test[_i][_qp] * _thermal_conductivity[_qp] * _grad_u[_qp] * _normals[_qp];
}

Real
HeatConductionOutflow::computeQpJacobian()
{
  // Derivative of the residual with respect to "u"
  return -_test[_i][_qp] * _thermal_conductivity[_qp] * _grad_phi[_j][_qp] * _normals[_qp];
}
