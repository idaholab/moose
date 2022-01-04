#include "ConnectorBase.h"

InputParameters
ConnectorBase::validParams()
{
  InputParameters params = Component::validParams();
  return params;
}

ConnectorBase::ConnectorBase(const InputParameters & params) : Component(params) {}
