#include "Kernel.h"

#ifndef MMSCONVECTION_H
#define MMSCONVECTION_H

class MMSConvection;

template<>
InputParameters validParams<MMSConvection>();

class MMSConvection : public Kernel
{
public:
  
 MMSConvection(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

private:

  RealVectorValue velocity;

  Real _x;
  Real _y;
  Real _z;

};
#endif //MMSCONVECTION_H
