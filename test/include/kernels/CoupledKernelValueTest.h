#ifndef COUPLEDKERNELVALUETEST_H
#define COUPLEDKERNELVALUETEST_H

#include "KernelValue.h"

class CoupledKernelValueTest;

template<>
InputParameters validParams<CoupledKernelValueTest>();


class CoupledKernelValueTest : public KernelValue
{
public:
  CoupledKernelValueTest(const std::string & name, InputParameters parameters);
  virtual ~CoupledKernelValueTest();

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  VariableValue & _var2;
  unsigned int _var2_num;

};


#endif /* COUPLEDKERNELVALUETEST_H */
