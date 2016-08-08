/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes inclues
#include "NSPressureNeumannBC.h"
#include "NS.h"

template<>
InputParameters validParams<NSPressureNeumannBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();

  params.addRequiredCoupledVar(NS::pressure, "The current value of the pressure");
  params.addRequiredParam<unsigned>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}



NSPressureNeumannBC::NSPressureNeumannBC(const InputParameters & parameters) :
    NSIntegratedBC(parameters),
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
  return computeJacobianHelper(_component + 1);  // <-- the on-diagonal variable number is _component+1
}



Real
NSPressureNeumannBC::computeQpOffDiagJacobian(unsigned jvar)
{
  unsigned m = mapVarNumber(jvar);
  return computeJacobianHelper(m);
}



Real
NSPressureNeumannBC::computeJacobianHelper(unsigned m)
{
  return _normals[_qp](_component) * _pressure_derivs.get_grad(m) * _phi[_j][_qp] * _test[_i][_qp];
}
