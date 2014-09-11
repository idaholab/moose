/*
Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311
Equation 63
*/

#ifndef ACFracCoupledInterface_H
#define ACFracCoupledInterface_H

#include "KernelGrad.h"

//Forward Declarations
class ACFracCoupledInterface;

template<>
InputParameters validParams<ACFracCoupledInterface>();

class ACFracCoupledInterface : public KernelGrad
{
public:

  ACFracCoupledInterface(const std::string & name, InputParameters parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  std::string _mob_name;
  std::string _kappa_name;

  VariableValue & _c;
  VariableGradient & _grad_c;
  unsigned int _c_var;

private:

  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _L;

};
#endif //ACInterface_H
