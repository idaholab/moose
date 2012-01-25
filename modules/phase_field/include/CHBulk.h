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
  std::string _Dmob_name;
  
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual RealGradient computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c) = 0;

private:
  
  MaterialProperty<Real> & _M;
  bool _has_MJac;
  MaterialProperty<Real> * _DM;
  bool _implicit;
};
#endif //CHBulk_H
