#include "BoundaryBase.h"

InputParameters
BoundaryBase::validParams()
{
  InputParameters params = Component::validParams();
  params.addPrivateParam<std::string>("component_type", "boundary");
  return params;
}

BoundaryBase::BoundaryBase(const InputParameters & params) : Component(params) {}
