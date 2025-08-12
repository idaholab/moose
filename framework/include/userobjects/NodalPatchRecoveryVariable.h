#pragma once
#include "NodalPatchRecoveryBase.h"

/// Patch recovery from a coupled variable
class NodalPatchRecoveryVariable : public NodalPatchRecoveryBase
{
public:
  static InputParameters validParams();
  NodalPatchRecoveryVariable(const InputParameters & params);

protected:
  /// Returns the recovered value at the quadrature point (_qp)
  Real computeValue() override;

private:
  /// Variable value
  const VariableValue & _v;
};
