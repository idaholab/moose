#ifndef CHSPLIT1_H
#define CHSPLIT1_H

#include "Kernel.h"


//Forward Declarations
class CHSplit1;

template<>
InputParameters validParams<CHSplit1>();
/**
 * CHSplit1 calculates the c residual of the split Cahn-Hilliard equation
 */
class CHSplit1 : public Kernel
{
public:

  CHSplit1(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _w_var;
  VariableGradient & _grad_w;
  
  std::string _mob_name;
  MaterialProperty<Real> & _M;
};
#endif //CHSPLIT1_H
