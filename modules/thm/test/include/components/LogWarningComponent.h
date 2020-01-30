#pragma once

#include "Component.h"

class LogWarningComponent;

template <>
InputParameters validParams<LogWarningComponent>();

/**
 * Component that logs a warning
 */
class LogWarningComponent : public Component
{
public:
  LogWarningComponent(const InputParameters & params);
};
