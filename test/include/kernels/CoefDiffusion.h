#ifndef COEFDIFFUSION_H
#define COEFDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class CoefDiffusion;

template<>
InputParameters validParams<CoefDiffusion>();

class CoefDiffusion : public Kernel
{
public:

  CoefDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  Real _coef;
};
#endif //COEFDIFFUSION_H
