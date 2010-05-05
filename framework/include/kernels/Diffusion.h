#ifndef DIFFUSION_H
#define DIFFUSION_H

#include "Kernel.h"


// Forward Declaration
class Diffusion;

template<>
InputParameters validParams<Diffusion>();


class Diffusion : public Kernel
{
public:

  Diffusion(std::string name, MooseSystem & moose_system, InputParameters parameters);  

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
};
#endif //DIFFUSION_H
