#pragma once

#include "Material.h"

/**
 *
 */
class ConcreteProperties : public Material
{
public:
  static InputParameters validParams();

  ConcreteProperties(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
};
