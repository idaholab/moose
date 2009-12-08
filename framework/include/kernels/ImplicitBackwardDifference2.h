#ifndef IMPLICITBD2
#define IMPLICITBD2

#include "Kernel.h"

class ImplicitBackwardDifference2 : public Kernel
{
public:

  ImplicitBackwardDifference2(std::string name,
                              InputParameters parameters,
                              std::string var_name,
                              std::vector<std::string> coupled_to=std::vector<std::string>(0),
                              std::vector<std::string> coupled_as=std::vector<std::string>(0));

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
};
#endif //IMPLICITBD2
