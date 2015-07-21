#ifndef EULER2RGBAUX_H
#define EULER2RGBAUX_H

#include "AuxKernel.h"

class Euler2RGBAux;

template<>
InputParameters validParams<Euler2RGBAux>();

class Euler2RGBAux : public AuxKernel
{
public:
  Euler2RGBAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  unsigned int _sd;
  const MaterialProperty<Real> & _phi1;
  const MaterialProperty<Real> & _phi;
  const MaterialProperty<Real> & _phi2;
  const MaterialProperty<unsigned int> & _phase;
  const MaterialProperty<unsigned int> & _sym;
};

#endif //EULER2RGBAUX_H
