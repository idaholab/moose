#ifndef IMPLICITEULER
#define IMPLICITEULER

#include "Kernel.h"

class ImplicitEuler : public Kernel
{
public:

  ImplicitEuler(std::string name,
                InputParameters parameters,
                std::string var_name,
                std::vector<std::string> coupled_to=std::vector<std::string>(0),
                std::vector<std::string> coupled_as=std::vector<std::string>(0));

protected:
  virtual Real computeQpResidual();
  

  virtual Real computeQpJacobian();

};
#endif //IMPLICITEULER
