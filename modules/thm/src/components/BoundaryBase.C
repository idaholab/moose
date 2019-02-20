#include "BoundaryBase.h"

template <>
InputParameters
validParams<BoundaryBase>()
{
  InputParameters params = validParams<Component>();
  params.addPrivateParam<std::string>("component_type", "boundary");
  return params;
}

BoundaryBase::BoundaryBase(const InputParameters & params) : Component(params) {}
