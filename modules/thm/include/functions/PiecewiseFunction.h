#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class PiecewiseFunction;

template <>
InputParameters validParams<PiecewiseFunction>();

/**
 * Function which provides a piecewise representation of arbitrary functions.
 */
class PiecewiseFunction : public Function, public FunctionInterface
{
public:
  PiecewiseFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;
  virtual RealVectorValue gradient(Real t, const Point & p) override;
  virtual Real timeDerivative(Real t, const Point & p) override;

protected:
  /**
   * Gets the index of the function at the specified time and spatial point.
   *
   * @param[in] t   Time
   * @param[in] p   Spatial point
   *
   * @returns Index of the function at the specified time and spatial point
   */
  unsigned int getFunctionIndex(Real t, const Point & p) const;

  /// Axis on which the N-1 delimiting points lie
  const unsigned int _component;
  /// Use the time axis?
  const bool _use_time;
  /// N-1 coordinates in the chosen axis, in increasing order, delimiting the N function regions
  const std::vector<Real> & _axis_coordinates;

  /// Names of the functions in the N regions
  const std::vector<FunctionName> & _function_names;
  /// Number of functions, N
  const unsigned int _n_functions;
  /// Functions in the N regions
  std::vector<Function *> _functions;
};
