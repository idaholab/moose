#ifndef AC_H
#define AC_H

#include "Kernel.h"

//Forward Declarations
class AC;

template<>
InputParameters validParams<AC>();

class AC : public Kernel
{
public:

  AC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeDFDOP(PFFunctionType type);
  

private:
  
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _L;
  
};
#endif //AC_H
