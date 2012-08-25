#ifndef GENERICDIFFUSION_H
#define GENERICDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class GenericDiffusion;

template<>
InputParameters validParams<GenericDiffusion>();

class GenericDiffusion : public Kernel
{
public:

  GenericDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  MaterialProperty<Real> & _diffusivity;
};

#endif //GENERICDIFFUSION_H
