#include "ImplicitBackwardDifference2.h"

template<>
InputParameters validParams<ImplicitBackwardDifference2>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("start_with_be", true, "Whether or not to use first order Backward Euler for the first timestep");
  return params;
}

ImplicitBackwardDifference2::ImplicitBackwardDifference2(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _start_with_be(parameters.get<bool>("start_with_be"))
{ _t_scheme = 1;}

Real
ImplicitBackwardDifference2::computeQpResidual()
{
  if(_t_step==1 && _start_with_be) // First step, use BE
    return _test[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
  else
    return _test[_i][_qp]*((_bdf2_wei[2]*_u[_qp]+_bdf2_wei[1]*_u_old[_qp]+_bdf2_wei[0]*_u_older[_qp])/_dt);
}

Real
ImplicitBackwardDifference2::computeQpJacobian()
{
  
  if(_t_step==1 && _start_with_be) // First step, use BE
    return _test[_i][_qp]*_phi[_j][_qp]/_dt;
  else
    return _test[_i][_qp]*_phi[_j][_qp]*_bdf2_wei[2]/_dt;
}


