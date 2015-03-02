/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryConvection.h"
#include "Material.h"

template<>
InputParameters validParams<PrimaryConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("p", "Pressure");
  return params;
}

PrimaryConvection::PrimaryConvection(const std::string & name, InputParameters parameters)

  // You must call the constructor of the base class first
  // The "true" here specifies that this Kernel is to be integrated
  // over the domain.
  :Kernel(name,parameters),
   _cond(getMaterialProperty<Real>("conductivity")),
   // coupledGradient will give us a reference to the gradient of another
   // variable in the computation.  We are going to use that gradient
   // as our velocity vector.
   //
   // Note that "some_var" is the name this Kernel expects... ie
   // what should be in "coupled_as"
   //
   // You can also use coupledValue() and coupledValueOld() if you want
   // values
   _grad_p(coupledGradient("p"))
{
}

Real PrimaryConvection::computeQpResidual()
{
  // _grad_p[_qp] * _grad_u[_qp] is actually doing a dot product
  RealGradient _Darcy_vel=-_grad_p[_qp]*_cond[_qp];

//  Moose::out << "Pore velocity " << _Darcy_vel(0) << std::endl;

  return _test[_i][_qp]*(_Darcy_vel*_grad_u[_qp]);
}

Real PrimaryConvection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _dphi[_j]
  RealGradient _Darcy_vel=-_grad_p[_qp]*_cond[_qp];

  return _test[_i][_qp]*(_Darcy_vel*_grad_phi[_j][_qp]);
}

Real PrimaryConvection::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}
