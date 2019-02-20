#ifndef LAPIDUSCOEFMATERIAL_H
#define LAPIDUSCOEFMATERIAL_H

#include "Material.h"

class LapidusCoefMaterial;

template <>
InputParameters validParams<LapidusCoefMaterial>();

/**
 * Computes the stabilization coefficient for Lapidus smoother
 */
class LapidusCoefMaterial : public Material
{
public:
  LapidusCoefMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const VariableGradient & _velocity_grad;
  Real _cl;
  /// The direction of the pipe
  const MaterialProperty<RealVectorValue> & _dir;

  MaterialProperty<Real> & _coef;
};

#endif /* LAPIDUSCOEFMATERIAL_H */
