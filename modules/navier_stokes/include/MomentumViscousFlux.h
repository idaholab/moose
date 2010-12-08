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

  MomentumViscousFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  int _component;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
};
 
#endif
