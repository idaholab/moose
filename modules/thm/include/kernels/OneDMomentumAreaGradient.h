#ifndef ONEDMOMENTUMAREAGRADIENT_H
#define ONEDMOMENTUMAREAGRADIENT_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDMomentumAreaGradient;

template <>
InputParameters validParams<OneDMomentumAreaGradient>();

/**
 * Computes the area gradient term in the momentum equation.
 *
 * This kernel computes the following area gradient term in the momentum equation:
 * \f[
 *   p \frac{\partial A}{\partial x} .
 * \f]
 */
class OneDMomentumAreaGradient : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDMomentumAreaGradient(const InputParameters & parameters);
  virtual ~OneDMomentumAreaGradient();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _dp_drhoA;
  const MaterialProperty<Real> & _dp_drhouA;
  const MaterialProperty<Real> * const _dp_drhoEA;
  const VariableValue & _area;
  const VariableGradient & _area_grad;

  unsigned int _rhoA_var_number;
  unsigned int _rhoEA_var_number;
};

#endif /* ONEDMOMENTUMAREAGRADIENT_H */
