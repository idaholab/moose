#include "ConnectorBase.h"

template <>
InputParameters
validParams<ConnectorBase>()
{
  InputParameters params = validParams<Component>();
  return params;
}

ConnectorBase::ConnectorBase(const InputParameters & params) : Component(params) {}
