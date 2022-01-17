#pragma once

#include "Component.h"

/**
 * Component that logs a warning
 */
class LogWarningComponent : public Component
{
public:
  LogWarningComponent(const InputParameters & params);

public:
  static InputParameters validParams();
};
