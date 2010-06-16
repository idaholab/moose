#include "Convection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();

  // Define our coupling parameter ( in this case it is required )
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection Kernel");
    
  return params;
}

Convection::Convection(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)

  // You must call the constructor of the base class first
  // The "true" here specifies that this Kernel is to be integrated
  // over the domain.
  :Kernel(name, sys, parameters),

   // coupledGrad will give us a reference to the gradient of another
   // variable in the computation.  We are going to use that gradient
   // as our velocity vector.
   //
   // Note that "velocity_vector" is the name this Kernel expects... ie
   // what should appear on the lhs of the assignment operation in the input file
   // 
   // You can also use coupledVal() and coupledValOld() if you want
   // values
   _velocity_vector(coupledGrad("velocity_vector"))
{
}

Real Convection::computeQpResidual()
{
  // _grad_some_var[_qp] * _grad_u[_qp] is actually doing a dot product
  return _phi[_i][_qp]*(_velocity_vector[_qp]*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _dphi[_j]
  return _phi[_i][_qp]*(_velocity_vector[_qp]*_dphi[_j][_qp]);
}
