#include "Diffusion.h"

Diffusion::Diffusion(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
  {}

Real
Diffusion::computeQpResidual()
  {
    return _dphi[_i][_qp]*_grad_u[_qp];
  }

Real
Diffusion::computeQpJacobian()
  {
    return _dphi[_i][_qp]*_dphi[_j][_qp];
  }
