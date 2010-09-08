#ifndef MMSDIFFUSION_H
#define MMSDIFFUSION_H

#include "Kernel.h"

class MMSDiffusion;

template<>
InputParameters validParams<MMSDiffusion>();


class MMSDiffusion : public Kernel
{
public:

  MMSDiffusion(const std::string & name, MooseSystem & moose_system, InputParameters parameters);  

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
};
#endif //MMSDIFFUSION_H
