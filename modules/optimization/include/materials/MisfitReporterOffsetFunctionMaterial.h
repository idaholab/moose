#pragma once

#include "ReporterOffsetFunctionMaterial.h"
#include "Function.h"

class Function;

/**
 * This class creates a misfit and misfit gradient material that can be used for
 * optimizing measurements weighted by functions.
 */
class MisfitReporterOffsetFunctionMaterial : public ReporterOffsetFunctionMaterial
{
public:
  static InputParameters validParams();

  MisfitReporterOffsetFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Simulation variable
  const VariableValue & _sim_var;

  /// Gradient of misfit with respect to material properties
  MaterialProperty<Real> & _mat_prop_gradient;

  /// values at each xyz coordinate
  const std::vector<Real> & _measurement_values;

  Real computeWeighting(const Point & point_shift);

private:
  /// convenience vectors (these are not const because reporters can change their size)
  std::vector<Real> _ones_vec;
  std::vector<Real> _zeros_vec;
  std::vector<Point> _zeros_pts;
};
