#ifndef ELEMENTHEATFLUXPOSTPROCESSOR_H
#define ELEMENTHEATFLUXPOSTPROCESSOR_H

#include "ElementIntegralPostprocessor.h"

class ElementHeatFluxPostprocessor;

template <>
InputParameters validParams<ElementHeatFluxPostprocessor>();

/**
 * Postprocessor to compute total heat flux going into the fluid
 *
 * This is used for debugging.
 */
class ElementHeatFluxPostprocessor : public ElementIntegralPostprocessor
{
public:
  ElementHeatFluxPostprocessor(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;
  const VariableValue & _Tfluid;
  /// convective heat transfer coefficient, W/m^2-K
  const MaterialProperty<Real> & _Hw;
  /// Heat flux perimeter
  const VariableValue & _P_hf;
};

#endif /* ELEMENTHEATFLUXPOSTPROCESSOR_H */
