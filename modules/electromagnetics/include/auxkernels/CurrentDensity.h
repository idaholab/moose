#pragma once

#include "AuxKernel.h"

/**
 *  Calculates the current density vector field when given electrostatic potential
 *  (electrostatic = true, default) or electric field.
 */
template <bool is_ad>
class CurrentDensityTempl : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  CurrentDensityTempl(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// Is the current density based on electrostatic potential?
  const bool & _is_es;

  /// Gradient of electrostatic potential
  const VariableGradient & _grad_potential;

  /// Vector variable of electric field (calculated using full electromagnetic description)
  const VectorVariableValue & _electric_field;

  /// Electrical conductivity (in S/m)
  const GenericMaterialProperty<Real, is_ad> & _conductivity;
};

typedef CurrentDensityTempl<false> CurrentDensity;
typedef CurrentDensityTempl<true> ADCurrentDensity;
