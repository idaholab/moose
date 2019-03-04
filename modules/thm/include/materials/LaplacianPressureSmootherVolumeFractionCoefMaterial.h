#ifndef LAPLACIANPRESSURESMOOTHERVOLUMEFRACTIONCOEFMATERIAL_H
#define LAPLACIANPRESSURESMOOTHERVOLUMEFRACTIONCOEFMATERIAL_H

#include "Material.h"

class LaplacianPressureSmootherVolumeFractionCoefMaterial;

template <>
InputParameters validParams<LaplacianPressureSmootherVolumeFractionCoefMaterial>();

class LaplacianPressureSmootherVolumeFractionCoefMaterial : public Material
{
public:
  LaplacianPressureSmootherVolumeFractionCoefMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  Real _Ce;
  const VariableValue & _p_bar;
  const VariableValue & _laplace_p;

  /// interfacial velocity
  const MaterialProperty<Real> & _vI;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _c;

  const bool _use_reference_pressure;
  const Real _p_ref;

  const bool _use_low_mach_fix;

  /// artificial dissipation coefficient \f$\mu\f$
  MaterialProperty<Real> & _coef;
};

#endif /* LAPLACIANPRESSURESMOOTHERVOLUMEFRACTIONCOEFMATERIAL_H */
