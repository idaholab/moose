#include "Kernel.h"

#ifndef POLYCONVECTION_H
#define POLYCONVECTION_H

class PolyConvection;

template<>
InputParameters validParams<PolyConvection>();

class PolyConvection : public Kernel
{
public:
  
 PolyConvection(const std::string & name,
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
#endif //POLYCONVECTION_H
