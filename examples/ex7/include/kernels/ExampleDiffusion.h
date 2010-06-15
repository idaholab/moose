#include "Diffusion.h"

#ifndef EXAMPLEDIFFUSION_H
#define EXAMPLEDIFFUSION_H
//Forward Declarations
class ExampleDiffusion;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<ExampleDiffusion>();

class ExampleDiffusion : public Diffusion
{
public:

  ExampleDiffusion(std::string name,
                   MooseSystem &sys,
                   InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  MooseArray<Real> & _diffusivity;
};
#endif //EXAMPLEDIFFUSION_H
