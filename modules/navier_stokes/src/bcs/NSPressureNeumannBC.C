//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes inclues
#include "NS.h"
#include "NSPressureNeumannBC.h"

registerMooseObject("NavierStokesApp", NSPressureNeumannBC);

InputParameters
NSPressureNeumannBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();

  params.addClassDescription("This kernel is appropriate for use with a 'zero normal flow' "
                             "boundary condition in the context of the Euler equations.");
  params.addRequiredCoupledVar(NS::pressure, "The current value of the pressure");
  params.addRequiredParam<unsigned>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}

NSPressureNeumannBC::NSPressureNeumannBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _pressure(coupledValue(NS::pressure)),
    _component(getParam<unsigned>("component")),
    _pressure_derivs(*this)
{
}

Real
NSPressureNeumannBC::computeQpResidual()
{
  return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}

Real
NSPressureNeumannBC::computeQpJacobian()
{
  return computeJacobianHelper(_component +
                               1); // <-- the on-diagonal variable number is _component+1
}

Real
NSPressureNeumannBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSPressureNeumannBC::computeJacobianHelper(unsigned m)
{
  return _normals[_qp](_component) * _pressure_derivs.get_grad(m) * _phi[_j][_qp] * _test[_i][_qp];
}
