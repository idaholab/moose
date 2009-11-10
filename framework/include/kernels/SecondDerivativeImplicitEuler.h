#ifndef SECONDDERIVATIVEIMPLICITEULER_H
#define SECONDDERIVATIVEIMPLICITEULER_H

#include "Kernel.h"


//Forward Declarations
class SecondDerivativeImplicitEuler;

template<>
InputParameters valid_params<SecondDerivativeImplicitEuler>();

class SecondDerivativeImplicitEuler : public Kernel
{
public:

  SecondDerivativeImplicitEuler(std::string name,
                                InputParameters parameters,
                                std::string var_name,
                                std::vector<std::string> coupled_to=std::vector<std::string>(0),
                                std::vector<std::string> coupled_as=std::vector<std::string>(0));

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

};
#endif //SECONDDERIVATIVEIMPLICITEULER_H
