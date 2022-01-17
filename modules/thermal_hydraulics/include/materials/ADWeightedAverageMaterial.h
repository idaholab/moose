#pragma once

#include "Material.h"

/**
 * Weighted average of material properties using aux variables as the weights
 *
 * This weighted average is computed as:
 * \f[
 *   \bar{y} = \frac{\sum\limits_i w_i y_i}{\sum\limits_i w_i} .
 * \f]
 ]
 */
class ADWeightedAverageMaterial : public Material
{
public:
  ADWeightedAverageMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The material property where the weighted average is stored
  ADMaterialProperty<Real> & _prop;
  /// Number of components (i.e. values and weights)
  const unsigned int _n_values;
  /// Values to average
  std::vector<const ADMaterialProperty<Real> *> _values;
  /// Weights of the values
  std::vector<const ADVariableValue *> _weights;

public:
  static InputParameters validParams();
};
