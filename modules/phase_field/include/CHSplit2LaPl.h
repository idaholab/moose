#ifndef CHSPLIT2LAPL_H
#define CHSPLIT2LAPL_H

#include "Kernel.h"


//Forward Declarations
class CHSplit2LaPl;

template<>
InputParameters validParams<CHSplit2LaPl>();

class CHSplit2LaPl : public Kernel
{
public:

  CHSplit2LaPl(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _c_var;
  MooseArray<RealGradient> & _grad_c;
  
  std::string _kappa_name;
  MaterialProperty<Real> & _kappa;
  
};
#endif //CHSPLIT2LAPL_H
