#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class GeneralizedCircumference;

template <>
InputParameters validParams<GeneralizedCircumference>();

/**
 * The generalized circumference, sigma_w, is defined such that the
 *
 * "projected area of a segment of duct wall of length dx" = sigma_w * dx
 *
 * It is used in the variable-area wall heating term:
 *
 * H_w * sigma_w * (T - T_w)
 *
 * for the variable-area equations.  This class currently assumes a
 * circular flow channel, but this could later be generalized for any type of
 * flow channel cross section.  For a circular flow channel, the generalized
 * circumference can be shown to be:
 *
 * sigma_w = sqrt(4*pi*A + (dA/dx)^2)
 */
class GeneralizedCircumference : public Function, public FunctionInterface
{
public:
  GeneralizedCircumference(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;

protected:
  // The generalized circumference function depends on the cross-sectional area.
  const Function & _area_func;
};
