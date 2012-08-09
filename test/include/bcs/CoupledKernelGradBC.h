#ifndef COUPLEDKERNELGRADBC_H
#define COUPLEDKERNELGRADBC_H

#include "IntegratedBC.h"

class CoupledKernelGradBC;
class Function;

template <>
InputParameters validParams<CoupledKernelGradBC>();

class CoupledKernelGradBC : public IntegratedBC
{
public:
  CoupledKernelGradBC(const std::string & name, InputParameters parameters);

  virtual ~CoupledKernelGradBC();

protected:
  virtual Real computeQpResidual();

  RealVectorValue _beta;
  VariableValue & _var2;
};

#endif /* COUPLEDKERNELGRADBC_H */
