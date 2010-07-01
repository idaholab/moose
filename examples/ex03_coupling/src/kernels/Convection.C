#include "Convection.h"

template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();
  /**
   * Define our coupling parameter through validParams so it
   * will be read from the input file by the Parser.  This
   * version of the function "addRequiredCoupledVar" makes the
   * coupling required so you can rely on it being available.
   */
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection Kernel");
  return params;
}

Convection::Convection(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
 :Kernel(name, sys, parameters),
  _velocity_vector(coupledGradient("velocity_vector"))     // <- Initialize the class member with 
                                                       //    a reference to the coupled variable
                                                       //    by name from the input file
{}

Real Convection::computeQpResidual()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_phi[_j][_qp]);
}
