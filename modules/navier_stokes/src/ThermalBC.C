#include "ThermalBC.h"

template<>
InputParameters validParams<ThermalBC>()
{
  InputParameters params = validParams<NodalBC>();

  // Declare some required coupled variables
  params.addRequiredCoupledVar("p", "");

  // Velocities likely to be AuxVariables, but it doesn't matter,
  // they can be coupled just the same...
  //params.addRequiredCoupledVar("u", "");
  //params.addRequiredCoupledVar("v", "");
  //params.addCoupledVar("w", "");
  params.addRequiredCoupledVar("c_v", ""); // This is now an aux variable, it's needed in the BC

  params.addRequiredParam<Real>("initial", "Initial temperature");
  params.addRequiredParam<Real>("final", "Final temperature");
  params.addRequiredParam<Real>("duration", "Time over which temperature ramps up from initial to final");

  return params;
}

ThermalBC::ThermalBC(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters),
   _p_var(coupled("p")),
   _p(coupledValue("p")),
   //_u_vel_var(coupled("u")),
   //_u_vel(coupledValue("u")),
   //_v_vel_var(coupled("v")),
   //_v_vel(coupledValue("v")),
   //_w_vel_var(_dim == 3 ? coupled("w") : 0),
   //_w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _c_v_var(coupled("c_v")),
   _c_v(coupledValue("c_v")),
   _initial(getParam<Real>("initial")),
   _final(getParam<Real>("final")),
   _duration(getParam<Real>("duration"))//,
   //_gamma(getMaterialProperty<Real>("gamma")),
   //_R(getMaterialProperty<Real>("R"))//,
   //_c_v(getMaterialProperty<Real>("c_v"))
  {}




//Real
//ThermalBC::temperature()
//{
//  Real value = 1.0/_c_v[_qp];
//
//  Real et = _u[_qp]/_p[_qp];
//
//  RealVectorValue vec(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);
//
//  // multiply by internal energy = (total energy - |vec|^2/2)
//  value *= et - ((vec * vec) / 2.0);
//
//  return value;
//}
  



Real
ThermalBC::computeQpResidual()
{
  Real value;

  // For constant temperature, set _initial = _final, or set _duration=0 and set _final.
  //  
  // T(t) = T_i + (T_f - T_i) * sin (pi/2 * t/t_d)
  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5* libMesh::pi)  * _t/_duration);
  else
    value = _final;

  // For the total energy, the essential BC is: 
  // rho*E = rho*(c_v*T + |u|^2/2)
  //
  // or, in residual form, (In general, this BC is coupled to the velocity variables.)
  // rho*E - rho*(c_v*T + |u|^2/2) = 0
  //
  // ***at a no-slip wall*** this further reduces to (no coupling to velocity variables):
  // rho*E - rho*cv*T = 0

// std::ios_base::fmtflags flags = std::cout.flags();  
// std::cout << std::scientific << std::setprecision(16);
// std::cout << "rho*E                        =" << _u[_qp] << std::endl;
// std::cout << "(_p[_qp] * _c_v[_qp] * value)=" << (_p[_qp] * _c_v[_qp] * value) << std::endl;
// //std::cout << "_c_v[_qp]                    =" << _c_v[_qp] << std::endl;
// std::cout.flags(flags);

  return _u[_qp] - (_p[_qp] * _c_v[_qp] * value);
}
