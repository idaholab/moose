#include "Convection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<Convection>()
{
  InputParameters params;
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

Convection::Convection(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
  // You must call the constructor of the base class first
  // The "true" here specifies that this Kernel is to be integrated
  // over the domain.
  :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),

   // This is the "Intialization List" it sets the values of class variables
   // Here we are grabbing the values of Parameters to use for a velocity vector
   _x(_parameters.get<Real>("x")),
   _y(_parameters.get<Real>("y")),
   _z(_parameters.get<Real>("z"))
{
  // Build a velocity vector to use in the residual / jacobian computations.
  // We do this here so that it's only done once and then we just reuse it.
  // Note that RealVectorValues ALWAYS have 3 components... even when running in
  // 2D or 1D.  This makes the code simpler...
  velocity(0)=_x;
  velocity(1)=_y;
  velocity(2)=_z;
}

Real Convection::computeQpResidual()
{
  /*
  std::cout<<"phi : "<<_phi[_i][_qp]<<std::endl;
  std::cout<<"test: "<<_test[_i][_qp]<<std::endl;
  std::cout<<"res : "<<(velocity*_grad_u[_qp])<<std::endl;
  std::cout<<"all : "<<_test[_i][_qp]*(velocity*_grad_u[_qp])<<std::endl;
  */

  // velocity * _grad_u[_qp] is actually doing a dot product
  return _test[_i][_qp]*(velocity*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _dphi[_j]
  return _test[_i][_qp]*(velocity*_dphi[_j][_qp]);
}
