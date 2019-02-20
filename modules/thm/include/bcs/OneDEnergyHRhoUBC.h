#ifndef ONEDENERGYHRHOUBC_H
#define ONEDENERGYHRHOUBC_H

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDEnergyHRhoUBC;

template <>
InputParameters validParams<OneDEnergyHRhoUBC>();

/**
 *
 */
class OneDEnergyHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Volume fraction
  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;
  /// Specified enthalpy
  const Real & _H;
  /// Specified momentum
  const Real & _rhou;
  /// Cross-sectional area
  const VariableValue & _area;

  unsigned int _beta_var_num;
};

#endif /* ONEDENERGYHRHOUBC_H */
