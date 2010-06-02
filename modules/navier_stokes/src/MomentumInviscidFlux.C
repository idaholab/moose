#include "MomentumInviscidFlux.h"
 

template<>
InputParameters validParams<MomentumInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("component") = -1;
  return params;
}

MomentumInviscidFlux::MomentumInviscidFlux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _u_vel_var(coupled("u")),
   _u_vel(coupledVal("u")),
   _v_vel_var(coupled("v")),
   _v_vel(coupledVal("v")),
   _w_vel_var(_dim == 3 ? coupled("w") : 0),
   _w_vel(_dim == 3 ? coupledVal("w") : _zero),
   _component(parameters.get<Real>("component")),
   _pressure(getRealMaterialProperty("pressure"))
{
  if(_component < 0)
  {
    std::cout<<"Must select a component for MomentumInviscidFlux"<<std::endl;
    libmesh_error();
  }
}

Real
MomentumInviscidFlux::computeQpResidual()
{
  RealVectorValue vec(_u[_qp]*_u_vel[_qp],_u[_qp]*_v_vel[_qp],_u[_qp]*_w_vel[_qp]);

  vec(_component) += _pressure[_qp];

  return -(vec*_dtest[_i][_qp]);
}

Real
MomentumInviscidFlux::computeQpJacobian()
{
  RealVectorValue vec(_phi[_j][_qp]*_u_vel[_qp],_phi[_j][_qp]*_v_vel[_qp],_phi[_j][_qp]*_w_vel[_qp]);

  return -(vec*_dtest[_i][_qp]);
}

Real
MomentumInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _u_vel_var)
  {
    RealVectorValue vec(_u[_qp]*_phi[_j][_qp],0,0);
    return -(vec*_dtest[_i][_qp]);
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue vec(0,_u[_qp]*_phi[_j][_qp],0);
    return -(vec*_dtest[_i][_qp]);
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue vec(0,0,_u[_qp]*_phi[_j][_qp]);
    return -(vec*_dtest[_i][_qp]);
  }

  return 0;
}
