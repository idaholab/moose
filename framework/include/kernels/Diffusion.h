#ifndef DIFFUSION_H
#define DIFFUSION_H

#include "Kernel.h"

class Diffusion;

template<>
InputParameters validParams<Diffusion>();


class Diffusion : public Kernel
{
public:
  Diffusion(const std::string & name, InputParameters parameters);
  virtual ~Diffusion();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};


#endif /* DIFFUSION_H_ */
