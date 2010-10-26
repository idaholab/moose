#ifndef ACBulk_H
#define ACBulk_H

#include "KernelValue.h"

//Forward Declarations
class ACBulk;

template<>
InputParameters validParams<ACBulk>();

class ACBulk : public KernelValue
{
public:

  ACBulk(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeDFDOP(PFFunctionType type);
  std::string _mob_name;
  

private:
  
  MaterialProperty<Real> & _L;
  
};
#endif //ACBulk_H
