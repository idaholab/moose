#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class CircularAreaHydraulicDiameterFunction;

template <>
InputParameters validParams<CircularAreaHydraulicDiameterFunction>();

/**
 * Computes hydraulic diameter for a circular area from its area function
 */
class CircularAreaHydraulicDiameterFunction : public Function, public FunctionInterface
{
public:
  CircularAreaHydraulicDiameterFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;
  virtual RealVectorValue gradient(Real t, const Point & p) override;

protected:
  /// Area function
  Function & _area_function;
};
