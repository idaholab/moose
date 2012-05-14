#ifndef EXCEPTIONKERNEL_H
#define EXCEPTIONKERNEL_H

#include "Kernel.h"

// Forward Declaration
class ExceptionKernel;

template<>
InputParameters validParams<ExceptionKernel>();

/**
 * Kernel that generates MooseException
 */
class ExceptionKernel : public Kernel
{
public:
  ExceptionKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif /* EXCEPTIONKERNEL_H */
