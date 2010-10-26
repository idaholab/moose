#ifndef ACInterface_H
#define ACInterface_H

#include "KernelGrad.h"

//Forward Declarations
class ACInterface;

template<>
InputParameters validParams<ACInterface>();

class ACInterface : public KernelGrad
{
public:

  ACInterface(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  std::string _mob_name;
  std::string _kappa_name;
  

private:
  
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _L;
  
};
#endif //ACInterface_H
