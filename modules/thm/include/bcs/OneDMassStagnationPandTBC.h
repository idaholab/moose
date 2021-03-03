#pragma once

#include "OneDNodalBC.h"
#include "OneDStagnationPandTBase.h"

class SinglePhaseFluidProperties;

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
  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _arhoEA;

  const unsigned int _arhouA_var_number;
  const unsigned int _arhoEA_var_number;

public:
  static InputParameters validParams();
};
