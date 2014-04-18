#ifndef COUPLEDEIGENKERNEL_H
#define COUPLEDEIGENKERNEL_H

#include "EigenKernel.h"

//Forward Declarations
class CoupledEigenKernel;

template<>
InputParameters validParams<CoupledEigenKernel>();

class CoupledEigenKernel : public EigenKernel
{
public:
  CoupledEigenKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  VariableValue & _v;
};

#endif //COUPLEDEIGENKERNEL_H
