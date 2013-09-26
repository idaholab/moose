#ifndef RESTARTDIFFUSION_H
#define RESTARTDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class RestartDiffusion;

template<>
InputParameters validParams<RestartDiffusion>();

class RestartDiffusion : public Kernel
{
public:

  RestartDiffusion(const std::string & name, InputParameters parameters);

  virtual void timestepSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
  Real & _current_coef;
};

#endif //RESTARTDIFFUSION_H
