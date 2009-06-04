#include "Diffusion.h"

#ifndef EXAMPLEDIFFUSION_H
#define EXAMPLEDIFFUSION_H
//Forward Declarations
class ExampleDiffusion;

class ExampleDiffusion : public Diffusion
{
public:

  ExampleDiffusion(std::string name,
                   Parameters parameters,
                   std::string var_name,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as)
    :Diffusion(name,parameters,var_name,coupled_to,coupled_as)
  {}

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
