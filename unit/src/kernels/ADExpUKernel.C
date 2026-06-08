#include "ADExpUKernel.h"

registerMooseObject("MooseUnitApp", ADExpUKernel);

InputParameters
ADExpUKernel::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Field kernel source density: exp(u - 1).");
  return params;
}

ADExpUKernel::ADExpUKernel(const InputParameters & parameters) : ADKernelValue(parameters) {}

ADReal
ADExpUKernel::precomputeQpResidual()
{
  using std::exp;
  return exp(_u[_qp] - 1.0);
}
