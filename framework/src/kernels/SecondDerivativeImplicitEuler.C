#include "SecondDerivativeImplicitEuler.h"
 

template<>
InputParameters validParams<SecondDerivativeImplicitEuler>()
{
  InputParameters params;
  return params;
}

SecondDerivativeImplicitEuler::SecondDerivativeImplicitEuler(std::string name,
                                InputParameters parameters,
                                std::string var_name,
                                std::vector<std::string> coupled_to,
                                std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
  {}

Real
SecondDerivativeImplicitEuler::computeQpResidual()
  {
    return _phi[_i][_qp]*((_u[_qp]-2*_u_old[_qp]+_u_older[_qp])/(_dt*_dt));
  }

Real
SecondDerivativeImplicitEuler::computeQpJacobian()
  {
    return _phi[_i][_qp]*(_phi[_j][_qp]/(_dt*_dt));
  }
