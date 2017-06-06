#ifndef FLUIDPROPERTIESVUMATERIAL_H
#define FLUIDPROPERTIESVUMATERIAL_H

#include "DerivativeMaterialInterfaceRelap.h"

class FluidProperties3EqnMaterial;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<FluidProperties3EqnMaterial>();

/**
 * Computes velocity and thermodynamic variables from solution variables for 1-phase flow.
 */
class FluidProperties3EqnMaterial : public DerivativeMaterialInterfaceRelap<Material>
{
public:
  FluidProperties3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Cross-sectional area
  const VariableValue & _area;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;

  /// Density
  MaterialProperty<Real> & _rho;
  MaterialProperty<Real> & _drho_drhoA;

  /// Specific volume
  MaterialProperty<Real> & _v;
  MaterialProperty<Real> & _dv_drhoA;

  /// Velocity
  MaterialProperty<Real> & _vel;
  MaterialProperty<Real> & _dvel_drhoA;
  MaterialProperty<Real> & _dvel_drhouA;

  /// Specific internal energy
  MaterialProperty<Real> & _e;
  MaterialProperty<Real> & _de_drhoA;
  MaterialProperty<Real> & _de_drhouA;
  MaterialProperty<Real> & _de_drhoEA;

  /// Pressure
  MaterialProperty<Real> & _p;
  MaterialProperty<Real> & _dp_drhoA;
  MaterialProperty<Real> & _dp_drhouA;
  MaterialProperty<Real> & _dp_drhoEA;

  /// Temperature
  MaterialProperty<Real> & _T;
  MaterialProperty<Real> & _dT_drhoA;
  MaterialProperty<Real> & _dT_drhouA;
  MaterialProperty<Real> & _dT_drhoEA;

  /// Sound speed
  MaterialProperty<Real> & _c;
  MaterialProperty<Real> & _cp;
  MaterialProperty<Real> & _cv;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _k;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};

#endif /* FLUIDPROPERTIESVUMATERIAL_H */
