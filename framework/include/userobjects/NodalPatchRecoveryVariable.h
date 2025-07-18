#pragma once
#include "NodalPatchRecoveryBase.h"

/**
 * NodalPatchRecoveryVariable
 *
 * Derived from NodalPatchRecoveryBase.
 * Reads any coupled variable using `coupledValue(<var_name>)[_qp]`.
 * Suitable for all field scalar-valued variables; for vector variables, split them in the input
 * file as <var>_x, <var>_y, etc., and couple them separately.
 */
class NodalPatchRecoveryVariable : public NodalPatchRecoveryBase
{
public:
  static InputParameters validParams();
  NodalPatchRecoveryVariable(const InputParameters & params);

protected:
  /// Returns the recovered value at the quadrature point (_qp)
  Real computeValue() override { return _var[_qp]; };

private:
  /// Variable value
  const VariableValue & _var;

  /// Variable name
  const VariableName _var_name;
};
