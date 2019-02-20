#include "JunctionScalarKernel.h"

template <>
InputParameters
validParams<JunctionScalarKernel>()
{
  InputParameters params = validParams<NodalScalarKernel>();

  params.addRequiredParam<std::vector<Real>>("normals", "node normals");

  return params;
}

JunctionScalarKernel::JunctionScalarKernel(const InputParameters & parameters)
  : NodalScalarKernel(parameters), _normals(getParam<std::vector<Real>>("normals"))
{
}
