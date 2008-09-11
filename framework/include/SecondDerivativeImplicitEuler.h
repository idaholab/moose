#include "Kernel.h"

#ifndef SECONDDERIVATIVEIMPLICITEULER_H
#define SECONDDERIVATIVEIMPLICITEULER_H

//Forward Declarations
class SecondDerivativeImplicitEuler;

template<>
Parameters valid_params<SecondDerivativeImplicitEuler>();

class SecondDerivativeImplicitEuler : public Kernel
{
public:

  SecondDerivativeImplicitEuler(Parameters parameters,
                 std::string var_name,
                 std::vector<std::string> coupled_to=std::vector<std::string>(0),
                 std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as)
  {}

protected:
  virtual Real computeQpResidual()
  {
    return _phi[_i][_qp]*((_u[_qp]-2*_u_old[_qp]+_u_older[_qp])/(_dt*_dt));
  }

  virtual Real computeQpJacobian()
  {
    return _phi[_i][_qp]*(_phi[_j][_qp]/(_dt*_dt));
  }

};
#endif //SECONDDERIVATIVEIMPLICITEULER_H
