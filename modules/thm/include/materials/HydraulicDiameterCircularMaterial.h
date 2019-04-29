#pragma once

#include "Material.h"

class HydraulicDiameterCircularMaterial;

template <>
InputParameters validParams<HydraulicDiameterCircularMaterial>();

/**
 * Computes hydraulic diameter for a circular flow channel
 */
class HydraulicDiameterCircularMaterial : public Material
{
public:
  HydraulicDiameterCircularMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<Real> & _D_h;

  const VariableValue & _area;
};
