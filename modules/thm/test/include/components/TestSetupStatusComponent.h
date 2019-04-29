#pragma once

#include "Component.h"

class TestSetupStatusComponent;

template <>
InputParameters validParams<TestSetupStatusComponent>();

/**
 * Component used to test setup-status-checking capability
 */
class TestSetupStatusComponent : public Component
{
public:
  TestSetupStatusComponent(const InputParameters & params);

protected:
  virtual void init() override;
};
