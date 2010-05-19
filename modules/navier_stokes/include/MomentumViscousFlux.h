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

  MomentumViscousFlux(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual void subdomainSetup();
  
protected:
  virtual Real computeQpResidual();

  int _component;

  MooseArray<RealTensorValue> * _viscous_stress_tensor;
};
 
#endif
