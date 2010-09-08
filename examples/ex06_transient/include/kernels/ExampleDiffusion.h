#ifndef EXAMPLEDIFFUSION_H
#define EXAMPLEDIFFUSION_H

#include "Diffusion.h"

//Forward Declarations
class ExampleDiffusion;

/* This class extends the Diffusion kernel to multiply by a coefficient
 * read from the input file
 */
template<>
InputParameters validParams<ExampleDiffusion>();

class ExampleDiffusion : public Diffusion
{
public:

  ExampleDiffusion(const std::string & name,
                   MooseSystem &sys,
                   InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _diffusivity;
};
#endif //EXAMPLEDIFFUSION_H
