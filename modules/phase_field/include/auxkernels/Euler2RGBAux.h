#ifndef EULER2RGBAUX_H
#define EULER2RGBAUX_H

#include "AuxKernel.h"

class Euler2RGBAux;

template<>
InputParameters validParams<Euler2RGBAux>();

class Euler2RGBAux : public AuxKernel
{
public:
  Euler2RGBAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

private:
  unsigned int _sd;
  MaterialProperty<Real> & _phi1;
  MaterialProperty<Real> & _phi;
  MaterialProperty<Real> & _phi2;
  MaterialProperty<unsigned int> & _phase;
  MaterialProperty<unsigned int> & _sym;
};

#endif //EULER2RGBAUX_H
