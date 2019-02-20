#ifndef ONEDMASSSTAGNATIONPANDTBC_H
#define ONEDMASSSTAGNATIONPANDTBC_H

#include "OneDNodalBC.h"
#include "OneDStagnationPandTBase.h"

// Forward Declarations
class OneDMassStagnationPandTBC;
class SinglePhaseFluidProperties;
class VolumeFractionMapper;

template <>
InputParameters validParams<OneDMassStagnationPandTBC>();

/**
 * Stagnation P and T BC
 */
class OneDMassStagnationPandTBC : public OneDNodalBC, public OneDStagnationPandTBase
{
public:
  OneDMassStagnationPandTBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const bool & _reversible;
  const Real & _T0;
  const Real & _p0;

  const VariableValue & _area;
  const VariableValue & _vel;
  const VariableValue & _vel_old;
  const VariableValue & _alpha;
  const VariableValue & _beta;
  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _arhoEA;

  const Real _sign;

  const unsigned int _beta_var_number;
  const unsigned int _arhouA_var_number;
  const unsigned int _arhoEA_var_number;

  const VolumeFractionMapper * _vfm;
};

#endif // ONEDMASSSTAGNATIONPANDTBC_H
