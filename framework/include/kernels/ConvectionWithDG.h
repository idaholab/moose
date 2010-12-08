#ifndef CONVECTIONWITHDG_H
#define CONVECTIONWITHDG_H

#include "Kernel.h"

// Forward Declaration
class ConvectionWithDG;


template<>
InputParameters validParams<ConvectionWithDG>();

class ConvectionWithDG : public Kernel
{
public:

  ConvectionWithDG(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
  
};
#endif // CONVECTIONWITHDG_H
