#include "EnergyInviscidFlux.h"
 

template<>
Parameters valid_params<EnergyInviscidFlux>()
{
  Parameters params;
  return params;
}

EnergyInviscidFlux::EnergyInviscidFlux(std::string name,
                  Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
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

  return -(vec*_dphi[_i][_qp]);
}


Real
EnergyInviscidFlux::computeQpJacobian()
{
  RealVectorValue vec(_u_vel[_qp]*_phi[_j][_qp],
                      _v_vel[_qp]*_phi[_j][_qp],
                      _w_vel[_qp]*_phi[_j][_qp]);

  return -(vec*_dphi[_i][_qp]);
}

Real
EnergyInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue vec(_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]),0,0);
    return -(vec*_dphi[_i][_qp]);
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue vec(0,_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]),0);
    return -(vec*_dphi[_i][_qp]);
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]*(_u[_qp]+(*_pressure)[_qp]));
    return -(vec*_dphi[_i][_qp]);
  }

  return 0;
}

