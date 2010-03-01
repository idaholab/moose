#include "Reaction.h"

Reaction::Reaction(std::string name,
           InputParameters parameters,
           std::string var_name,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
  {}

Real
Reaction::computeQpResidual()
  {
    return _test[_i][_qp]*_u[_qp];
  }

Real
Reaction::computeQpJacobian()
  {
    return _test[_i][_qp]*_phi[_j][_qp];
  }
