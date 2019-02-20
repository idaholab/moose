#ifndef COSINEHUMPFUNCTION_H
#define COSINEHUMPFUNCTION_H

#include "Function.h"

class CosineHumpFunction;

template <>
InputParameters validParams<CosineHumpFunction>();

/**
 * Computes a cosine hump of a user-specified width and height
 *
 * Currently it is assumed that the direction of the hump is in the x, y,
 * or z direction, but this could later be extended to an arbitrary direction.
 */
class CosineHumpFunction : public Function
{
public:
  CosineHumpFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
  virtual RealVectorValue gradient(Real t, const Point & p);

protected:
  /// Component index of axis on which hump occurs
  const unsigned int _component;
  /// Width of hump
  const Real & _hump_width;
  /// Hump center position on selected axis
  const Real & _hump_center_position;

  /// Value before and after the hump
  const Real & _hump_begin_value;
  /// Value at the center of the hump
  const Real & _hump_center_value;

  /// Cosine amplitude
  const Real _cosine_amplitude;
  /// Middle value of hump
  const Real _hump_mid_value;
  /// Left end of hump
  const Real _hump_left_end;
  /// Right end of hump
  const Real _hump_right_end;
};

#endif // COSINEHUMPFUNCTION_H
