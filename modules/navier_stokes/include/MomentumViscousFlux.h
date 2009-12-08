#ifndef MOMENTUMVISCOUSFLUX_H
#define MOMENTUMVISCOUSFLUX_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class MomentumViscousFlux;

template<>
InputParameters validParams<MomentumViscousFlux>();

class MomentumViscousFlux : public Kernel
{
public:

  MomentumViscousFlux(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                      std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  virtual void subdomainSetup();
  
protected:
  virtual Real computeQpResidual();

  int _component;

  std::vector<RealTensorValue> * _viscous_stress_tensor;
};
 
#endif
