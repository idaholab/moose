#include "Temperature.h"
 

template<>
InputParameters validParams<Temperature>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

Temperature::Temperature(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
    _p_var(coupled("p")),
    _p(coupledValue("p")),
    _pe_var(coupled("pe")),
    _pe(coupledValue("pe")),
    _u_vel_var(coupled("u")),
    _u_vel(coupledValue("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledValue("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : 0),
    _w_vel(_dim == 3 ? coupledValue("w") : _zero),
    _c_v(getMaterialProperty<Real>("c_v"))
{}

Real
Temperature::computeQpResidual()
{
  Real value = 1.0/_c_v[_qp];

  Real et = _pe[_qp]/_p[_qp];

  RealVectorValue vec(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  value *= et - ((vec * vec) / 2.0);

  return (_u[_qp]-value)*_test[_i][_qp];
}


Real
Temperature::computeQpJacobian()
{
  Real value = 1.0/_c_v[_qp];

  Real et = _pe[_qp]/_p[_qp];

  RealVectorValue vec(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  value *= et - ((vec * vec) / 2.0);
  
  return _phi[_j][_qp]*_test[_i][_qp];
}

Real
Temperature::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _p_var)
  {
    Real value = 1.0/_c_v[_qp];

    Real et = (_pe[_qp]/(-_p[_qp]*_p[_qp]))*_phi[_j][_qp];

    value *= et;

    return (-value)*_test[_i][_qp];    
  }
  else if(jvar == _pe_var)
  {
    Real value = 1.0/_c_v[_qp];

    Real et = _phi[_j][_qp]/_p[_qp];

    value *= et;

    return (-value)*_test[_i][_qp];    
  }
  
  return 0;
}

