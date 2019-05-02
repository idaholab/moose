#include "PostprocessorSourceScalarKernel.h"

registerMooseObject("THMApp", PostprocessorSourceScalarKernel);

template <>
InputParameters
validParams<PostprocessorSourceScalarKernel>()
{
  InputParameters params = validParams<ODEKernel>();

  params.addRequiredParam<PostprocessorName>("pp", "Post-processor to act as source");

  params.addClassDescription("Adds arbitrary post-processor value as source term");

  return params;
}

PostprocessorSourceScalarKernel::PostprocessorSourceScalarKernel(const InputParameters & params)
  : ODEKernel(params),

    _pp(getPostprocessorValue("pp"))
{
}

Real
PostprocessorSourceScalarKernel::computeQpResidual()
{
  return -_pp;
}
