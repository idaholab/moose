#include "MassInviscidFlux.h"

template<>
InputParameters validParams<MassInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MassInviscidFlux::MassInviscidFlux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _pu_var(coupled("pu")),
    _pu(coupledVal("pu")),
    _pv_var(coupled("pv")),
    _pv(coupledVal("pv")),
    _pw_var(_dim == 3 ? coupled("pw") : 999999),
    _pw(_dim == 3 ? coupledVal("pw") : _zero)
  {}

Real
MassInviscidFlux::computeQpResidual()
{
  RealVectorValue vec(_pu[_qp],_pv[_qp],_pw[_qp]);

  return -(vec*_grad_test[_i][_qp]);
}

Real
MassInviscidFlux::computeQpJacobian()
{
  //Essentially a vector of the velocities
  RealVectorValue vec(_pu[_qp]/_u[_qp],_pv[_qp]/_u[_qp],_pw[_qp]/_u[_qp]);

  return -(_phi[_j][_qp]*vec*_grad_test[_i][_qp]);
}

Real
MassInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _pu_var)
  {
    RealVectorValue vec(_phi[_j][_qp],0,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _pv_var)
  {
    RealVectorValue vec(0,_phi[_j][_qp],0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _pw_var)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]);
    return -(vec*_grad_test[_i][_qp]);
  }

  return 0;
}
