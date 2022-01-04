#include "PostprocessorSourceScalarKernel.h"

registerMooseObject("THMApp", PostprocessorSourceScalarKernel);

InputParameters
PostprocessorSourceScalarKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();

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
