#pragma once

#include "Component.h"

/**
 * Component used to test setup-status-checking capability
 */
class TestSetupStatusComponent : public Component
{
public:
  TestSetupStatusComponent(const InputParameters & params);

protected:
  virtual void init() override;

public:
  static InputParameters validParams();
};
