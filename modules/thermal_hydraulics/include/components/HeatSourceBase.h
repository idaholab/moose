#pragma once

#include "Component.h"

/**
 * Base class for heat source components
 */
class HeatSourceBase : public Component
{
public:
  HeatSourceBase(const InputParameters & parameters);

protected:
  virtual void check() const override;

  /// Names of the heat structure regions where heat generation is to be applied
  const std::vector<std::string> & _region_names;

public:
  static InputParameters validParams();
};
