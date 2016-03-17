/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSTemperatureL2.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSTemperatureL2>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("pe", "");
  params.addRequiredCoupledVar("p", "");
  return params;
}

NSTemperatureL2::NSTemperatureL2(const InputParameters & parameters) :
    Kernel(parameters),
    _p_var(coupled("p")),
    _p(coupledValue("p")),
    _pe_var(coupled("pe")),
    _pe(coupledValue("pe")),
    _u_vel_var(coupled("u")),
    _u_vel(coupledValue("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledValue("v")),
    _w_vel_var(_mesh.dimension() == 3 ? coupled("w") : 0),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _c_v(getMaterialProperty<Real>("c_v"))
{
}

Real
NSTemperatureL2::computeQpResidual()
{
  Real value = 1.0 / _c_v[_qp];

  const Real et = _pe[_qp] / _p[_qp];
  const RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  value *= et - ((vec * vec) / 2.0);

  // L2-projection
  return (_u[_qp] - value) * _test[_i][_qp];
}

Real
NSTemperatureL2::computeQpJacobian()
{
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
NSTemperatureL2::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _p_var)
  {
    const Real et = (_pe[_qp]/(-_p[_qp]*_p[_qp]))*_phi[_j][_qp];
    Real value = et / _c_v[_qp];

    return -value * _test[_i][_qp];
  }
  else if (jvar == _pe_var)
  {
    const Real et = _phi[_j][_qp]/_p[_qp];
    Real value = et / _c_v[_qp];

    return -value * _test[_i][_qp];
  }

  return 0.0;
}
