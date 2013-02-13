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

  enum WhenType {
    RESIDUAL = 0,
    JACOBIAN,
    INITIAL_CONDITION
  } _when;
  /// Counter for the number of calls
  unsigned int _call_no;
};

#endif /* EXCEPTIONKERNEL_H */
