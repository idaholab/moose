#pragma once

#include "Material.h"

/**
 * Computes hydraulic diameter for a circular flow channel
 */
class ADHydraulicDiameterCircularMaterial : public Material
{
public:
  ADHydraulicDiameterCircularMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  ADMaterialProperty<Real> & _D_h;

  const ADVariableValue & _area;

public:
  static InputParameters validParams();
};
