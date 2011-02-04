#ifndef VELOCITYGRADIENTAUX_H
#define VELOCITYGRADIENTAUX_H

#include "AuxKernel.h"

class VelocityGradientAux;

template<>
InputParameters validParams<VelocityGradientAux>();


class VelocityGradientAux : public AuxKernel
{
public:
  VelocityGradientAux( const std::string & name, InputParameters parameters );

  virtual ~VelocityGradientAux() {}

protected:
  virtual Real computeValue();

  const unsigned _index;
  MaterialProperty<RealTensorValue> & _total_strain;

};

#endif // PLASTICSTRAINAUX_H
