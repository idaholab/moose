#ifndef CHSPLITVAR_H
#define CHSPLITVAR_H

#include "KernelGrad.h"

//Forward Declarations
class CHSplitVar;

template<>
InputParameters validParams<CHSplitVar>();

/**
 *
 */
class CHSplitVar : public KernelGrad
{
public:
  CHSplitVar(const std::string & name, InputParameters parameters);

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  Real _var_c;
  VariableGradient & _grad_c;
};

#endif //CHSPLITVAR_H
