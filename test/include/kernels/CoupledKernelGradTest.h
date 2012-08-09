#ifndef COUPLEDKERNELGRADTEST_H
#define COUPLEDKERNELGRADTEST_H

#include "KernelGrad.h"

class CoupledKernelGradTest;

template<>
InputParameters validParams<CoupledKernelGradTest>();


class CoupledKernelGradTest : public KernelGrad
{
public:
  CoupledKernelGradTest(const std::string & name, InputParameters parameters);
  virtual ~CoupledKernelGradTest();

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  RealVectorValue _beta;
  VariableValue & _var2;
  unsigned int _var2_num;

};


#endif /* COUPLEDKERNELGRADTEST_H */
