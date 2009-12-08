#include "EnergyViscousFlux.h"
 

template<>
InputParameters validParams<EnergyViscousFlux>()
{
  InputParameters params;
  return params;
}
EnergyViscousFlux::EnergyViscousFlux(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _u_vel_var(coupled("u")),
    _u_vel(coupledVal("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledVal("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : 999999),
    _w_vel(_dim == 3 ? coupledVal("w") : _zero),
    _temp_var(coupled("temp")),
    _grad_temp(coupledGrad("temp"))
  {}

void
EnergyViscousFlux::subdomainSetup()
  {
    _viscous_stress_tensor = &_material->getTensorProperty("viscous_stress_tensor");
    _thermal_conductivity  = &_material->getRealProperty("thermal_conductivity");
  }

Real
EnergyViscousFlux::computeQpResidual()
{
  RealVectorValue velocity(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);
  
  RealVectorValue vec = _grad_temp[_qp];

  // -(-k*grad_T)
  vec *= (*_thermal_conductivity)[_qp];

  // Tau dot velocity
  vec += (*_viscous_stress_tensor)[_qp] * velocity;

  return vec*_dphi[_i][_qp];
}

Real
EnergyViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue velocity(_phi[_j][_qp],0,0);
    return ( (*_viscous_stress_tensor)[_qp] * velocity ) *_dphi[_i][_qp];
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue velocity(0,_phi[_j][_qp],0);
    return ( (*_viscous_stress_tensor)[_qp] * velocity ) *_dphi[_i][_qp];
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue velocity(0,0,_phi[_j][_qp]);
    return ( (*_viscous_stress_tensor)[_qp] * velocity ) *_dphi[_i][_qp];
  }
  else if(jvar == _temp_var)
  {
    RealVectorValue vec = _dphi[_j][_qp];

    // -(-k*grad_T)
    vec *= (*_thermal_conductivity)[_qp];

    return vec*_dphi[_i][_qp];
  }

  return 0;
}
