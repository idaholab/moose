#include "EnergyInviscidFlux.h"
 
template<>
InputParameters validParams<EnergyInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

EnergyInviscidFlux::EnergyInviscidFlux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _u_vel_var(coupled("u")),
    _u_vel(coupledVal("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledVal("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : 9999999),
    _w_vel(_dim == 3 ? coupledVal("w") : _zero)
  {}

void
EnergyInviscidFlux::subdomainSetup()
  {
    _pressure = &_material->getRealProperty("pressure");
  }

Real
EnergyInviscidFlux::computeQpResidual()
{
  RealVectorValue vec(_u_vel[_qp]*(_u[_qp]+(*_pressure)[_qp]),
                      _v_vel[_qp]*(_u[_qp]+(*_pressure)[_qp]),
                      _w_vel[_qp]*(_u[_qp]+(*_pressure)[_qp]));

  return -(vec*_dtest[_i][_qp]);
}


Real
EnergyInviscidFlux::computeQpJacobian()
{
  RealVectorValue vec(_u_vel[_qp]*_phi[_j][_qp],
                      _v_vel[_qp]*_phi[_j][_qp],
                      _w_vel[_qp]*_phi[_j][_qp]);

  return -(vec*_dtest[_i][_qp]);
}

Real
EnergyInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue vec(_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]),0,0);
    return -(vec*_dtest[_i][_qp]);
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue vec(0,_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]),0);
    return -(vec*_dtest[_i][_qp]);
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]));
    return -(vec*_dtest[_i][_qp]);
  }

  return 0;
}

