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
  /// The gradient of the scalar q field
  VariableGradient & _grad_of_scalar_q;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_node_index;
  const unsigned int _crack_front_node_index;
  bool _treat_as_2d;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;
};

#endif //JINTEGRAL3D_H
