#ifndef CHBulk_H
#define CHBulk_H

#include "KernelGrad.h"

//Forward Declarations
class CHBulk;

template<>
InputParameters validParams<CHBulk>();

class CHBulk : public KernelGrad
{
public:

  CHBulk(const std::string & name, InputParameters parameters);
  
protected:
  std::string _mob_name;
  
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual RealGradient computeGradDFDCons(PFFunctionType type);

private:
  
  MaterialProperty<Real> & _M;
};
#endif //CHBulk_H
