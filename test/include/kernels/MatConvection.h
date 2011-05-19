#ifndef MATCONVECTION_H
#define MATCONVECTION_H

#include "Kernel.h"

// Forward Declaration
class MatConvection;


template<>
InputParameters validParams<MatConvection>();

class MatConvection : public Kernel
{
public:
  MatConvection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  std::string _conv_prop_name;
  MaterialProperty<Real> & _conv_prop;

  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
  
};

#endif // MATCONVECTION_H
