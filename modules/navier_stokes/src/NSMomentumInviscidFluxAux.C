#include "NSMomentumInviscidFluxAux.h"

template<>
InputParameters validParams<NSMomentumInviscidFluxAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Mark variables as required as necessary
  params.addRequiredCoupledVar("rhou", "");
  params.addRequiredCoupledVar("rhov", "");
  params.addCoupledVar("rhow", ""); // Only required in 3D...

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // Only required in 3D...

  params.addRequiredCoupledVar("pressure", "");
  
  // Mark parameters as required as necessary
  params.addRequiredParam<unsigned>("flux_vector_subscript", "=0,1,2, determines which inviscid flux vector, F_i, to compute");
  params.addRequiredParam<unsigned>("equation_index", "=0,1,2, determines which momentum equation we are computing in");
  
  return params;
}



NSMomentumInviscidFluxAux::NSMomentumInviscidFluxAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _rhou(coupledValue("rhou")),
   _rhov(coupledValue("rhov")),
   _rhow(_dim == 3 ? coupledValue("rhow") : _zero),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pressure(coupledValue("pressure")),
   _flux_vector_subscript(getParam<unsigned>("flux_vector_subscript")),
   _equation_index(getParam<unsigned>("equation_index"))
{}




Real
NSMomentumInviscidFluxAux::computeValue()
{
  RealVectorValue mom_vec(_rhou[_qp], _rhov[_qp], _rhow[_qp]);
  RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  
  // (rho * u_i) * u_j + \delta_{ij} * P 
  // where: 
  // i==_flux_vector_subscript, 
  // j==_equation_index
  return mom_vec(_flux_vector_subscript) * vec(_equation_index) + (_flux_vector_subscript == _equation_index ? _pressure[_qp] : 0.);
}
