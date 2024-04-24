#include "ElementIntegralPostprocessor.h"

/**
 * Computes the L2-Norm difference between two solution vector fields.
 */
class ElementVectorL2Difference : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementVectorL2Difference(const InputParameters & parameters);

  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;

  const VectorVariableValue & _vector_variable;
  const VectorVariableValue & _other_vector_variable;

private:
  void checkVectorVariables() const;
};
