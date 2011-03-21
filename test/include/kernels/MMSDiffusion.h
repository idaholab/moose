#ifndef MMSDIFFUSION_H_
#define MMSDIFFUSION_H_

#include "Kernel.h"

class MMSDiffusion;

template<>
InputParameters validParams<MMSDiffusion>();


class MMSDiffusion : public Kernel
{
public:
  MMSDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif //MMSDIFFUSION_H
