#ifndef PMASSEIGENKERNEL_H
#define PMASSEIGENKERNEL_H

#include "EigenKernel.h"

//Forward Declarations
class PMassEigenKernel;

template<>
InputParameters validParams<PMassEigenKernel>();

/**
 * This kernel implements (v, |u|^(p-2) u)/k, where u is the variable, v is the test function
 * and k is the eigenvalue. When p=2, this kernel is equivalent with MassEigenKernel.
 */

class PMassEigenKernel : public EigenKernel
{
public:
  PMassEigenKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif //PMASSEIGENKERNEL_H
