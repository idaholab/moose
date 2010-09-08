#ifndef POLYDIFFUSION_H
#define POLYDIFFUSION_H

#include "Kernel.h"

class PolyDiffusion;

template<>
InputParameters validParams<PolyDiffusion>();


class PolyDiffusion : public Kernel
{
public:

  PolyDiffusion(const std::string & name, MooseSystem & moose_system, InputParameters parameters);  

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
};
#endif //POLYDIFFUSION_H
