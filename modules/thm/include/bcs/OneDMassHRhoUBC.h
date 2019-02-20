#ifndef ONEDMASSHRHOUBC_H
#define ONEDMASSHRHOUBC_H

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDMassHRhoUBC;

template <>
InputParameters validParams<OneDMassHRhoUBC>();

/**
 *
 */
class OneDMassHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMassHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Volume fraction
  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;
  /// Specified momentum
  const Real & _rhou;
  /// Cross-sectional area
  const VariableValue & _area;

  unsigned int _beta_var_num;
};

#endif /* ONEDMASSHRHOUBC_H */
