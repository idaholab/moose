#include "LogWarningComponent.h"

registerMooseObject("THMTestApp", LogWarningComponent);

template <>
InputParameters
validParams<LogWarningComponent>()
{
  InputParameters params = validParams<Component>();
  params.addClassDescription("Component that logs a warning.");
  return params;
}

LogWarningComponent::LogWarningComponent(const InputParameters & params) : Component(params)
{
  logWarning("This is a warning.");
}
