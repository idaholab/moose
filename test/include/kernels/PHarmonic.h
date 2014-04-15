#ifndef PHARMONIC_H
#define PHARMONIC_H

#include "Kernel.h"

//Forward Declarations
class PHarmonic;

template<>
InputParameters validParams<PHarmonic>();

/**
 * This kernel implements (grad(v), |grad(u)|^(p-2) grad(u)), where u is the solution
 * and v is the test function. When p=2, this kernel is equivalent with Diffusion.
 */

class PHarmonic : public Kernel
{
public:

  PHarmonic(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif //PHARMONIC_H
