#ifndef JINTEGRAL_H
#define JINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "CrackFrontDefinition.h"

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
  virtual void initialSetup();
  virtual Real computeQpIntegral();
  /// The gradient of the coupled q function
  VariableValue & _q;
  VariableGradient & _q_grad;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_node_index;
  const unsigned int _crack_front_node_index;
  bool _treat_as_2d;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor_small;
  const bool _large;
};

#endif //JINTEGRAL3D_H
