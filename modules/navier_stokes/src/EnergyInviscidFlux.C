#include "EnergyInviscidFlux.h"
 
template<>
InputParameters validParams<EnergyInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");
  params.addRequiredCoupledVar("pressure", ""); // Now computed as an AuxKernel
  return params;
}

EnergyInviscidFlux::EnergyInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel_var(coupled("u")),
   _u_vel(coupledValue("u")),
   _v_vel_var(coupled("v")),
   _v_vel(coupledValue("v")),
   _w_vel_var(_dim == 3 ? coupled("w") : std::numeric_limits<unsigned int>::max()),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pressure_var(coupled("pressure")),
   _pressure(coupledValue("pressure"))
   //_pressure(getMaterialProperty<Real>("pressure")) // now an Aux var
{}




Real
EnergyInviscidFlux::computeQpResidual()
{
  // H = total enthalpy = E + P/rho
  // => rho * U * H = rho * U ( E + P/rho)
  //                =       U ( rho*E + P)
//  RealVectorValue vec(_u_vel[_qp]*(_u[_qp] + _pressure[_qp]),
//                      _v_vel[_qp]*(_u[_qp] + _pressure[_qp]),
//                      _w_vel[_qp]*(_u[_qp] + _pressure[_qp]));

  // U
  RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // ( rho*E + P ) * U
  vec *= (_u[_qp] + _pressure[_qp]);

  // -( rho*E + P ) * U * grad(phi)
  return -(vec*_grad_test[_i][_qp]);
}




Real
EnergyInviscidFlux::computeQpJacobian()
{
  // phi_j * (U * grad(phi))
  RealVectorValue vec(_u_vel[_qp]*_phi[_j][_qp],
                      _v_vel[_qp]*_phi[_j][_qp],
                      _w_vel[_qp]*_phi[_j][_qp]);

  return -(vec*_grad_test[_i][_qp]);
}




Real
EnergyInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue vec(_phi[_j][_qp]*(_u[_qp]+_pressure[_qp]),0,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue vec(0,_phi[_j][_qp]*(_u[_qp]+_pressure[_qp]),0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]*(_u[_qp]+_pressure[_qp]));
    return -(vec*_grad_test[_i][_qp]);
  }

  return 0;
}

