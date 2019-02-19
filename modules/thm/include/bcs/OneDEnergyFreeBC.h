#ifndef ONEDENERGYFREEBC_H
#define ONEDENERGYFREEBC_H

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class OneDEnergyFreeBC;

template <>
InputParameters validParams<OneDEnergyFreeBC>();

/**
 * A BC for the mass equation in which nothing is specified (i.e.
 * everything is allowed to be "free".
 */
class OneDEnergyFreeBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  bool _is_liquid;
  Real _sign;
  unsigned _arhoA_var_number;
  unsigned _arhouA_var_number;
  unsigned _arhoEA_var_number;
  const VariableValue & _arhouA;
  const VariableValue & _enthalpy;
  const VariableValue & _vel;
  const VariableValue & _area;
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  bool _has_beta;
  unsigned int _beta_var_number;
  const MaterialProperty<Real> * _dp_dbeta;
  const VariableValue & _alpha;
};

#endif // ONEDENERGYFREEBC_H
