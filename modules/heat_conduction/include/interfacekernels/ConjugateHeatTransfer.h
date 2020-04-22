#include "InterfaceKernel.h"

class ConjugateHeatTransfer;

/**
 * InterfaceKernel for modeling conjugate heat transfer.
 * The fluid block must be the master block that owns
 * the sidset; therefore, "element" is the element that
 * the fluid temperature is defined. The solid block is the
 * neighbor side and the solid element is the "neighbor_elem"
 * The fluid energy equation may not be solved for temperature.
 * To handle that case, T_fluid must be provided separately.
 * If T_fluid is the primary variable of the fluid energy equation
 * then variable and T_fluid are the same.
 */
class ConjugateHeatTransfer : public InterfaceKernel
{
public:
  static InputParameters validParams();

  ConjugateHeatTransfer(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  /// Convective heat transfer coefficient
  const ADMaterialProperty<Real> & _h;

  /// Fluid temperature
  const VariableValue & _T_fluid;

  /// Element Jacobian is only applied if variable = T_fluid
  bool _apply_element_jacobian;
};
