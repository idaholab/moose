#include "ImplicitBackwardDifference2.h"

template<>
InputParameters validParams<ImplicitBackwardDifference2>()
{
  InputParameters params;
  return params;
}

ImplicitBackwardDifference2::ImplicitBackwardDifference2(std::string name,
                              InputParameters parameters,
                              std::string var_name,
                              std::vector<std::string> coupled_to,
                              std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
{ _t_scheme = 1;}

Real
ImplicitBackwardDifference2::computeQpResidual()
{
  if(_t_step==1) // First step, use BE
    return _phi[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
  else
    return _phi[_i][_qp]*((_bdf2_wei[2]*_u[_qp]+_bdf2_wei[1]*_u_old[_qp]+_bdf2_wei[0]*_u_older[_qp])/_dt);
}

Real
ImplicitBackwardDifference2::computeQpJacobian()
{
  
  if(_t_step==1) // First step, use BE
    return _phi[_i][_qp]*_phi[_j][_qp]/_dt;
  else
    return _phi[_i][_qp]*_phi[_j][_qp]*_bdf2_wei[2]/_dt;
}


