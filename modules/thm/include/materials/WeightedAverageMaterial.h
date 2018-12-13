#ifndef WEIGHTEDAVERAGEMATERIAL_H
#define WEIGHTEDAVERAGEMATERIAL_H

#include "Material.h"

class WeightedAverageMaterial;

template <>
InputParameters validParams<WeightedAverageMaterial>();

/**
 * Weighted average of material properties using aux variables as the weights
 *
 * This weighted average is computed as:
 * \f[
 *   \bar{y} = \frac{\sum\limits_i w_i y_i}{\sum\limits_i w_i} .
 * \f]
 ]
 */
class WeightedAverageMaterial : public Material
{
public:
  WeightedAverageMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The material property where the weighted average is stored
  MaterialProperty<Real> & _prop;
  /// Number of components (i.e. values and weights)
  const unsigned int _n_values;
  /// Values to average
  std::vector<const MaterialProperty<Real> *> _values;
  /// Weights of the values
  std::vector<const VariableValue *> _weights;
};

#endif /* WEIGHTEDAVERAGEMATERIAL_H */
