#ifndef POLYCONVECTION_H_
#define POLYCONVECTION_H_

#include "Kernel.h"

class PolyConvection;

template<>
InputParameters validParams<PolyConvection>();

class PolyConvection : public Kernel
{
public:
  PolyConvection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue velocity;

  Real _x;
  Real _y;
  Real _z;

};

#endif //POLYCONVECTION_H_
