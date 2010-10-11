#ifndef CHBulk_H
#define CHBulk_H

#include "Kernel.h"

//Forward Declarations
class CHBulk;

template<>
InputParameters validParams<CHBulk>();

class CHBulk : public Kernel
{
public:

  CHBulk(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  std::string _mob_name;
  
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual RealGradient computeGradDFDCons(PFFunctionType type);

private:
  
  MaterialProperty<Real> & _M;
};
#endif //CHBulk_H
