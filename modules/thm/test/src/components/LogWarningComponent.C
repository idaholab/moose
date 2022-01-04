#include "LogWarningComponent.h"

registerMooseObject("THMTestApp", LogWarningComponent);

InputParameters
LogWarningComponent::validParams()
{
  InputParameters params = Component::validParams();
  params.addClassDescription("Component that logs a warning.");
  return params;
}

LogWarningComponent::LogWarningComponent(const InputParameters & params) : Component(params)
{
  logWarning("This is a warning.");
}
