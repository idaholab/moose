#pragma once

#include "CNSKernel.h"

class CNSMomentumPressureGradient;

declareADValidParams(CNSMomentumPressureGradient);

/**
 * Kernel representing the momentum pressure gradient term with strong
 * form $\epsilon\nabla P$. By default, the divergence theorem is applied
 * to this kernel by expressing the strong form instead as
 * $\nabla(\epsilon P)-P\nabla\epsilon$ and integrating the $\nabla(\epsilon P)$
 * term by parts to provide a boundary condition.
 */
class CNSMomentumPressureGradient : public CNSKernel
{
public:
  CNSMomentumPressureGradient(const InputParameters & parameters);

protected:
  virtual ADReal weakResidual() override;

  virtual ADReal strongResidual() override;

  /// whether to apply the divergence theorem to this kernel
  const bool & _divergence;

  /// component of the momentum equation
  const unsigned int & _component;

  /// porosity
  const VariableValue & _eps;

  /// porosity gradient
  const VariableGradient & _grad_eps;

  /// pressure
  const ADMaterialProperty<Real> & _pressure;

  /// pressure gradient
  const ADMaterialProperty<RealVectorValue> & _grad_pressure;

  using CNSKernel::_rz_coord;
};
