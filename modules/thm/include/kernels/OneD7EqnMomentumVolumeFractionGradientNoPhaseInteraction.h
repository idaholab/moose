#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction;

template <>
InputParameters validParams<OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction>();

/**
 * Computes the volume fraction gradient term in the momentum equation.
 *
 * This kernel computes the following volume fraction gradient term in the phase-\f$k\f$ momentum
 * equation when there is no phase interaction:
 * \f[
 *   p_k A \frac{\partial \alpha_k}{\partial x} .
 * \f]
 */
class OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction
  : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableValue & _area;
  const VariableGradient & _alpha_grad;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

  const unsigned int _arhoA_var_number;
  const unsigned int _arhoEA_var_number;
};
