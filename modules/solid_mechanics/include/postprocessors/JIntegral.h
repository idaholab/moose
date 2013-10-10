#ifndef JINTEGRAL_H
#define JINTEGRAL_H

#include "ElementIntegralPostprocessor.h"

//Forward Declarations
class JIntegral;

template<>
InputParameters validParams<JIntegral>();

/**
 * This postprocessor computes the J-Integral
 *
 */
class JIntegral: public ElementIntegralPostprocessor
{
public:
  JIntegral(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();
  /// The gradient of the coupled q function
  VariableValue & _q;
  VariableGradient & _q_grad;
  RealVectorValue _direction;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor_small;
  const bool _large;
};

#endif
