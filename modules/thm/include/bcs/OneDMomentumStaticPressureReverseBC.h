#ifndef ONEDMOMENTUMSTATICPRESSUREREVERSEBC_H
#define ONEDMOMENTUMSTATICPRESSUREREVERSEBC_H

#include "OneDIntegratedBC.h"
#include "Function.h"

// Forward Declarations
class OneDMomentumStaticPressureReverseBC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<OneDMomentumStaticPressureReverseBC>();

/**
 * Static p, T applied at the outlet in case of reversal flow
 */
class OneDMomentumStaticPressureReverseBC : public OneDIntegratedBC
{
public:
  OneDMomentumStaticPressureReverseBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _is_liquid;
  Real _sign;

  const VariableValue & _alpha;
  const VariableValue & _vel;
  const VariableValue & _vel_old;
  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _temperature;

  unsigned int _beta_varnum;
  unsigned int _arhoA_varnum;

  /// Specified pressure
  Real _p;

  const SinglePhaseFluidProperties & _spfp;
};

#endif // ONED7EQNMOMENTUMSTATICPANDTREVERSEBC_H
