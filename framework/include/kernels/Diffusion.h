#ifndef DIFFUSION_H
#define DIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class Diffusion;

class Diffusion : public Kernel
{
public:

  Diffusion(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to=std::vector<std::string>(0),
            std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
    

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
};
#endif //DIFFUSION_H
