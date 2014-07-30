#ifndef LANGEVINNOISE_H
#define LANGEVINNOISE_H

#include "Kernel.h"

//Forward Declarations
class LangevinNoise;

template<>
InputParameters validParams<LangevinNoise>();

class LangevinNoise : public Kernel
{
public:
  LangevinNoise(const std::string & name, InputParameters parameters);

protected:
  virtual void residualSetup();
  virtual Real computeQpResidual();

  const Real _amplitude;
  const MaterialProperty<Real> * _multiplier_prop;
};

#endif //LANGEVINNOISE_H
