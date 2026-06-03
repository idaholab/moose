#include "ADExpU.h"

registerMooseObject("MooseTestApp", ADExpU);

InputParameters
ADExpU::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addClassDescription("Scalar kernel residual: exp(u - 1).");
  return params;
}

ADExpU::ADExpU(const InputParameters & parameters) : ADScalarKernel(parameters) {}

ADReal
ADExpU::computeQpResidual()
{
  using std::exp;
  return exp(_u[_i] - 1.0);
}
