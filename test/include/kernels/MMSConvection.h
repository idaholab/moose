#ifndef MMSCONVECTION_H_
#define MMSCONVECTION_H_

#include "Kernel.h"

class MMSConvection;

template<>
InputParameters validParams<MMSConvection>();

class MMSConvection : public Kernel
{
public:
  MMSConvection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue velocity;

  Real _x;
  Real _y;
  Real _z;

};

#endif //MMSCONVECTION_H_
