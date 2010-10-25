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

  AC(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeDFDOP(PFFunctionType type);
  std::string _mob_name;
  std::string _kappa_name;
  

private:
  
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _L;
  
};
#endif //AC_H
