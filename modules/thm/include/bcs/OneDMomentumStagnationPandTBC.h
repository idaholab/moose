#ifndef ONEDMOMENTUMSTAGNATIONPANDTBC_H
#define ONEDMOMENTUMSTAGNATIONPANDTBC_H

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "OneDStagnationPandTBase.h"

// Forward Declarations
class OneDMomentumStagnationPandTBC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<OneDMomentumStagnationPandTBC>();

/**
 * Stagnation P and T BC
 */
class OneDMomentumStagnationPandTBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>,
                                      public OneDStagnationPandTBase
{
public:
  OneDMomentumStagnationPandTBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  const bool & _reversible;
  const Real & _T0;
  const Real & _p0;

  const VariableValue & _area;
  const VariableValue & _vel;
  const VariableValue & _vel_old;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _arhoEA;

  const unsigned int _beta_var_number;
  const unsigned int _arhoA_var_number;
  const unsigned int _arhoEA_var_number;
};

#endif // ONEDMOMENTUMSTAGNATIONPANDTBC_H
