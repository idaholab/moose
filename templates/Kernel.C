
#include "TmplKernel.h"

template<>
InputParameters validParams<TmplKernel>()
{
  InputParameters params = validParams<Kernel>();
  // add custom kernel parameters here
  return params;
}

TmplKernel::TmplKernel(const InputParameters & parameters)
    : Kernel(parameters)
{
}

Real
TmplKernel::computeQpResidual() {
  // implement residual calculation here
  return 0;
}

Real
TmplKernel::computeQpJacobian() {
  // optionally implement jacobian calculation here
  return 0;
}

// implement these functions only if you have a particular need to:
Real TmplKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }
void TmplKernel::precalculateResidual() { }
void TmplKernel::precalculateJacobian() { }
void TmplKernel::precalculateOffDiagJacobian(unsigned int /* jvar */) { }

