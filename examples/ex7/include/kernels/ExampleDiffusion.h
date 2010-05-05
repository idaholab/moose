#include "Diffusion.h"

#ifndef EXAMPLEDIFFUSION_H
#define EXAMPLEDIFFUSION_H
//Forward Declarations
class ExampleDiffusion;

class ExampleDiffusion : public Diffusion
{
public:

  ExampleDiffusion(std::string name,
                   MooseSystem &sys,
                   InputParameters parameters);

  // subdomainSetup() gets called each time the subdomain (block) changes.
  // This is where you grab material properties for the current block
  // You can also do other calculations in here that apply to a whole
  // subdomain.
  virtual void subdomainSetup();  
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  std::vector<Real> * _diffusivity;
};
#endif //EXAMPLEDIFFUSION_H
