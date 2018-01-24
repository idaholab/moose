#ifndef PFPARAMSPOLYFREEENERGY_H
#define PFPARAMSPOLYFREEENERGY_H

#include "Material.h"

// Forward Declarations
class PFParamsPolyFreeEnergy;

template <>
InputParameters validParams<PFParamsPolyFreeEnergy>();

/**
 * Calculated properties for a single component phase field model using polynomial free energies
 */
class PFParamsPolyFreeEnergy : public Material
{
public:
  PFParamsPolyFreeEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  ///Variable values
  const VariableValue & _c;
  const VariableValue & _T;

  ///Mateiral property declarations
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;

  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _c_eq;
  MaterialProperty<Real> & _W;
  MaterialProperty<Real> & _Qstar;
  MaterialProperty<Real> & _D;

  ///Input parameters
  Real _int_width;
  Real _length_scale;
  Real _time_scale;
  MooseEnum _order;
  Real _D0;
  Real _Em;
  Real _Ef;
  Real _surface_energy;

  const Real _JtoeV;
  const Real _kb;
};

#endif // PFPARAMSPOLYFREEENERGY_H
