#include "EnergyViscousFlux.h"
 

template<>
InputParameters validParams<EnergyViscousFlux>()
{
  InputParameters params = validParams<Kernel>();
  
  // Declare some required coupled variables
  // Velocities likely to be AuxVariables, but it doesn't matter,
  // they can be coupled just the same...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("temperature", "");

  return params;
}

EnergyViscousFlux::EnergyViscousFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel_var(coupled("u")),
   _u_vel(coupledValue("u")),
   _v_vel_var(coupled("v")),
   _v_vel(coupledValue("v")),
   _w_vel_var(_dim == 3 ? coupled("w") : std::numeric_limits<unsigned int>::max()),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _temp_var(coupled("temperature")),
   _grad_temp(coupledGradient("temperature")),
   _temp(coupledValue("temperature")),
   _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))//,
   //_temperature(getMaterialProperty<Real>("temperature"))
  {}




Real
EnergyViscousFlux::computeQpResidual()
{
  // (k*grad(T) + tau * u) * grad(phi)
  RealVectorValue velocity(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  
  // Debugging, don't really need T just grad(T)
  //std::cout << "_u_vel=" << _u_vel[_qp] << std::endl;
  //std::cout << "_temp=" << _temp[_qp] << std::endl;

  // vec = grad(T)
  RealVectorValue vec = _grad_temp[_qp];
  //std::cout << vec << std::endl;

  // vec = k*grad(T)
  vec *= _thermal_conductivity[_qp];

  // vec = tau*u + k*grad(T)
  vec += _viscous_stress_tensor[_qp] * velocity;

  return vec * _grad_test[_i][_qp];
}




Real
EnergyViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue velocity(_phi[_j][_qp],0,0);
    return ( _viscous_stress_tensor[_qp] * velocity ) *_grad_test[_i][_qp];
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue velocity(0,_phi[_j][_qp],0);
    return ( _viscous_stress_tensor[_qp] * velocity ) *_grad_test[_i][_qp];
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue velocity(0,0,_phi[_j][_qp]);
    return ( _viscous_stress_tensor[_qp] * velocity ) *_grad_test[_i][_qp];
  }
  else if(jvar == _temp_var)
  {
    RealVectorValue vec = _grad_phi[_j][_qp];

    // -(-k*grad_T)
    vec *= _thermal_conductivity[_qp];

    return vec*_grad_test[_i][_qp];
  }

  return 0;
}
