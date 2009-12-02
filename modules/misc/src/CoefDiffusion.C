#include "CoefDiffusion.h"


template<>
InputParameters valid_params<CoefDiffusion>()
{
  InputParameters params;
  params.set<Real>("coef")=0.0;
  return params;
}

CoefDiffusion::CoefDiffusion(std::string name,
                             InputParameters parameters,
                             std::string var_name,
                             std::vector<std::string> coupled_to,
                             std::vector<std::string> coupled_as)
  :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
   _coef(_parameters.get<Real>("coef"))
{}

Real
CoefDiffusion::computeQpResidual()
{
  return _coef*_dphi[_i][_qp]*_grad_u[_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  return _coef*_dphi[_i][_qp]*_dphi[_j][_qp];
}
