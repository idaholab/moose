#pragma once

#include "FlowJunction.h"

class JunctionWithLossesBase;

template <>
InputParameters validParams<JunctionWithLossesBase>();

/**
 * Base class for junctions that have losses
 */
class JunctionWithLossesBase : public FlowJunction
{
public:
  JunctionWithLossesBase(const InputParameters & parameters);

protected:
  /// A vector to store user inputed K loss (form loss, minor loss) coefficients.
  std::vector<Real> _k_coeffs;
  /// A vector to store user inputed reverse K loss (form loss, minor loss) coefficients.
  std::vector<Real> _kr_coeffs;
  /// A reference area for this junction to calculate its reference velocity (User input)
  const Real & _ref_area;
};
